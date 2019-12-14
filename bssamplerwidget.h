#ifndef BSSAMPLERWIDGET_H
#define BSSAMPLERWIDGET_H

#include <QWidget>
#include <QDebug>
#include <QTimer>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTcpSocket>
#include <QTcpServer>
#include <QMap>
#include <QDialog>
#include <relaychannel.h>

namespace Ui {
class BSSamplerWidget;
}

class BSSamplerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BSSamplerWidget(QWidget *parent = 0);
    ~BSSamplerWidget();
    static void TaskBarShow();
    static void TaskBarHide();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_clearButton_clicked();
    void on_sendButton_clicked();
    void on_openButton_clicked();
    void on_closeButton_clicked();
    void Read_Data();
    void Read_Line();
    void on_refreshButton_clicked();
    void on_stdweightcalButton_clicked();
    void on_offsetcalButton_clicked();
    void clipboard_changed();
    void on_hideButton_clicked();
    bool on_screenshotButton_clicked();
    void Capture();
    void on_openClientButton_clicked();
    void printServerMessage();

    void on_btnMenu_Max_clicked();
    void on_cboxStyle_currentIndexChanged(const QString &text);
    void on_channelBox_currentIndexChanged();
    void on_configNetButton_clicked();
    void on_readNetworkStatusButton_clicked();
    void on_currentSideButton_clicked();
    void on_fullScreenButton_clicked();
    void on_configVcpButton_clicked();

private:
    const static int MAX_CHNL_NUM = 10;
    Ui::BSSamplerWidget *ui;
    QSerialPort *serial;
    QClipboard *board;
    QTcpSocket *m_client;
    QTcpSocket ctrClient;
    RelayChannel *relayChannelPool[MAX_CHNL_NUM] = {NULL};
    QWindow *m_window;
    QWidget *m_widget;
    int activeChannelIndex = 0;
    QTimer dataTimer;
    QTimer captureTimer;
    const static int RECORD_DATA_LEN = 8;
    const static int CAPTURE_SCAN_INTERVAL = 200;
    double offset_val = -98503.75;
    double k_val = 0.0000025402; //测力计
//    double offset_val = -137669.14;
//    double k_val = 0.0000373440; //电子称
    double data_record[RECORD_DATA_LEN];
    void refresh_serialPorts();
    void setup_CustomPlot();
    double data_average();
    void findtarget();
    void enableSettings();
    void disableSettings();
    void initNetwork();
    void linkMstsc(const QString path,const QStringList argc);
    int screencount;
    bool ishide = false;
    bool islocal = true;
    bool islinked = false;

    void initStyle();   //初始化无边框窗体
    void initForm();    //初始化窗体数据

    bool m_max;           //是否处于最大化状态
    QRect locatRect;     //鼠标移动窗体后的坐标位置

    QMap<QString, QString> mapStyle;
};

#endif
