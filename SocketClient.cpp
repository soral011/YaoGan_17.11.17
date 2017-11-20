#include "SocketClient.h"
#include "Constant.h"
#include "MyDebug.cpp"
#include <QTimer>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define PACKET_SIZE  (1024 * 500)

SocketClient::SocketClient()
{
    m_dataSocket  = NULL;
    m_cmdSocket   = NULL;
    m_shakeSocket = NULL;
    m_isConnecting = true;

    //维护客户端也需要监听？？？
    //不监听则无法接受服务器端发来的信息!!!
    m_udpClient = new QUdpSocket();
    bind(m_udpClient);

    connect(&m_findUdpTimer,SIGNAL(timeout()),
            this,SLOT(findUDPServer()));
    m_findUdpTimer.start(600);
//    QTimer::singleShot(500,this,SLOT(findUDPServer()));

    connect(&m_diagnoseTimer, SIGNAL(timeout()),
            this, SLOT(socketTimeOut()));

    connect(&m_sendFileTimer, SIGNAL(timeout()),
            this, SLOT(sendFilePacket()));

    if(NET_SHAKE_HAND_ENABLED == true)
    {
        QTimer::singleShot(2000, this, SLOT(shakeHand()));
    }
//    qDebug()<<"findUDPServer() !!!";
}

//使用单例模式
SocketClient *SocketClient::getInstance()
{
    static SocketClient instance; //局部静态变量，若定义为指针，则需要手动释放内容
    return &instance;
}

bool SocketClient::isConnected()
{
    if(m_cmdSocket == NULL || (m_cmdSocket !=  NULL &&
       m_cmdSocket->state() !=  QTcpSocket::ConnectedState))
    {
        return false;
    }
    else if(m_dataSocket == NULL || (m_dataSocket !=  NULL &&
        m_dataSocket->state() !=  QTcpSocket::ConnectedState))
    {
        return false;
    }
    else
    {
        return true;
    }
}

void SocketClient::bind(QUdpSocket *udpSocket)
{
    for(int i = 0; i<20 ;i++)
    {
        // 有时一台电脑同时运行多个YaoGan软件，防止使用相同端口号
        // 作为客户端可以使用任意端口号，服务器则必须使用约定端口号
        srand((int)time(NULL)); //每次执行种子不同，生成不同的随机数
        int n = qrand() % 1000;
        MY_DEBUG("n="<<n);
        if(udpSocket->bind(NET_UDP_PORT + i*NET_PORT_SPACE + n,
           QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))//理论上可以采用16bit的端口号
        {
            connect(udpSocket,SIGNAL(readyRead()),this, SLOT(readDatagram()));
            MY_DEBUG("\ni = "<<i);
            MY_DEBUG("udpClient->state() = "<<udpSocket->state());
            break;
        }
        else
        {
            MY_LOG(i<<". udpClient binding failed !! ");
        }
    }
}

void SocketClient::connectTcpServer()
{
    for(int i = 0; i < 3; i++)
    {
        QTcpSocket *socket = new QTcpSocket(this);
        socket->connectToHost(m_tcpServerIP,m_tcpServerPort);

        switch(i)
        {
        case 0:
            m_dataSocket  = socket;
        case 1:
            m_cmdSocket   = socket;
        case 2:
            m_shakeSocket = socket;
        }

        /* 确保当前Socket连接成功，再连接下一个Socket，
         * 同时connectToHost有可能不会按照代码的先后顺
         * 序连接成功 2016-07-12
         */
        int secs = 2000;
        int interval = 100;
        while(true)
        {
            if(socket->state() == QTcpSocket::ConnectedState)
            {
                break;
            }
            else
            {
                g_msleep(interval);
                secs = secs - interval;
//                MY_DEBUG("secs = "<<secs);
            }

            if(secs <=  0)
            {
                socket->abort();//如果在这里没有连接成功则放弃连接
                MY_DEBUG("connection failed.");
                break;
            }
        }
    }

    QTimer::singleShot(10,this,SLOT(isAllConnected()));

}

