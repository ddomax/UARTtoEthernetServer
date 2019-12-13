#include "bssamplerwidget.h"
#include "ui_bssamplerwidget.h"
#include "api/myhelper.h"

BSSamplerWidget::BSSamplerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BSSamplerWidget)
{
    ui->setupUi(this);
    setup_CustomPlot();

    board = QApplication::clipboard();
    connect(board, SIGNAL(dataChanged()), this, SLOT(clipboard_changed()));

    for(int i=0;i<RECORD_DATA_LEN;i++)
        data_record[i] = 0;

    QStringList baudrates;
    baudrates<<"1200"<<"4800"<<"9600"<<"115200";
    ui->BaudBox->addItems(baudrates);

    QStringList bitnums;
    bitnums<<"8";
    ui->BitNumBox->insertItems(8,bitnums);

    QStringList parity;
    parity<<"No"<<"Yes";
    ui->ParityBox->insertItems(0,parity);

    QStringList stopbits;
    stopbits<<"1"<<"2";
    ui->StopBox->insertItems(1,stopbits);

    refresh_serialPorts();
    //设置波特率下拉菜单默认显示第三项
    ui->BaudBox->setCurrentIndex(2);
    //关闭发送按钮的使能
    ui->sendButton->setEnabled(false);

    //   setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint);

    this->initStyle();
    this->initForm();
    //   myHelper::moveFormToCenter(this);
    screencount = QApplication::desktop()->screenCount();
    QRect screen1;
    screen1 = QApplication::desktop()->screenGeometry(screencount-1);
    this->setGeometry(screen1.x(),screen1.y(),1024,800);
    qDebug() << QString("界面设定成功！");
    qDebug() << QString("%1 %2 %3 %4").arg(screen1.x())
                                .arg(screen1.y())
                                .arg(screen1.width())
                                .arg(screen1.height());

//    myHelper::showMessageBoxInfo("使用步骤：连接接收端硬件->点击打开串口");
//    myHelper::showMessageBoxInfo("->接通电子称电源->点击“进入后台监控”");
//    myHelper::showMessageBoxInfo("->从头开始放映PPT");
    m_client = new QTcpSocket();
    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    connect(&captureTimer, SIGNAL(timeout()), this, SLOT(Capture()));
//    on_openButton_clicked();
}

BSSamplerWidget::~BSSamplerWidget()
{
    TaskBarShow();
    m_client->close();
    delete ui;
}

void BSSamplerWidget::initStyle()
{
    this->m_max = false;
    this->locatRect = this->geometry();
    this->setProperty("Form", true);
    this->setProperty("CanMove", true);
    this->setWindowTitle(ui->lab_Title->text());

    //设置窗体标题栏隐藏
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);
    //安装事件监听器,让标题栏识别鼠标双击
    ui->lab_Title->installEventFilter(this);

    IconHelper::getInstance()->setIcon(ui->btnMenu_Close, QChar(0xf00d));
    IconHelper::getInstance()->setIcon(ui->btnMenu_Max, QChar(0xf096));
    IconHelper::getInstance()->setIcon(ui->btnMenu_Min, QChar(0xf068));
//    IconHelper::getInstance()->setIcon(ui->lab_Ico, QChar(0xf015));
    QImage* img=new QImage;
    img->load(":/HNSDFZ.jpg");
    QImage scaled_img =  img->scaled(ui->lab_Ico->height(),ui->lab_Ico->height(),Qt::IgnoreAspectRatio);
//    QImage scaled_img = img->scaled(32,32,Qt::IgnoreAspectRatio);
    ui->lab_Ico->setPixmap(QPixmap::fromImage(scaled_img));

    mapStyle["黑色"] = QString(":/qss/black.css");  // 对应的主题样式
    mapStyle["灰黑色"] = QString(":/qss/brown.css");
    mapStyle["灰色"] = QString(":/qss/gray.css");
    mapStyle["浅灰色"] = QString(":/qss/lightgray.css");
    mapStyle["深灰色"] = QString(":/qss/darkgray.css");
    mapStyle["银色"] = QString(":/qss/silvery.css");
    mapStyle["淡蓝色"] = QString(":/qss/blue.css");
    mapStyle["蓝色"] = QString(":/qss/dev.css");

    connect(ui->btnMenu_Close, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->btnMenu_Min, SIGNAL(clicked()), this, SLOT(showMinimized()));
}

