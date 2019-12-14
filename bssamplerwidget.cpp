#include "bssamplerwidget.h"
#include "ui_bssamplerwidget.h"
#include "api/myhelper.h"
#include<QtNetwork>
//#include<iphlpapi.h>

BSSamplerWidget::BSSamplerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BSSamplerWidget)
{
    ui->setupUi(this);
    setup_CustomPlot();

    board = QApplication::clipboard();
//    connect(board, SIGNAL(dataChanged()), this, SLOT(clipboard_changed()));

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

    QStringList channelindex;
    channelindex<<"1"<<"2"<<"3"<<"4"<<"5"<<"6"<<"7"<<"8"<<"9"<<"10";
    ui->channelBox->addItems(channelindex);

    QStringList opmodetext;
    opmodetext<<"TCPClient"<<"TCPServer"<<"UDP";
    ui->opmodeBox->addItems(opmodetext);

    QStringList serialmodetext;
    serialmodetext<<"RS232"<<"RS422"<<"RS485"<<"TTL";
    ui->serialModeBox->addItems(serialmodetext);

    refresh_serialPorts();
    //设置波特率下拉菜单默认显示第三项
    ui->BaudBox->setCurrentIndex(2);
    //关闭发送按钮的使能
//    ui->sendButton->setEnabled(false);

    //   setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint);

    this->initStyle();
    this->initForm();
    //   myHelper::moveFormToCenter(this);
    screencount = QApplication::desktop()->screenCount();
    QRect screen1;
    screen1 = QApplication::desktop()->screenGeometry(screencount-1);
    this->setGeometry(screen1.x(),screen1.y(),800,1024);
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
//    img->load(":/HNSDFZ.jpg");
    img->load(":/UESTC.jpg");
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
    ui->cboxStyle->setCurrentIndex(7);
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

    ui->customPlot->xAxis->setLabel("Time(s)");
    ui->customPlot->yAxis->setLabel("Speed(B/s)");

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
    dataTimer.start(20); // Interval 0 means to refresh as fast as possible
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
//    ui->textEdit->clear();
    ui->customPlot->graph(0)->data().data()->clear();
//    ui->customPlot->graph(1)->data().data()->clear();
}
//发送数据
void BSSamplerWidget::on_sendButton_clicked()
{
//    serial->write(ui->textEdit_2->toPlainText().toLatin1());
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
//        ui->textEdit->insertPlainText(buf);
//        ui->textEdit->append(buf);
    }
    buf.clear();
}

void BSSamplerWidget::Read_Line()
{
    RelayChannel* curChannel = relayChannelPool[activeChannelIndex];
    if(curChannel==NULL)
    {
        return;
    }
    static long long int lastRecNum = 0;
    static QTime time(QTime::currentTime());
    // calculate two new data points:
    double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
    static double lastPointKey = 0;
    if (key-lastPointKey > 0.2) // at most add point every 2 ms
    {
        // add data to lines:
        ui->customPlot->graph(0)->addData(key, (curChannel->chnlStatus.recNum-lastRecNum)/(key-lastPointKey));
        // rescale value (vertical) axis to fit the current data:
        ui->customPlot->graph(0)->rescaleValueAxis();
//        ui->customPlot->graph(1)->rescaleValueAxis(true);
        lastRecNum = curChannel->chnlStatus.recNum;
        lastPointKey = key;
    }
    // make key axis range scroll with the data (at a constant range size of 8):
    ui->customPlot->xAxis->setRange(key, 8, Qt::AlignRight);
    ui->customPlot->replot();

    // calculate frames per second:
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 2) // average fps over 2 seconds
    {

      qDebug() << QString("%1 FPS, Total Data points: %2, Buffer: %3")
                  .arg(frameCount/(key-lastFpsKey), 0, 'f', 1)
                  .arg(ui->customPlot->graph(0)->data()->size())
                  .arg(m_client->bytesAvailable());
      lastFpsKey = key;
      frameCount = 0;
    }
}