void SocketClient::isAllConnected()
{
    QList<QTcpSocket*> socketList;
    socketList<<m_dataSocket
              <<m_cmdSocket
              <<m_shakeSocket;
    bool isConnected = true;
    foreach (QTcpSocket *socket, socketList)
    {
        if(socket->state() == QTcpSocket::ConnectedState)
        {
            connect(socket, SIGNAL(readyRead()), this, SLOT(OnReadyRead()));
            connect(socket, SIGNAL(disconnected()), this, SLOT(OnDisconnected()));
        }
        else
        {
            isConnected = false;
            break;
        }
    }

    if(isConnected == true)
    {
        MY_DEBUG("3 socket connected successful!");
        m_findUdpTimer.stop();
        emit hasClient(true);
        m_isConnecting = false;

        if(NET_SHAKE_HAND_ENABLED == true)
        {
//            MY_DEBUG("m_diagnoseTimer.start");
            m_diagnoseTimer.start(NET_SOCKET_TIMEOUT);
        }
    }
    else
    {
        MY_DEBUG("3 socket connect failed!");
        foreach (QTcpSocket *socket, socketList)
        {
            if(socket !=  NULL)
            {
                socket->abort();
            }
            socket == NULL;
        }

        QTimer::singleShot(100,this,SLOT(connectTcpServer()));
//        m_findUdpTimer.start();
//        QTimer::singleShot(10,this,SLOT(OnDisconnected()));
    }
}

void SocketClient::OnReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());

    if(socket == m_dataSocket )
    {
//        readSocket(socket, &m_dataPackage);
        readSocket(socket, &m_dataPackage); //使用新的方法解析数据 161229
    }
    else if(socket == m_cmdSocket)
    {
        readSocket(socket, &m_cmdPackage);
        MY_DEBUG("");
    }
    else if(socket == m_shakeSocket)
    {
        if(NET_SHAKE_HAND_ENABLED == true)
        {
            m_diagnoseTimer.start(NET_SOCKET_TIMEOUT);
//            MY_DEBUG("m_diagnoseTimer.start(NET_SOCKET_TIMEOUT)");
        }
    }
    else
    {
        MY_LOG("socket connection abnormal");
    }


}

void SocketClient::readSocket(QTcpSocket *socket,
                              SocketPacket *packet)
{

    packet->data += socket->readAll();
    bool isOk = false;
    do{
        isOk = parsePacket(socket, packet);
    }while(isOk);
}

bool SocketClient::parsePacket(QTcpSocket *socket, SocketPacket *packet)
{
    int pIndexStart = packet->data.indexOf(NET_PACKAGE_START);
    if(pIndexStart < 0)
    {
        return false;
    }

    packet->data = packet->data.mid(pIndexStart); //截取从包头index_start到末尾的数据
    SocketPacket tmpPacket;
    tmpPacket.data = packet->data;

    tmpPacket.data.remove(0, QByteArray(NET_PACKAGE_START).size());//删除包头

    //解析包长度
    if(tmpPacket.data.count() < NET_PACKAGE_LTNGTH_BYTES)
    {
        return false;
    }
    bool isOk;
    tmpPacket.length = tmpPacket.data.mid(0, NET_PACKAGE_LTNGTH_BYTES).toLong(&isOk);
    if(isOk == false)
    {
        packet->data.remove(0, QByteArray(NET_PACKAGE_START).size());//删除包头
        if(packet->data.indexOf(NET_PACKAGE_START) >= 0)
        {
            return true;//有可能出现粘包的情况，继续解析后面数据
        }
        else
        {
            return false;
        }
    }

    //数据到达包长度
    tmpPacket.data.remove(0, NET_PACKAGE_LTNGTH_BYTES);//删除数据长度
    if(tmpPacket.length > tmpPacket.data.count())
    {
        return false;
    }

    //包尾是否匹配
    tmpPacket.data.resize(tmpPacket.length);//删除多余数据
    if(tmpPacket.data.endsWith(NET_PACKAGE_END) == false)
    {
        packet->data.remove(0, QByteArray(NET_PACKAGE_START).size());//删除包头
        if(packet->data.indexOf(NET_PACKAGE_START) >= 0)
        {
            return true;//有可能出现粘包的情况，继续解析后面数据
        }
        else
        {
            return false;
        }
    }

    tmpPacket.data.resize(tmpPacket.length -
                          QByteArray(NET_PACKAGE_END).count()); //删除包尾

    //解析出数据类型
    if(tmpPacket.data.count() < NET_DATA_TYPE_BYTES)
    {
        return false;
    }
    QByteArray dataType = tmpPacket.data.left(NET_DATA_TYPE_BYTES);
    tmpPacket.dataType = dataType;

    tmpPacket.data.remove(0,NET_DATA_TYPE_BYTES);//删除数据类型

    //如果是命令类型，解析命令
    if(dataType  ==  NET_DATA_TYPE_CMD)
    {
        tmpPacket.keyValues = tmpPacket.data.split(',');

        //第一个数据是命令
        tmpPacket.cmd = tmpPacket.keyValues.first();

        //其余的是键值对，key1 = value1，key2 = value2，...
        tmpPacket.keyValues.removeFirst();
    }

    //发送数据包消息
    if(socket == m_cmdSocket)
    {
        emit cmdPacket(tmpPacket);
    }
    else if(socket == m_dataSocket)
    {
//        static int times = 0;
//        times++;
//        if(tmpPacket.dataType == NET_DATA_TYPE_TESTER_STATUS &&
//                times > 100)
//        {
            emit dataPacketReady(tmpPacket);
//            times = 0;
//        }
    }

    //删除当前包数据
    packet->data.remove(0,
                    QByteArray(NET_PACKAGE_START).size() +
                    NET_PACKAGE_LTNGTH_BYTES +
                    tmpPacket.length);

    return true;
}