bool BSSamplerWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick) {
        this->on_btnMenu_Max_clicked();
        return true;
    }
    return QObject::eventFilter(obj, event);
}

void BSSamplerWidget::initForm()
{
    QStringList qssName;
    qssName << "黑色" << "灰黑色" << "灰色" << "浅灰色" << "深灰色" << "银色" << "淡蓝色" << "蓝色";
    ui->cboxStyle->addItems(qssName);

    myHelper::sleep(300);
    ui->cboxStyle->setCurrentIndex(3);
}

void BSSamplerWidget::on_btnMenu_Max_clicked()
{
    if (m_max) {
        this->setGeometry(locatRect);
        IconHelper::getInstance()->setIcon(ui->btnMenu_Max, QChar(0xf096));
        ui->btnMenu_Max->setToolTip("最大化");
        this->setProperty("CanMove", true);
    } else {
        locatRect = this->geometry();
        this->setGeometry(qApp->desktop()->availableGeometry());
        IconHelper::getInstance()->setIcon(ui->btnMenu_Max, QChar(0xf079));
        ui->btnMenu_Max->setToolTip("还原");
        this->setProperty("CanMove", false);
    }

    m_max = !m_max;
}

void BSSamplerWidget::on_cboxStyle_currentIndexChanged(const QString &text) //@wzguo 2015.09.09
{
    QString qssName = text.trimmed();

    QString qssFile = mapStyle[qssName];

    if (qssFile.trimmed() == "")
        qssFile = ":/qss/black.css";

    myHelper::setStyle(qssFile);
}

void BSSamplerWidget::setup_CustomPlot()
{
    ui->customPlot->addGraph(); // blue line
    ui->customPlot->graph(0)->setPen(QPen(QColor(40, 110, 255)));
    ui->customPlot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));
//    ui->customPlot->graph(0)->setBrush(QBrush(QPixmap(":/HNSDFZ.jpg")));
//    ui->customPlot->addGraph(); // red line
//    ui->customPlot->graph(1)->setPen(QPen(QColor(255, 110, 40)));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    ui->customPlot->xAxis->setTicker(timeTicker);
    ui->customPlot->axisRect()->setupFullAxesBox();
    ui->customPlot->yAxis->setRange(0, 60);

    ui->customPlot->xAxis->setLabel("时间（s）");
    ui->customPlot->yAxis->setLabel("视重（kg）");

    ui->customPlot->xAxis->setLabelFont(QFont(QFont().family(), 12));
    ui->customPlot->yAxis->setLabelFont(QFont(QFont().family(), 12));

    ui->customPlot->xAxis->setTickLabelFont(QFont(QFont().family(), 12));
    ui->customPlot->yAxis->setTickLabelFont(QFont(QFont().family(), 12));

    ui->customPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom| QCP::iSelectAxes |
                                      QCP::iSelectLegend | QCP::iSelectPlottables);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(Read_Line()));
    dataTimer.start(5); // Interval 0 means to refresh as fast as possible
}

void BSSamplerWidget::refresh_serialPorts()
{
    ui->PortBox->clear();
    //查找可用的串口
   foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
   {
       QSerialPort serial;
       serial.setPort(info);
       if(serial.open(QIODevice::ReadWrite))
       {
           ui->PortBox->addItem(serial.portName());
           serial.close();
       }
   }
}


//清空接收窗口
void BSSamplerWidget::on_clearButton_clicked()
{
    ui->textEdit->clear();
    ui->customPlot->graph(0)->data().data()->clear();
//    ui->customPlot->graph(1)->data().data()->clear();
}
//发送数据
void BSSamplerWidget::on_sendButton_clicked()
{
    serial->write(ui->textEdit_2->toPlainText().toLatin1());
}
//读取接收到的数据
void BSSamplerWidget::Read_Data()
{
    QByteArray buf;
    buf = serial->readLine();
    if(!buf.isEmpty())
    {
//        QString str = ui->textEdit->toPlainText();
//        str+=tr(buf);
//        ui->textEdit->clear();
//        ui->textEdit->append(str);
        ui->textEdit->insertPlainText(buf);
//        ui->textEdit->append(buf);
    }
    buf.clear();
}