void BSSamplerWidget::on_channelBox_currentIndexChanged()
{
    activeChannelIndex = ui->channelBox->currentIndex();
    if(relayChannelPool[activeChannelIndex]==NULL)
    {
        enableSettings();

        qDebug() << QString("Current channel is closed!No refresh is needed");
        return;
    }
    RelayChannel *m_relayChannel = relayChannelPool[activeChannelIndex];
    ui->PortBox->setCurrentIndex(m_relayChannel->chnlStatus.portBoxIndex);
    ui->PortBox->setCurrentText(m_relayChannel->serial->portName());
    ui->StopBox->setCurrentIndex(m_relayChannel->chnlStatus.StopBoxIndex);
    ui->ParityBox->setCurrentIndex(m_relayChannel->chnlStatus.ParityBoxIndex);
    ui->BaudBox->setCurrentIndex(m_relayChannel->chnlStatus.BaudBoxIndex);
    ui->serialModeBox->setCurrentIndex(m_relayChannel->chnlStatus.serialModeBoxIndex);
    ui->opmodeBox->setCurrentIndex(m_relayChannel->chnlStatus.currentOpmode);
    ui->localAddressEdit->setText(m_relayChannel->chnlStatus.localAddress);
    ui->remoteAddressEdit->setText(m_relayChannel->chnlStatus.remoteAddress);
    ui->localPortEdit->setText(QString("%1").arg(m_relayChannel->chnlStatus.localPortNum));
    ui->remotePortEdit->setText(QString("%1").arg(m_relayChannel->chnlStatus.remotePortNum));
    ui->localNetmaskEdit->setText(m_relayChannel->chnlStatus.localNetmask);
    ui->remoteNetmaskEdit->setText(m_relayChannel->chnlStatus.remoteNetmask);
    disableSettings();
}

void BSSamplerWidget::on_openButton_clicked()
{
    if(relayChannelPool[activeChannelIndex]!=NULL)
    {
        qDebug() << QString("Channel already exists!");
        return;
    }
    relayChannelPool[activeChannelIndex] = new RelayChannel();
    RelayChannel *m_relayChannel = relayChannelPool[activeChannelIndex];
    if(m_relayChannel->serial == NULL)
    {
        qDebug() << QString("Serial object creation failed!");
        return;
    }
    //设置串口名
    m_relayChannel->serial->setPortName(ui->PortBox->currentText());
    m_relayChannel->chnlStatus.portBoxIndex = ui->PortBox->currentIndex();
    //设置波特率
    m_relayChannel->serial->setBaudRate(ui->BaudBox->currentText().toInt());
    m_relayChannel->chnlStatus.BaudBoxIndex = ui->BaudBox->currentIndex();
    //设置数据位数
    switch(ui->BitNumBox->currentIndex())
    {
    case 8: m_relayChannel->serial->setDataBits(QSerialPort::Data8); break;
    default: break;
    }
    //设置奇偶校验
    switch(ui->ParityBox->currentIndex())
    {
    case 0: m_relayChannel->serial->setParity(QSerialPort::NoParity); break;
    default: break;
    }
    m_relayChannel->chnlStatus.ParityBoxIndex = ui->ParityBox->currentIndex();
    //设置停止位
    switch(ui->StopBox->currentIndex())
    {
    case 1: m_relayChannel->serial->setStopBits(QSerialPort::OneStop); break;
    case 2: m_relayChannel->serial->setStopBits(QSerialPort::TwoStop); break;
    default: break;
    }
    m_relayChannel->chnlStatus.StopBoxIndex = ui->StopBox->currentIndex();
    //设置流控制
    m_relayChannel->serial->setFlowControl(QSerialPort::NoFlowControl);
    //Set Ethernet protocol mode
    m_relayChannel->chnlStatus.currentOpmode = static_cast<MRelayChannel::opmode_t>(ui->opmodeBox->currentIndex());
    m_relayChannel->chnlStatus.remoteAddress = ui->remoteAddressEdit->text();
    m_relayChannel->chnlStatus.remotePortNum = ui->remotePortEdit->text().toInt();
    m_relayChannel->chnlStatus.remoteNetmask = ui->remoteNetmaskEdit->text();
    m_relayChannel->chnlStatus.localAddress = ui->localAddressEdit->text();
    m_relayChannel->chnlStatus.localPortNum = ui->localPortEdit->text().toInt();
    m_relayChannel->chnlStatus.localNetmask = ui->localNetmaskEdit->text();

    //Set Serial Port Mode
    m_relayChannel->chnlStatus.serialModeBoxIndex = ui->serialModeBox->currentIndex();

    //打开Channel
    m_relayChannel->openChannel();
    if(m_relayChannel->chnlStatus.currentLinkStatus == MRelayChannel::CLOSED)
    {
        qDebug() << QString("Channel Open Failed! DeleteLater");
        relayChannelPool[activeChannelIndex]->deleteLater();
        relayChannelPool[activeChannelIndex] = NULL;
        return;
    }
    //关闭设置菜单使能
    disableSettings();
    //连接信号槽
//    QObject::connect(serial, &QSerialPort::readyRead, this, &BSSamplerWidget::Read_Line);
//    QObject::connect(serial, &QSerialPort::readyRead, this, &BSSamplerWidget::Read_Data);

}

