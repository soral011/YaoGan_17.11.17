/***********************************************************
**
** 作者：lth
** 日期：2016-03-09
** 描述：1.定义网络通讯中用到的常量；
**      2.规定主从机。
**
************************************************************/

#ifndef SOCKETCONSTANT
#define SOCKETCONSTANT
#include <QByteArray>
#include <QList>
#include "Constant.h"

/* 约定：
 * 1.测量主机作为TCPServer、UDPServer；
 * 2.测量主机UDPServer返回信息格式：key1=value1，key2=value2，...
 * 3.包数据结构: [ 头 + 长度(8字节) + 类型(2字节) + 数据 + 尾 ]
 *     a.头、尾是固定字符串，数据的长度可变;
 *     b.长度= 2(类型) + sizeof(数据) + sizeof(尾);
 * 4.以太网最大传输单元MTU:1500字节,因此每次写1024个字节数据
 * 5.当数据类型为命令时，数据格式为：[ CMD，key1=value1，key2=value2，... ]
 *
 */

//Socket连接参数
#define NET_CMD_IDENTIFICATION   "I_AM_YAOGAN_TESTER" //身份识别,手机客户端请求连接
#define NET_CMD_SHAKE_HAND       "SHAKE_HAND"         //手机客户端发出握手信号,灯光仪有反馈说明处于连接
#define NET_CMD_GET_SERVER_IP    "GET_SERVER_IP"      //获取服务端IP地址
#define NET_UDP_PORT             5666    //UDP监听“起始”端口号
#define NET_TCP_PORT             5888    //TCP服务器监听“起始”端口号
#define NET_PORT_SPACE           10      //尝试监听、绑定、广播端口号间隔

#define NET_SOCKET_TIMEOUT       10*1000 //多长时间没有收到客户端信息说明已经断开
#define NET_SHAKE_HAND_ENABLED   true

//控制命令
#define NET_CMD_CALIBRATE_AUTOMATICALLY  "CALIBRATE_AUTOMATICALLY"
#define NET_CMD_CALIBRATE_MANUALLY       "CALIBRATE_MANUALLY"
#define NET_CMD_STOP_CALIBRATING         "STOP_CALIBRATING"
#define NET_CMD_GET_ALL_CALI_DATA        "GET_ALL_CALI_DATA"    //获取所有标定数据
#define NET_CMD_GET_INTEGRATION_TIME     "GET_INTEGRATION_TIME" //获取积分时间等
#define NET_CMD_REBOOT_TESTER            "REBOOT_TESTER"        //重启测量主机
#define NET_CMD_SET_RT_TEST_MODE         "SET_RT_TEST_MODE"     //设置测量主机为实时测量模式
#define NET_CMD_QUIT_RT_TEST_MODE        "QUIT_RT_TEST_MODE"    //测量主机退出实时测量模式,RT--real time
#define NET_CMD_SAVE_BG_SPECTRUM         "SAVE_BG_SPECTRUM"     //实时测量时保存背景光谱
#define NET_CMD_READ_CALI_DATA           "READ_CALI_DATA"       //读取配置参数
#define NET_CMD_EXPORT_FACTORY_CALI_DATA "EXPORT_FACTORY_CALI_DATA" //导出出厂标定数据

//数据包数据结构
#define NET_PACKAGE_START                "PACKAGE_START"
#define NET_PACKAGE_END                  "PACKAGE_END"
#define NET_PACKAGE_TYPE_BYTES           2    //包类型占几个字节
#define NET_PACKAGE_LTNGTH_BYTES         8    //包数据长度占几个字节

//数据类型固定长度：2字节
#define NET_DATA_TYPE_BYTES              2    //数据类型固定长度占几个字节

//数据类型
#define NET_DATA_TYPE_CMD                "11" //控制命令（ + 参数 ）
#define NET_DATA_TYPE_TEST_RESULT        "12" //测量结果
#define NET_DATA_TYPE_FILE               "13" //文件
#define NET_DATA_TYPE_TESTER_STATUS      "14" //测量主机系统状态
#define NET_DATA_TYPE_TRIGGER_SIGNAL     "15" //测量主机触发信号
#define NET_DATA_TYPE_INTEGRATION_TIME   "16" //设置积分时间等
#define NET_DATA_TYPE_IO_CONTROL         "17" //控制气泵阀门等
#define NET_DATA_TYPE_COMMUNICATION_CMD  "18" //通讯命令，如TDLAS命令
#define NET_DATA_TYPE_CALI_GAS_C         "19" //标定气体浓度
#define NET_DATA_TYPE_CALI_STATUS        "1A" //标定状态
#define NET_DATA_TYPE_ALL_CALI_DATA      "1B" //获取所有标定数据
#define NET_DATA_TYPE_REMOTE_ASSISTANCE  "1C" //开启、关闭远程协助工具
#define NET_DATA_TYPE_RT_TEST_RESULT     "1D" //实时测量结果
#define NET_DATA_TYPE_UPDATE_SOFTWARE    "1E" //更新测量主机软件
#define NET_DATA_TYPE_CAR_LEFT           "1F" //车尾离开
#define NET_DATA_TYPE_MULTI_CALI         "21" //多点标定
#define NET_DATA_TYPE_SYNC_TIME          "22" //PC与测量主机时间同步
#define NET_DATA_TYPE_TDLAS_SETTINGS     "23" //TDLAS系统参数设置
#define NET_DATA_TYPE_FACTORY_CALI_DATA  "24" //遥感仪器出厂标定数据
#define NET_DATA_TYPE_USE_SIMU_SPECTRUM  "25" //使用模拟光谱数据
#define NET_DATA_TYPE_SET_TESTER_NAME    "26" //配置测量主机名称（ 出厂编号 ）
#define NET_DATA_TYPE_SET_THREE_POINT    "27" //三点标定

//参数名称
#define NET_KEY_TCPSERVER_PORT      "TCPSERVER_PORT" //用于UDP请求连接时返回TCPServer监听端口号
#define NET_KEY_UDPSERVER_PORT      "UDPSERVER_PORT" //用于UDP请求连接时返回TCPServer监听端口号
#define NET_KEY_TESTER_NAME         "TESTER_NAME"    //用于UDP请求连接时返回信息

#define	KEY_TCPSERVER_PORT(var)	(QString("%1=%2,").arg(NET_KEY_TCPSERVER_PORT).arg(var))
#define	KEY_TESTER_NAME(var)	(QString("%1=%2,").arg(NET_KEY_TESTER_NAME).arg(var))

//其他一些常量
#define WAIT_CMD_REPLY_SECS   5  //等待对方命令Socket反馈，单位秒

#define TEST_HOST_SOFTWARE_NAME     "YaoGanARM"      //测量主机端软件名称

//用于存储接收到的数据包
class SocketPacket
{
public:
    long length;         //数据长度
    QByteArray data;     //数据
    QByteArray dataType; //数据类型
    QString    fileName;

    QByteArray cmd;      //命令
    QList<QByteArray> keyValues;//命令参数，格式：key1=value1，key2=value2，...

    SocketPacket()
    {
        length = INIT_NUMBER;
    }

    void clear()
    {
        length = INIT_NUMBER;
        data.clear();
        dataType.clear();
        cmd.clear();
        keyValues.clear();
    }
};


#endif // SOCKETCONSTANT