void BSSamplerWidget::Read_Line()
{
    const int MAX_LEN = 64;
    static int rec_num,tol_size = 0;
    int size = 0;

//    if(ui->openButton->text()==tr("打开串口"))
//        return;
//    if(serial->bytesAvailable()<10)
//        return;
//    QByteArray buf;
//    buf = serial->readLine(MAX_LEN);

    if(ui->openClientButton->text()==QString("打开客户端"))
        return;
    if(m_client->bytesAvailable()<10)
        return;
    QByteArray buf;
    buf = m_client->readLine(MAX_LEN);

    size = buf.size();
    tol_size+=size;
    buf.chop(2);
//    qDebug() << buf;
    if(buf.isEmpty()){
        qDebug() << tr("Buffer is empty!");
        return;
    }
    rec_num++;
//    printf("Num:%3d Size:%2d Total:%8d Data:%8d\r\n",rec_num,size,tol_size,buf.toInt());


    for(int i=0;i<RECORD_DATA_LEN;i++)
        data_record[i] = data_record[i+1];
    data_record[RECORD_DATA_LEN-1] = buf.toDouble();

    static int points_count = 0;
    static QTime time(QTime::currentTime());
    // calculate two new data points:
    double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
    static double lastPointKey = 0;
//    if (key-lastPointKey > 0.002) // at most add point every 2 ms
//    {
        // add data to lines:
//        ui->customPlot->graph(0)->addData(key, 9.80665*(data_record[RECORD_DATA_LEN-1]-offset_val)*k_val);
//        if(ui->customPlot->graph(0)->dataCount()==0)
//            ui->customPlot->graph(0)->addData(key, 10);
//        ui->customPlot->graph(0)->addData(key, 9.80665*(data_record[RECORD_DATA_LEN-1]-offset_val)*k_val);
//        ui->customPlot->graph(0)->addData(key, 9.80665*(data_average()-offset_val)*k_val);
        ui->customPlot->graph(0)->addData(points_count/80.0, 9.80665*(data_average()-offset_val)*k_val);
//        ui->customPlot->graph(1)->addData(key, (data_record[RECORD_DATA_LEN-1]-offset_val)*k_val);
        // rescale value (vertical) axis to fit the current data:
        ui->customPlot->graph(0)->rescaleValueAxis();
//        ui->customPlot->graph(1)->rescaleValueAxis(true);
        lastPointKey = key;
//    }
    // make key axis range scroll with the data (at a constant range size of 8):
//    ui->customPlot->xAxis->setRange(key, 8, Qt::AlignRight);
    ui->customPlot->xAxis->setRange(points_count/80.0, 8, Qt::AlignRight);
    ui->customPlot->replot();
    points_count++;

    // calculate frames per second:
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 2) // average fps over 2 seconds
    {
//      ui->statusBar->showMessage(
//            QString("%1 FPS, Total Data points: %2")
//            .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
//            .arg(ui->customPlot->graph(0)->data()->size()+ui->customPlot->graph(1)->data()->size())
//            , 0);
      qDebug() << QString("%1 FPS, Total Data points: %2, Buffer: %3")
                  .arg(frameCount/(key-lastFpsKey), 0, 'f', 1)
                  .arg(ui->customPlot->graph(0)->data()->size())
                  .arg(m_client->bytesAvailable());

      lastFpsKey = key;
      frameCount = 0;
    }
}

void BSSamplerWidget::on_openButton_clicked()
{
    if(ui->openButton->text()==tr("打开串口"))
    {
        serial = new QSerialPort;
        //设置串口名
        serial->setPortName(ui->PortBox->currentText());
        //设置波特率
        serial->setBaudRate(ui->BaudBox->currentText().toInt());
        //设置数据位数
        switch(ui->BitNumBox->currentIndex())
        {
        case 8: serial->setDataBits(QSerialPort::Data8); break;
        default: break;
        }
        //设置奇偶校验
        switch(ui->ParityBox->currentIndex())
        {
        case 0: serial->setParity(QSerialPort::NoParity); break;
        default: break;
        }
        //设置停止位
        switch(ui->StopBox->currentIndex())
        {
        case 1: serial->setStopBits(QSerialPort::OneStop); break;
        case 2: serial->setStopBits(QSerialPort::TwoStop); break;
        default: break;
        }
        //设置流控制
        serial->setFlowControl(QSerialPort::NoFlowControl);
        //打开串口
        if(!serial->open(QIODevice::ReadWrite)){
            qDebug() << tr("串口打开失败！");
            return;
        }
        //关闭设置菜单使能
        ui->PortBox->setEnabled(false);
        ui->BaudBox->setEnabled(false);
        ui->BitNumBox->setEnabled(false);
        ui->ParityBox->setEnabled(false);
        ui->StopBox->setEnabled(false);
        ui->openButton->setText(tr("关闭串口"));
        ui->sendButton->setEnabled(true);
        //连接信号槽
//        QObject::connect(serial, &QSerialPort::readyRead, this, &BSSamplerWidget::Read_Line);
//        QObject::connect(serial, &QSerialPort::readyRead, this, &BSSamplerWidget::Read_Data);
    }
    else
    {
        //关闭串口
        serial->clear();
        serial->close();
        serial->deleteLater();
        //恢复设置使能
        ui->PortBox->setEnabled(true);
        ui->BaudBox->setEnabled(true);
        ui->BitNumBox->setEnabled(true);
        ui->ParityBox->setEnabled(true);
        ui->StopBox->setEnabled(true);
        ui->openButton->setText(tr("打开串口"));
        ui->sendButton->setEnabled(false);
    }
}

