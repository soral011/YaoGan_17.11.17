#include "BroadcastCaliStatus.h"
#include "MyDebug.cpp"
#include "SocketClient.h"

BroadcastCaliStatus::BroadcastCaliStatus()
{
    m_udpClient = new QUdpSocket();
    this->bind(m_udpClient);

    SocketClient *client = SocketClient::getInstance();
    connect(client, SIGNAL(dataPacketReady(SocketPacket)),
            this, SLOT(dataPacketReady(SocketPacket)));

}

BroadcastCaliStatus::~BroadcastCaliStatus()
{
    delete m_udpClient;
}

void BroadcastCaliStatus::dataPacketReady(SocketPacket packet)
{

    if(packet.dataType == NET_DATA_TYPE_CALI_STATUS)
    {

        if(packet.data.count() != sizeof(NetCalibrationStatus))
        {
//            MY_DEBUG("packet.data.count() != sizeof(NetCalibrationStatus)");
            return;
        }
        broadcastCaliStatus(packet.data);

//        NetCalibrationStatus caliStatus;
//        memcpy(&caliStatus, packet.data.data(), sizeof(NetCalibrationStatus));
//        QString text(caliStatus.text);
//        if(text.contains("光谱") && text.contains("NO"))
//        {
//            m_udpClient
//        }
    }
}

void BroadcastCaliStatus::bind(QUdpSocket *udpSocket)
{
    for(int i = 0; i<20 ;i++)
    {
        if(udpSocket->bind(QHostAddress("127.0.0.1"),
                           NET_UDP_PORT + i*NET_PORT_SPACE + 8,
                           QUdpSocket::ShareAddress |
                           QUdpSocket::ReuseAddressHint))//理论上可以采用16bit的端口号
        {
            connect(udpSocket,SIGNAL(readyRead()),this, SLOT(readDatagram()));
//            MY_DEBUG("\ni = "<<i);
//            MY_DEBUG("udpClient->state() = "<<udpSocket->state());
            break;
        }
        else
        {
            qDebug()<<i<<". udpClient binding failed !! ";
        }
    }
}

void BroadcastCaliStatus::broadcastCaliStatus(QByteArray data)
{
    for(int i = 0;i<20;i++)
    {
        m_udpClient->writeDatagram(data, QHostAddress("127.0.0.1"),
                                   NET_UDP_PORT + i*NET_PORT_SPACE);
    }
    g_msleep(5);
}

void BroadcastCaliStatus::readDatagram()
{
    QByteArray datagram;
    QHostAddress sender_ip;
    quint16 sender_port = -1;
    do{
        datagram.resize(m_udpClient->pendingDatagramSize());
        m_udpClient->readDatagram(datagram.data(),
                                  datagram.size(),
                                  &sender_ip,
                                  &sender_port);
//        qDebug()<<"datagram = "<<datagram;
        if(datagram.count() == sizeof(NetIO_Control))
        {
//            MY_DEBUG("packet.data.count() != sizeof(NetCalibrationStatus)");
            SocketClient *client = SocketClient::getInstance();
            client->send(datagram, NET_DATA_TYPE_IO_CONTROL);
        }
//        parseData(datagram);

     }while(m_udpClient->hasPendingDatagrams());
}