void SocketClient::findUDPServer()
{
//    MY_DEBUG("finding UDP server...");
    if(m_dataSocket !=  NULL)
    {
        return;
    }

    for(int i = 0;i<20;i++)
    {
        m_udpClient->writeDatagram(NET_CMD_IDENTIFICATION
                                   ,QHostAddress::Broadcast,
                                   NET_UDP_PORT+i*NET_PORT_SPACE);
    }
//    QTimer::singleShot(1000, this, SLOT(findUDPServer()));
}

void SocketClient::readDatagram()
{
    QByteArray datagram;
    QHostAddress senderHostAddress;
    QString senderIP;
    quint16 senderPort = -1;
    do{
        datagram.resize(m_udpClient->pendingDatagramSize());
        m_udpClient->readDatagram(datagram.data(),
                                  datagram.size(),
                                  &senderHostAddress,
                                  &senderPort);
//        MY_DEBUG("datagram = "<<datagram);

        //因为搜寻服务器时会接收到自己发送的广播，需要过滤掉
        if(NET_CMD_IDENTIFICATION == datagram ||
           NET_CMD_GET_SERVER_IP == datagram  ||
                datagram.isEmpty())
        {
//            MY_DEBUG("");
            continue;
        }

        if(senderHostAddress.isNull() == false && senderPort != -1)
        {
            //qDebug()<<"sender_ip = "<<sender_ip.toString()<<"sender_port = "<<sender_port;
            QStringList strList = QString(datagram).split(",");
            int port = this->getValue(strList,
                                    NET_KEY_TCPSERVER_PORT).toInt();
            senderHostAddress.setAddress(senderHostAddress.toIPv4Address());
            senderIP = senderHostAddress.toString();
            QString testerName = this->getValue(strList,NET_KEY_TESTER_NAME);
            MY_DEBUG("testerName="<<testerName);


            /* 加上m_findUdpTimer.isActive()，仅凭m_dataSocket == NULL条件，
             * connectTcpServer有可能会被连续调用两次 2016-07-12
             */
            if(m_dataSocket == NULL && m_findUdpTimer.isActive())
            {
                m_findUdpTimer.stop();
//                this->connectTcpServer(sender_ip,port);
                m_tcpServerIP   = senderHostAddress;
                m_tcpServerPort = port;

                senderHostAddress.toIPv4Address();
                MY_DEBUG("m_tcpServerIP = "<<senderHostAddress);
                MY_DEBUG("m_tcpServerPort = "<<port);
                MY_DEBUG("call connectTcpServer()");
                QTimer::singleShot(5,this,SLOT(connectTcpServer()));
                break;
            }
            else
            {
//                QString ip= sender_ip.toString();
//                if(m_serverMap.keys().contains(ip) == false)
//                {
//                    MY_DEBUG("");
//                sender_ip.setAddress(sender_ip.toIPv4Address());
//                m_serverMap.insert(sender_ip.toString(), port);
//                emit serverIpReady(m_serverMap.keys());
//                }
//                    break;

                MY_DEBUG("sender_ip.toString()="<<senderIP);
                QStringList serverNames;
                serverNames.append(testerName);

                bool ipExisted = false;
                for(int i = 0; i < m_serverList.count(); i++)
                {
                    serverNames.append(m_serverList.at(i).name);

                    if(m_serverList.at(i).ip == senderIP)
                    {
                        ipExisted = true;
                    }
                }

                if(!ipExisted && !senderIP.isEmpty())
                {
                    MY_DEBUG("");

                    ServerInfo info;
                    info.ip = senderIP;
                    info.name = testerName;
                    info.port = port;
//                    MY_LOG(QString("testerName = %1,senderIP = %2, port = %3")
//                           .arg(testerName)
//                           .arg(senderIP)
//                           .arg(port));

                    m_serverList.insert(0,info);

                    emit serverIpReady(serverNames);
                }

            }
        }


    }while(m_udpClient->hasPendingDatagrams());

}

