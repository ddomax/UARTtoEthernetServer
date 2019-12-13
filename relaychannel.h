#ifndef RELAYCHANNEL_H
#define RELAYCHANNEL_H

#include <QObject>
#include <QWidget>
#include <QDebug>
#include <QTimer>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTcpSocket>
#include <QTcpServer>
#include <QUdpSocket>
#include <chnlstatus.h>


class RelayChannel : public QObject
{
    Q_OBJECT
public:
    explicit RelayChannel(QObject *parent = nullptr);
    ~RelayChannel();
    QSerialPort *serial;
    QTcpSocket *m_socket = NULL;
    QTcpServer *m_server = NULL;
    QUdpSocket *m_udpsocket = NULL;
    QTimer *serialRxTimer;
    QTimer *networkRxTimer;
    void setOpmode(MRelayChannel::opmode_t targetOpmode);
    void openChannel();
    ChnlStatus chnlStatus;
private:
    static const int networkRxTimerInterval = 5;
    static const int serialRxTimerInterval = 5;
    static const int packSize = 100;
    static const int packSpiltTime = 10;
private slots:
    void networkToSerial();
    void serialToNetwork();
    void acceptConnection();


signals:

public slots:
};

#endif // RELAYCHANNEL_H
