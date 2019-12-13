#ifndef RELAYCHANNEL_H
#define RELAYCHANNEL_H

#include <QObject>
#include <QWidget>

class RelayChannel : public QObject
{
    Q_OBJECT
public:
    explicit RelayChannel(QObject *parent = nullptr);

signals:

public slots:
};

#endif // RELAYCHANNEL_H