void SocketClient::TCP_send(QByteArray data,char cmd)
{
    QTcpSocket *socket = m_dataSocket;

    data.prepend(cmd);

    data.replace(0xAA, QByteArray::fromHex("BB0A"));
    data.replace(0xBB, QByteArray::fromHex("BB0B"));


    data.prepend(0xAA);
    data.append(0xAA);

    socket->write(data);
}

void SocketClient::OnDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    MY_DEBUG("");
//    if(socket == m_dataSocket)
//    {
//        m_dataSocket->abort();
//        m_dataSocket = NULL;
//    }
//    else if(socket == m_cmdSocket)
//    {
//        m_cmdSocket->abort();
//        m_cmdSocket  = NULL;
//    }
//    else if(socket == m_shakeSocket)
//    {
//        m_shakeSocket->abort();
//        m_shakeSocket = NULL;
//    }

    if(m_dataSocket !=  NULL)
    {
        m_dataSocket->abort();
        m_dataSocket = NULL;
    }
    if(m_cmdSocket !=  NULL)
    {
        m_cmdSocket->abort();
        m_cmdSocket = NULL;
    }
    if(m_shakeSocket !=  NULL)
    {
        m_shakeSocket->abort();
        m_shakeSocket = NULL;
    }

    m_diagnoseTimer.stop();

//    emit hasClient(true);

//    return;
    //三个Socket都断了之后，重新连接
    if(m_shakeSocket == NULL && m_cmdSocket == NULL &&
       m_dataSocket == NULL &&  m_isConnecting == false)
    {
        MY_DEBUG("m_findUdpTimer.start()");
        m_isConnecting = true;
        emit hasClient(false);
//        QTimer::singleShot(500,this,SLOT(connectTcpServer()));
//          QTimer::singleShot(500,this,SLOT(findUDPServer()));

        readDatagram(); //将之前遗留的数据包全部读出来，否则不会产生触发信号 160826

        m_findUdpTimer.start();


//        m_udpClient->readAll();

    }
    MY_DEBUG("");
}

QString SocketClient::getValue(QStringList strList,QString key)
{
    foreach(QString s, strList)
    {
        if(s.startsWith(key))
        {
            return s.mid(key.size());
        }
    }
    return ERROR_INFO;
}

void SocketClient::send(QByteArray data,QByteArray dataType)
{
    QTcpSocket *socket = NULL;
    if(dataType == NET_DATA_TYPE_CMD)
    {
        socket = m_cmdSocket;
    }
    else
    {
        socket = m_dataSocket;
    }

    if(socket == NULL || (socket !=  NULL &&
       socket->state() !=  QTcpSocket::ConnectedState))
    {
//        emit sendingError();
        return;
    }
//    MY_DEBUG("data = "<<data<<",dataType = "<<dataType);

    data = dataType + data + NET_PACKAGE_END;//类型+数据+尾

    float l = data.size();

    //8字节长度，前面补零，如"00065536"
    QByteArray length = QByteArray().setNum(l);
    length = QByteArray(NET_PACKAGE_LTNGTH_BYTES,'0') + length;
    length = length.right(NET_PACKAGE_LTNGTH_BYTES);

    //把 头、数据长度 放到数据包的前面
    data = NET_PACKAGE_START + length + data;

//    qDebug()<<"b bytesToWrite = "<<sendTestInfoSocket->bytesToWrite();
    if(socket->bytesToWrite() > 5 * 1024) // 使用 > 0,有些命令会发送被忽略返回
    {
        MY_DEBUG("socket->bytesToWrite() > 5 * 1024");
        return;
    }
//    qDebug()<<"sendDataOnSocket data_tmp.size() = "<<data_tmp.size();
    int nwrite = socket->write(data);

    if(NET_DATA_TYPE_SET_THREE_POINT == dataType)
    {
        MY_DEBUG("data:"<<data.toHex());
        MY_DEBUG("nwrite = "<<nwrite);
    }
}