void BSSamplerWidget::on_refreshButton_clicked()
{
    refresh_serialPorts();
}

void BSSamplerWidget::on_offsetcalButton_clicked()
{
    offset_val = data_average();
    qDebug() << QString("偏移校准成功！偏移: %1").arg(offset_val, 0, 'f', 2);;
}

void BSSamplerWidget::on_stdweightcalButton_clicked()
{
    k_val=ui->stdweightEdit->text().toDouble() / (data_average()-offset_val);
    qDebug() << QString("斜率校准成功！斜率: %1").arg(k_val, 0, 'f', 10);
}

double BSSamplerWidget::data_average()
{
    double avg,acc=0;
    for(int i=0;i<RECORD_DATA_LEN;i++)
        acc += data_record[i];
    avg = acc / RECORD_DATA_LEN;
    return avg;
}

void BSSamplerWidget::clipboard_changed()
{
//    QString str = board->text();
//    QMessageBox::information(NULL, "From clipboard", str);
    on_screenshotButton_clicked();
    ishide = false;
    show();
    TaskBarHide();
}

void BSSamplerWidget::findtarget()
{
    if(on_screenshotButton_clicked())
    {
        ishide = false;
        show();
        TaskBarHide();
    }
}

void BSSamplerWidget::on_hideButton_clicked()
{
    ishide = true;
    hide();
    TaskBarShow();
    captureTimer.start(CAPTURE_SCAN_INTERVAL); // Interval 0 means to refresh as fast as possible
}

bool BSSamplerWidget::on_screenshotButton_clicked()
{
//    QString filetype = "bmp";

    //获取全屏截图

//    QPixmap fullScreen = QPixmap::grabWindow(QApplication::desktop()->winId());

    QList<QScreen *> list_screen =  QGuiApplication::screens(); //可能电脑接了多个屏幕
    QPixmap fullScreen;
    fullScreen = list_screen.at(screencount-1)->grabWindow(0); //at 0 at 1选择多屏

    QImage screen = fullScreen.toImage();
    QColor curpixel;

    int width = screen.width();
    int height = screen.height();
    bool hasFind = false;

    qDebug() << QString("Resolution: %1 x %2").arg(width).arg(height);

    int x=0;
    int y=0;
    int firstx=0;
    int firsty=0;
    int lastx=0;
    int lasty=0;
    for(y=0;y<height;y++)
    {
        for(x=0;x<width;x++)
        {
            curpixel = screen.pixelColor(x,y);
            if(curpixel==QColor(255,0,0))
            {
                if(!hasFind)
                {
                    firstx = x;
                    firsty = y;
                    hasFind = true;
                }
                lastx = x;
                lasty = y;
            }
        }
    }
//    qDebug() << QString("Find FirstPixel Pos:(%1,%2)").arg(firstx).arg(firsty);
//    qDebug() << QString("Find LastPixel  Pos:(%1,%2)").arg(lastx).arg(lasty);
//    qDebug() << QString("(%1, %2, %3)").arg(curpixel.red()).arg(curpixel.green()).arg(curpixel.blue());
    QRect screen1;
    screen1 = QApplication::desktop()->screenGeometry(screencount-1);
    if(firstx<screen1.width()/2 && firsty<screen1.height()/2 && (lastx-firstx)>200 && (lasty-firsty)>200)
    {
        this->setGeometry(screen1.x()+firstx,screen1.y()+firsty,lastx-firstx,lasty-firsty);
//        qDebug() << QString("Size:(%1,%2)").arg(lastx-firstx).arg(lasty-firsty);
//        qDebug() << QString("Find FirstPixel Pos:(%1,%2)").arg(firstx).arg(firsty);
//        qDebug() << QString("Find LastPixel  Pos:(%1,%2)").arg(lastx).arg(lasty);
        return true;
    }
    else
    {
//        this->setGeometry(screen1.x(),screen1.y(),1000,800);
        return false;
    }
//    QPoint p = this->pos();
//    this->move(x,y);

//    //打开文件对话框

//    QString fileName = QFileDialog::getSaveFileName(this, "文件另存为","",QString("*.")+filetype);

//    //保存截图

//    std::string str = filetype.toStdString();
//    const char* ch_filetype = str.c_str();

//    fullScreen.save(fileName,ch_filetype);
}