void BSSamplerWidget::on_closeButton_clicked()
{
    relayChannelPool[activeChannelIndex]->deleteLater();
    relayChannelPool[activeChannelIndex] = NULL;
    enableSettings();
}

void BSSamplerWidget::enableSettings()
{
    //恢复设置使能
    ui->PortBox->setEnabled(true);
    ui->BaudBox->setEnabled(true);
    ui->BitNumBox->setEnabled(true);
    ui->ParityBox->setEnabled(true);
    ui->StopBox->setEnabled(true);
    ui->serialModeBox->setEnabled(true);
    ui->refreshButton->setEnabled(true);
    ui->openButton->setEnabled(true);
    ui->closeButton->setEnabled(false);
//    ui->sendButton->setEnabled(false);
}

void BSSamplerWidget::disableSettings()
{
    //关闭设置菜单使能
    ui->PortBox->setEnabled(false);
    ui->BaudBox->setEnabled(false);
    ui->BitNumBox->setEnabled(false);
    ui->ParityBox->setEnabled(false);
    ui->StopBox->setEnabled(false);
    ui->serialModeBox->setEnabled(false);
    ui->refreshButton->setEnabled(false);
    ui->openButton->setEnabled(false);
    ui->closeButton->setEnabled(true);
//    ui->sendButton->setEnabled(true);
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
//    k_val=ui->stdweightEdit->text().toDouble() / (data_average()-offset_val);
//    qDebug() << QString("斜率校准成功！斜率: %1").arg(k_val, 0, 'f', 10);
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

void BSSamplerWidget::initNetwork()
{
//    ui.comboBox->clear();

    QString text;
    QTextStream out(&text);
    QList<QNetworkInterface> ifaceList = QNetworkInterface::allInterfaces();
    for (int i = 0; i < ifaceList.count(); i++)
    {
        QNetworkInterface var = ifaceList.at(i);
        if(var.humanReadableName() != QString("以太网"))
        {
            continue;
        }
        out << QString("########## 设备%1 ############").arg(i) << endl;
        out << QString("接口名称：") << var.humanReadableName() << endl;
        out << QString("设备名称：") << var.name() << endl;
        out << QString("硬件地址：") << var.hardwareAddress() << endl;

//        ui.comboBox->addItem(var.name());

        out << QString("IP地址列表：") << endl;
        // 读取一个IP地址的关联信息列表
        QList<QNetworkAddressEntry> entryList = var.addressEntries();
        for(int j = 0; j < entryList.count(); j++)
        {
            QNetworkAddressEntry entry = entryList.at(j);
            out << QString("%1_地址：").arg(j) << endl;
            out << QString(" IP地址：") << entry.ip().toString() << endl;
            out << QString(" 子网掩码：") << entry.netmask().toString() << endl;
            out << QString(" 广播地址：") << entry.broadcast().toString() << endl;
        }
    }

    ui->networkEdit->setPlainText(text);
}

void BSSamplerWidget::on_configNetButton_clicked()
{
//    QNetworkInterface m_interface;
//    QString name = m_interface.humanReadableName();
    QString name = "以太网";
    QString ip = ui->localAddressEdit->text();
    QString netmask = ui->localNetmaskEdit->text();

    QProcess cmd(this);

    // netsh interface ipv4 set address name = "以太网" source = static
    // address = 192.168.0.106 mask = 255.255.255.0 gateway = 192.168.0.1 gwmetric = 0
    QString command = "netsh interface ipv4 set address name = "
        + name + " source = static address = " + ip
        + " mask = " + netmask;
    qDebug() << command;
    cmd.start(command);
    cmd.waitForFinished();
    QByteArray qbt = cmd.readAllStandardOutput();
    QString msg = QString::fromLocal8Bit(qbt);
    qDebug() << msg;
    qDebug() << QString("Network configuration done!");
}

void BSSamplerWidget::on_readNetworkStatusButton_clicked()
{
    initNetwork();
}

void BSSamplerWidget::on_currentSideButton_clicked()
{
    if(ui->currentSideButton->text()==QString("打开服务器端配置界面"))
    {
        ui->currentSideButton->setText(QString("关闭服务器端配置界面"));
        const QString path = "C:\\Windows\\System32\\mstsc.exe";
        const QStringList argc = QStringList() << "C:\\Default.rdp" << "/v:" << ui->remoteAddressEdit->text();
        linkMstsc(path,argc);
    }
    else
    {
        m_window->close();
        m_widget->close();

        ui->currentSideButton->setText(QString("打开服务器端配置界面"));
    }


}

void BSSamplerWidget::linkMstsc(const QString path,const QStringList argc)
{
    //1、启动进程
    QProcess *pProcess = new QProcess(this);
    pProcess->start(path, argc);

//    QWidget *m_widget = NULL;
//    m_widget = new QWidget();
//    m_widget->setGeometry(0,0,1024,768);
//    m_widget->show();

    //2、延时，这个很重要，一定要大于50
//    Sleep(2000);

//    //3、搜索匹配参数的窗口
//    HWND childHwnd = FindWindow(L"TscShellContainerClass", NULL);


//    //4、设置父窗口
//    HWND parentHwnd = (HWND)m_widget->winId();
//    SetParent(childHwnd, parentHwnd);

//    //5、设置窗口位置
//    SetWindowPos(childHwnd, HWND_TOP, 0, 0, 500, 500, SWP_FRAMECHANGED);
//    Sleep(1000);
    WId wid = 0;
    while((WId)FindWindow(L"TscShellContainerClass", NULL)==0);
    Sleep(500);
    wid = (WId)FindWindow(L"TscShellContainerClass", NULL);
//    QWindow *m_window;
    m_window = QWindow::fromWinId(wid);
    m_window->setFlags(m_window->flags() | Qt::CustomizeWindowHint | Qt::WindowTitleHint); //

//    QWidget *m_widget;
    m_widget = QWidget::createWindowContainer(m_window);
    m_widget->setMinimumSize(1024, 768);
    m_widget->setWindowFlags(Qt::WindowMinimizeButtonHint); //Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint
    m_widget->setWindowTitle(QString("串口服务器端配置图形界面"));
    m_widget->show();
}

void BSSamplerWidget::on_configVcpButton_clicked()
{
    QProcess process(this);
    process.startDetached("D:\\VCP\\vspdconfig.exe");
}

void BSSamplerWidget::on_fullScreenButton_clicked()
{
    showFullScreen();
}