void SocketClient::shakeHand()
{
    if(m_shakeSocket !=  NULL &&
       m_shakeSocket->state() == QTcpSocket::ConnectedState)
    {
        m_shakeSocket->write(NET_CMD_SHAKE_HAND);
    }
    QTimer::singleShot(2000,this,SLOT(shakeHand()));
}

void SocketClient::socketTimeOut()
{
    MY_LOG("");
    MY_DEBUG("socketTimeOut !");
    m_diagnoseTimer.stop();
    OnDisconnected();

//    if(tcpSocket!= NULL)
//        tcpSocket->abort();
//    tcpSocket = NULL;
}

bool SocketClient::sendFile(QString fileName)
{   
    if(this->isConnected() == false ||
       m_sendFileTimer.isActive() == true)
    {
        return false;
    }

    m_fileName = fileName;
    QFile file(m_fileName);
    if(file.open(QFile::ReadOnly))
    {
        m_fileData.m_fileSize = file.size();
        m_fileData.m_fileName = QFileInfo(file).fileName();
        m_fileData.m_amountPacket = m_fileData.m_fileSize / PACKET_SIZE;
        long size = m_fileData.m_fileSize;
        if( (size % PACKET_SIZE) > 0)
        {
            m_fileData.m_amountPacket += 1;
        }
        m_fileData.m_packetNum = 0;
    }

    m_sendFileTimer.start(10);
    return true;
}

bool SocketClient::isSendingFile()
{
    return m_sendFileTimer.isActive();
}

void SocketClient::stopSendFile()
{
    m_sendFileTimer.stop();
}

void SocketClient::sendFilePacket()
{
    if(this->isConnected() == false)
    {
        m_sendFileTimer.stop();
        emit sendingFilePercent( -1 );
        return;
    }

    if(m_fileData.m_packetNum != m_fileData.m_amountPacket &&
        this->m_dataSocket->bytesToWrite() > PACKET_SIZE)
    {
//        MY_DEBUG("");
        return;
    }
    else if(m_fileData.m_packetNum == m_fileData.m_amountPacket &&
            m_dataSocket->bytesToWrite() > 0)
    {
        return;
    }
    else if(m_fileData.m_packetNum == m_fileData.m_amountPacket &&
           m_dataSocket->bytesToWrite() == 0)
    {
        m_sendFileTimer.stop();
    }

    QFile file(m_fileName);
    if(file.open(QFile::ReadOnly))
    {
        file.seek(m_fileData.m_packetNum * PACKET_SIZE);
        m_fileData.m_data = file.read(PACKET_SIZE);
        m_fileData.m_packetNum += 1;

        if(m_fileData.m_data.isEmpty())
        {
           m_sendFileTimer.stop();
           return;
        }

        QByteArray data;
        m_fileData.write(&data);
        this->send(data, NET_DATA_TYPE_FILE);

        int percent = 100 * m_fileData.m_packetNum / m_fileData.m_amountPacket;
        emit sendingFilePercent(percent);
    }
}

void SocketClient::findAllTcpServer()
{
    MY_DEBUG("");
//    m_serverMap.clear();
    m_serverList.clear();

    for(int i = 0; i < 1/*5*/; i++)
    {
        for(int i = 0;i<20;i++)
        {
            m_udpClient->writeDatagram(NET_CMD_IDENTIFICATION
                                       ,QHostAddress::Broadcast,
                                       NET_UDP_PORT + i*NET_PORT_SPACE);
        }
        g_msleep(5);
    }

    QTimer::singleShot(500, this, SLOT(readDatagram()));
}

void SocketClient::connectTcpServer(int index)
{
//    QList<QString> ipList = m_serverMap.keys();
//    foreach(QString hostIP, ipList)
//    {
//        if(hostIP == ip)
//        {
//            m_tcpServerIP = QHostAddress(hostIP);
//            m_tcpServerPort = m_serverMap.value(hostIP);
//            OnDisconnected();
//            QTimer::singleShot(5, this, SLOT(connectTcpServer()));
//        }
//    }

    if(index < m_serverList.count())
    {
        m_tcpServerIP = QHostAddress(m_serverList.at(index).ip);
        m_tcpServerPort = m_serverList.at(index).port;
        OnDisconnected();
        QTimer::singleShot(5, this, SLOT(connectTcpServer()));
        MY_DEBUG("m_tcpServerIP="<<m_tcpServerIP<<",port="<<m_tcpServerPort);
    }
}




