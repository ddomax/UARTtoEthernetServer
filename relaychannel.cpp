#include "relaychannel.h"
#include <QTime>

RelayChannel::RelayChannel(QObject *parent) : QObject(parent)
{
    serial = new QSerialPort();
    serialRxTimer = new QTimer();
    networkRxTimer = new QTimer();
    connect(serialRxTimer, SIGNAL(timeout()), this, SLOT(serialToNetwork()));
    connect(networkRxTimer, SIGNAL(timeout()), this, SLOT(networkToSerial()));
    qDebug() << QString("RelayChannel created!");
}

RelayChannel::~RelayChannel()
{
    qDebug() << QString("RelayChannel destroy process started!");
    serialRxTimer->stop();
    networkRxTimer->stop();
    serialRxTimer->deleteLater();
    networkRxTimer->deleteLater();
    serial->clear();
    serial->close();
    serial->deleteLater();
    if(m_socket!=NULL)
    {
        m_socket->close();
        m_socket->deleteLater();
    }
    if(m_server!=NULL)
    {
        m_server->close();
        m_server->deleteLater();
    }
    if(m_udpsocket!=NULL)
    {
        m_udpsocket->close();
        m_udpsocket->deleteLater();
    }
    qDebug() << QString("RelayChannel destroyed!");
}

void RelayChannel::setOpmode(MRelayChannel::opmode_t targetOpmode)
{
    chnlStatus.currentOpmode = targetOpmode;
}

void RelayChannel::openChannel()
{
    if(!serial->open(QIODevice::ReadWrite)){
        qDebug() << tr("串口打开失败！");
        return;
    }
    serialRxTimer->start(serialRxTimerInterval);
    switch (chnlStatus.currentOpmode) {
    case MRelayChannel::TCPCLIENT:
    {
        m_socket = new QTcpSocket();
        m_socket->connectToHost(chnlStatus.remoteAddress,chnlStatus.remotePortNum); //"192.168.3.1",777
        char* data="client:hello server,I am client.";
        m_socket->write(data);
        networkRxTimer->start(networkRxTimerInterval);
        break;
    }
    case MRelayChannel::TCPSERVER:
    {
        m_server = new QTcpServer();
        m_server->listen(QHostAddress::Any,chnlStatus.localPortNum);
        //连接信号与槽，若有连接过来则接收连接
        connect(m_server,SIGNAL(newConnection()),this,SLOT(acceptConnection()));
        break;
    }
    case MRelayChannel::UDP:
    {
        m_udpsocket = new QUdpSocket();
        //绑定接收地址和端口
        m_udpsocket->bind(QHostAddress(chnlStatus.localAddress),chnlStatus.localPortNum);
        networkRxTimer->start(networkRxTimerInterval);
    }
    default:
        break;
    }
    chnlStatus.currentLinkStatus = MRelayChannel::CONNECTED;
}

//接收客户端连接
void RelayChannel::acceptConnection()
{
    m_socket=m_server->nextPendingConnection();
    networkRxTimer->start(networkRxTimerInterval);
}

void RelayChannel::networkToSerial()
{
    static int returnCnt = 0;
//    qDebug() << m_udpsocket->bytesAvailable();
    if(chnlStatus.currentOpmode==MRelayChannel::UDP)
    {
        if(m_udpsocket->bytesAvailable()<packSize && returnCnt < packSpiltTime)
        {
            returnCnt++;
            return;
        }
        returnCnt = 0;
        if(m_udpsocket->bytesAvailable()>0)
        {
            chnlStatus.recNum+=m_udpsocket->pendingDatagramSize(); //!
            //UDP Rx to local
            // when data comes in
            QByteArray dataBuf;
            dataBuf.resize(m_udpsocket->pendingDatagramSize());

            // qint64 QUdpSocket::readDatagram(char * data, qint64 maxSize,
            //                 QHostAddress * address = 0, quint16 * port = 0)
            // Receives a datagram no larger than maxSize bytes and stores it in data.
            // The sender's host address and port is stored in *address and *port
            // (unless the pointers are 0).

            m_udpsocket->readDatagram(dataBuf.data(), dataBuf.size());
            serial->write(dataBuf);
        }
    }
    else
    {
        if(m_socket->bytesAvailable()<packSize && returnCnt < packSpiltTime)
        {
            returnCnt++;
            return;
        }
        returnCnt = 0;
        if(m_socket->bytesAvailable()>0)
        {
            chnlStatus.recNum+=m_socket->bytesAvailable();
            QByteArray dataBuf = m_socket->readAll();
            serial->write(dataBuf);
        }
    }
}

void RelayChannel::serialToNetwork()
{
    static int returnCnt = 0;
    if(serial->bytesAvailable()<packSize && returnCnt < packSpiltTime)
    {
        returnCnt++;
        return;
    }
    returnCnt = 0;
    if(serial->bytesAvailable()>0)
    {
        chnlStatus.recNum+=serial->bytesAvailable();
        if(chnlStatus.currentOpmode==MRelayChannel::UDP)
        {
            //UDP TX to remote
            QByteArray dataBuf = serial->readAll();
            m_udpsocket->writeDatagram(dataBuf, QHostAddress(chnlStatus.remoteAddress), chnlStatus.remotePortNum);
        }
        else
        {
            QByteArray dataBuf = serial->readAll();
            m_socket->write(dataBuf);
        }
    }
}