void BSSamplerWidget::TaskBarShow()
{
    QString a="Shell_TrayWnd";
    QString b="Button";
    HWND WndHandle;
    // 获取任务栏的窗口句柄，并显示
    WndHandle = FindWindow(a.toStdWString().c_str() , NULL);
    ShowWindow(WndHandle, SW_SHOW);
    HWND WndHandle2;
    // 获取开始按钮句柄,并显示
    WndHandle2 = FindWindow(b.toStdWString().c_str() , NULL);
    ShowWindow(WndHandle2, SW_SHOW);
}

void BSSamplerWidget::TaskBarHide()
{
    QString a="Shell_TrayWnd";
    QString b="Button";
    HWND WndHandle;
    // 获取任务栏的窗口句柄，并隐藏
    WndHandle = FindWindow(a.toStdWString().c_str() , NULL);
    ShowWindow(WndHandle, SW_HIDE);
    HWND WndHandle2;
    // 获取开始按钮句柄,并隐藏
    WndHandle2 = FindWindow(b.toStdWString().c_str() , NULL);
    ShowWindow(WndHandle2, SW_HIDE);
}

void BSSamplerWidget::Capture()
{
    if(ishide==false)
        return;
    captureTimer.stop();
//    QString filetype = "bmp";
    QList<QScreen *> list_screen =  QGuiApplication::screens(); //可能电脑接了多个屏幕
    QPixmap fullScreen;
    int awidth = list_screen.at(screencount-1)->availableGeometry().width();
//    int aheight = list_screen.at(screencount-1)->availableGeometry().height();
    fullScreen = list_screen.at(screencount-1)->grabWindow(0,awidth-100,0,100,100); //at 0 at 1选择多屏

    QImage screen = fullScreen.toImage();
    QColor curpixel;

    int width = screen.width();
    int height = screen.height();
    bool Trigger = true;
    int x=0;
    int y=0;
    for(y=0;y<height;y++)
        for(x=0;x<width;x++)
            if(screen.pixelColor(x,y)!=screen.pixelColor(0,0))
                Trigger = false;
    if(Trigger)
        findtarget();
//    //打开文件对话框

//    QString fileName = QFileDialog::getSaveFileName(this, "文件另存为","",QString("*.")+filetype);

//    //保存截图

//    std::string str = filetype.toStdString();
//    const char* ch_filetype = str.c_str();

//    fullScreen.save(fileName,ch_filetype);
    captureTimer.start(CAPTURE_SCAN_INTERVAL);
}

void BSSamplerWidget::on_openClientButton_clicked()
{
    if(ui->openClientButton->text() == QString("关闭客户端"))
    {
        m_client->close();
        ui->openClientButton->setText(QString("打开客户端"));
        return;
    }
    m_client->connectToHost("192.168.3.1",777);
    ui->openClientButton->setText(QString("关闭客户端"));
    char* data="client:hello server,I am client.";
    m_client->write(data);
//    connect(m_client,SIGNAL(readyRead()),this,SLOT(printServerMessage()));
}

void BSSamplerWidget::printServerMessage()
{
    QByteArray msg= m_client->readAll();
    qDebug() << msg;
    qDebug() << msg.toDouble();
//    myHelper::showMessageBoxInfo(msg);
//    QMessageBox::information(this,"来自服务端的消息",msg);
}
