/***********************************************************
**
** class: BroadcastCaliStatus
**
** 作者：lth
** 日期：2017-07-25
**
** 使用 UDP 广播标定状态
**
************************************************************/

#ifndef BROADCASTCALISTATUS_H
#define BROADCASTCALISTATUS_H

#include <QObject>
#include "SocketClient.h"


class BroadcastCaliStatus :public QObject
{
    Q_OBJECT

public:
    BroadcastCaliStatus();
    ~BroadcastCaliStatus();

private:
    QUdpSocket  *m_udpClient;

private:
    void broadcastCaliStatus(QByteArray data);
    void bind(QUdpSocket *udpSocket);

private slots:
    void dataPacketReady(SocketPacket socketPacket);
    void readDatagram();
};

#endif // BROADCASTCALISTATUS_H
