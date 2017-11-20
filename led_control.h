#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <QObject>
#include <QTcpSocket>

#define PACKGE_HEAD      2
#define PACKGE_UP_LIST   "010"

#define RED         "\\c255000000000"
#define GREEN       "\\c000255000000"
#define YELLOW      "\\c255255000000"

struct show_result{
    QString  car_license;               //车牌号
    double    co,co2,no,hc,yandu,heidu; //尾气测量结果
    QString      isok;                      //是否合格
};

class led_control:public QObject
{
    Q_OBJECT

public:
    void setParm(QString ip,int port);//设置网络连接参数
    bool connect_servers();//连接tcp服务器
    void sendmsg();//发送需显示的内容
    void set_playlst(struct show_result result);//组织play_list内容

    led_control();

private slots:
    void connect_success();
    void connect_fail();
    void readyRead();

private:
    QTcpSocket tcpSocket;
    QString ip;
    int port;
    bool wait_for_connect;//回应等待标志符
    bool is_connected;//连接是否成功标志符
    QString m_filename;

    unsigned short getCRC16(uchar *ptr,  uchar len);
};

#endif // LED_CONTROL_H
