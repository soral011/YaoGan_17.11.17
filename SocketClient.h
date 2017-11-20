/***********************************************************
**
** 作者：lth
** 日期：2016-03-09
** 描述：1.使用UDP搜索TCP服务器并连接；
**      2.收发Socket数据包。
**
************************************************************/

#ifndef SOCKETCOMMUNICATION_H
#define SOCKETCOMMUNICATION_H
#include <QObject>
#include <QUdpSocket>
#include <QTcpSocket>
#include "SocketConstant.h"
#include <QTimer>
#include "Global.h"
#include <QMap>

struct ServerInfo
{
    QString ip;
    QString name;
    int port;
};

class SocketClient: public QObject
{
    Q_OBJECT

protected://构造函数是保护型的，此类使用单例模式
    SocketClient();

//    QTimer        m_replyTimer;  //接收对方Socket反馈
//    volatile bool m_isReplyTrue; //若收到对方Socket反馈则置为true

private:
    QUdpSocket  *m_udpClient;

    /* 为什么会有m_dataSocket和m_cmdSocket呢，当初的考虑是
     * 当正在发送一个大的数据块（包）时，一般需要耗时较长，只有
     * 当对方接收完这个数据块（包）时，才会继续接收下一个数据块
     * （包），因此若要终止当前传输数据，必须先完成当前收发数据
     * 块（包），所以需要一个专门发送命令的m_dataSocket，用于
     * 终止m_dataSocket当前收发数据块。
     * （但是可以通过发送特定的字符串终止当前收发数据块，若对方
     * 收到这个特定长度的字符串，则终止接收！！！
     * 待日后优化，20160614）
     */
    QTcpSocket  *m_dataSocket;  //收发数据
    QTcpSocket  *m_cmdSocket;   //处理远程控制指令
    QTcpSocket  *m_shakeSocket; //保持握手

    QTimer       m_diagnoseTimer; //接收到握手信号，过一段时间再判断是否还接收到
    QTimer       m_findUdpTimer;
    QTimer       m_sendFileTimer;

    QString      m_fileName;//发送文件名称
    NetFileData  m_fileData;//发送文件数据包

    SocketPacket m_dataPackage;
    SocketPacket m_cmdPackage;
    QHostAddress m_tcpServerIP;
    int          m_tcpServerPort;
    volatile bool m_isConnecting;

//    QMap<QString, int> m_serverMap;
    QList<ServerInfo>  m_serverList;

public:

    //使用单例模式
    static SocketClient *getInstance();

    bool isConnected();
    bool sendFile(QString fileName);
    bool isSendingFile();
    void stopSendFile();
    void findAllTcpServer();
    void connectTcpServer(int index);
    void TCP_send(QByteArray data,char cmd);

private:
    void bind(QUdpSocket *udpSocket);
    void readSocket(QTcpSocket *socket,
                    SocketPacket *socketPacket);
    bool parsePacket(QTcpSocket *socket, SocketPacket *packet);
    QString getValue(QStringList strList, QString key);


signals:
    void dataPacketReady(SocketPacket socketData);
    void cmdPacket(SocketPacket socketData);
    void hasClient(bool);
//    void sendingError(); //可以通过调用实例来判断是否连接
    void sendingFilePercent(int percent);
    void serverIpReady(QList<QString> ipList);

public slots:
    void send(QByteArray data, QByteArray dataType);
    void OnReadyRead();
    void readDatagram();
    void OnDisconnected();
    void connectTcpServer();

    void findUDPServer();
    void isAllConnected();
    void shakeHand();
    void socketTimeOut();
    void sendFilePacket();
};

#endif // SOCKETCOMMUNICATION_H
