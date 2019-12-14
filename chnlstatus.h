#ifndef CHNLSTATUS_H
#define CHNLSTATUS_H
#include<QString>

namespace MRelayChannel {
    enum opmode_e { TCPCLIENT, TCPSERVER, UDP };
    typedef enum opmode_e opmode_t;

    enum linkstatus_e { CLOSED, DISCONNECTED, CONNECTED };
    typedef enum linkstatus_e linkstatus_t;
}


class ChnlStatus
{
public:
    ChnlStatus();
    MRelayChannel::opmode_t currentOpmode;
    MRelayChannel::linkstatus_t currentLinkStatus = MRelayChannel::CLOSED;
    QString localAddress;
    QString remoteAddress;
    int localPortNum;
    int remotePortNum;
    long long int recNum = 0;

    int portBoxIndex = 0;
    int BaudBoxIndex = 3;
    
};

#endif // CHNLSTATUS_H
