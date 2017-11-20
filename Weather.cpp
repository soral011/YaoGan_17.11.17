#include "Weather.h"
#include <QDebug>
#include "Global.h"
#include <QMutexLocker>
#include <math.h>
#include "MyDebug.cpp"

#define CAN_WE_GO {;{\
                        QMutexLocker locker(&m_metex);\
                        if(m_isStopThread == true)\
                        { \
                          break; \
                        }\
                   }}

Weather::Weather()
{
    m_isStopThread = false;
    connect(&g_weatherCom,SIGNAL(readyRead()),
            this,SLOT(readSerialPortData()));

    connect(this,SIGNAL(writeData(QByteArray ))
            ,this,SLOT(sendDataOnSerialPort(QByteArray )));

    connect(this,SIGNAL(initSerialPortSignal())
            ,this,SLOT(initSerialPort()));

//    connect(&g_weatherCom, SIGNAL(error(QSerialPort::SerialPortError)),
//            this, SLOT(weatherComError(QSerialPort::SerialPortError)));

}

Weather::~Weather()
{
    stop();
    MY_DEBUG("");
}

//使用单例模式
Weather *Weather::getInstance()
{
    static Weather instance; //局部静态变量，若定义为指针，则需要手动释放内容
    return &instance;
}

void Weather::stop()
{
    QMutexLocker locke(&m_metex);
    this->m_isStopThread = true;
}

bool Weather::myMSleep(int msecs)
{
    while(msecs)
    {
        msecs -= 10;
        msleep(10);
        QMutexLocker locke(&m_metex);
        if(m_isStopThread == true)
        {
            break;
        }
    }
    return m_isStopThread;
}

void Weather::run()
{
    while(1)
    {
        {
            QMutexLocker locke(&m_metex);
            if(m_isStopThread  ==  true)
            {
                break;
            }
        }

        int interval  =  1000;
        msleep(interval);
        if(g_weatherCom.openMode() == QIODevice::NotOpen ||
           g_weatherCom.error() != QSerialPort::NoError)
        {
//            MY_DEBUG("g_weatherCom.error()="<<g_weatherCom.error());
            g_weatherCom.close();
            emit weatherStatus(false);
            emit initSerialPortSignal();
            emit updateWeather(WeatherReport());
            myMSleep(2000);
            continue;
        }
        else
        {
            emit weatherStatus(true);
        }

        /*
        QList<QByteArray> byteArrayList;
        byteArrayList<<MODBUS_READ_WIND_SPEED
                     <<MODBUS_READ_WIND_DIRECTION
                     <<MODBUS_READ_TEMPERATURE
                     <<MODBUS_READ_HUMIDITY
                     <<MODBUS_READ_ATMOSPHERE;

        foreach(QByteArray data,byteArrayList)
        {
            emit this->sendData(data);
            if(myMSleep(interval) == true)
            {
                break;
            }
        }
        */
        //只需发送一条命令
        emit sendData(MODBUS_READ_ALL_DATA);
    }
}

bool Weather::initSerialPort()
{
//    g_logHere(LOG_INFO(g_log_var_list,""));
    QString port_name = g_settings.value(HKEY_WEATHER_COM, "关闭").toString();
    if(port_name.size() >=  5)
        port_name = "\\\\.\\"+port_name;
    g_weatherCom.setPortName(port_name);
    QSerialPort::BaudRate baudrate = QSerialPort::Baud9600;

    if(g_weatherCom.openMode() !=  QIODevice::NotOpen) //已经打开串口
    {
        g_weatherCom.setBaudRate(baudrate);//重新设置波特率
        return true;
    }
    else if(g_weatherCom.open(QIODevice::ReadWrite) == true)  //打开串口
    {
        g_weatherCom.setBaudRate(baudrate);
        g_weatherCom.setDataBits(QSerialPort::Data8); //数据位设置，设置为8位数据位
        g_weatherCom.setParity(QSerialPort::NoParity);//奇偶校验设置，设置为无校验
        g_weatherCom.setStopBits(QSerialPort::OneStop);//停止位设置，设置为1位停止位
        g_weatherCom.setFlowControl(QSerialPort::NoFlowControl);//数据流控制设置，设置为无数据流控制
//        m_serialPort.setTimeout(10);
//        qDebug()<<"open com_protocol_thread successful !";
        return true;
    }
    else//打开串口失败
    {
        return false;
    }
}

void Weather::sendData(QByteArray data)
{
    m_historyCommand = data;
    data = QString(data).remove("-").remove(" ").toUtf8();
    data = QByteArray::fromHex(data);
    m_isReceiveFeedback = false;

    uchar *ptr = (uchar *)data.data();
    uint  CRC  = this->getCRC16(ptr,data.count());
    uchar crcL = CRC & 0xff;
    uchar crcH = (CRC & 0xff00)>>8;

    data.append(crcH);
    data.append(crcL);

//    qDebug("crcL = %02X,crcH = %02X\n",crcL,crcH);

    emit writeData(data);//使用UI线程来发送，否则会产生警告信息(带充分验证)

}

void Weather::sendDataOnSerialPort(QByteArray data)
{
    g_weatherCom.write(data);
}

void Weather::weatherComError(QSerialPort::SerialPortError error)
{
    MY_DEBUG("g_weatherCom.errorString()="<<g_weatherCom.errorString().toUtf8());
    MY_DEBUG("g_weatherCom.error="<<error);
}

void Weather::readSerialPortData()
{
//    qDebug()<<"readSerialPortData";
    static QByteArray buffer;
    buffer += g_weatherCom.readAll();
    if(buffer.count()<25)
    {
        return;
    }

    bool isOk = isCRCOk(buffer);
//    qDebug()<<"isCRCOk = "<<isOk;
    if(isOk == false)
    {
        buffer.clear();
        return;
    }

    m_isReceiveFeedback = true;

    uchar    *buf = (uchar*)buffer.data();
    float   tmp;
    int     dislen = 4;//1个寄存器4字节（待验证）

    //风速
    changeUPDown(&buf[3]);
    memcpy(&tmp,&buf[3],4);
    m_weatherData.windSpeed.sprintf("%.2f",tmp);
    //风向
    changeUPDown(&buf[3+dislen]);
    memcpy(&tmp,&buf[3+dislen],4);
    m_weatherData.windDirection.sprintf("%.2f",tmp);

    //温度
    changeUPDown(&buf[3+dislen*2]);
    memcpy(&tmp,&buf[3+dislen*2],4);

    m_weatherData.temperature.sprintf("%.2f",tmp);
    //湿度
    changeUPDown(&buf[3+dislen*3]);
    memcpy(&tmp,&buf[3+dislen*3],4);
    m_weatherData.humidity.sprintf("%.2f",tmp);
    //大气压
    changeUPDown(&buf[3+dislen*4]);
    memcpy(&tmp,&buf[3+dislen*4],4);
    m_weatherData.atmosphere.sprintf("%.2f",tmp);

    emit updateWeather(m_weatherData);//上传气象站数据

    buffer.clear();
}

int Weather::byteArrayToInt(QByteArray *byteArray,int n,uchar &CS)
{
    //LOG_HERE("");
    int H = 0,L = 0;
    if(n == 2)
    {
        H = ((uchar)byteArray->at(0))/*<<8*/;
        L = (uchar)byteArray->at(1);
    }
    else if(n == 1)
        L = (uchar)byteArray->at(0);
    byteArray->remove(0,n);

    CS += H;
    CS += L;

    H = (H<<8);
    return (H|L);
}

bool Weather::isCRCOk(QByteArray byteArray)
{
    int dataCount = (uchar)byteArray.at(2);

    QByteArray temp = byteArray.left(3+dataCount);
    uchar *ptr = (uchar *)temp.data();
    uint caculateCRC = getCRC16(ptr,3+dataCount);

    temp = byteArray.right(2);
    uchar nothing;
    uint originalCRC = byteArrayToInt(&temp,2,nothing);

    if(caculateCRC == originalCRC)
    {
        return true;
    }
    else
    {
        return false;
    }
}

uint Weather::getCRC16(uchar *ptr,  uchar len)
{
    uint index;
    uchar crch  =  0xFF;  //高CRC字节
    uchar crcl  =  0xFF;  //低CRC字节
    uchar TabH[]  =  {  //CRC高位字节值表
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
        0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
        0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
        0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
        0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
        0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
        0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
        0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
        0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
        0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
    };
    uchar TabL[]  =  {  //CRC低位字节值表
        0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
        0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
        0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
        0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
        0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
        0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
        0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
        0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
        0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
        0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
        0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
        0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
        0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
        0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
        0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
        0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
        0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
        0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
        0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
        0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
        0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
        0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
        0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
        0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
        0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
        0x43, 0x83, 0x41, 0x81, 0x80, 0x40
    };

    while(len--)  //计算指定长度的CRC
    {
        uchar value = (uchar)*ptr;
//        qDebug()<<"value = "<<value;
        ptr++;
        //index  =  crch ^ *ptr++;
        index  =  crch ^ value;
        crch  =  crcl ^ TabH[index];
        crcl  =  TabL[index];
    }

    return ((crch<<8) |  crcl);
}

void Weather::changeUPDown(uchar *buf)
{
    uchar t1,t2;
    t1=buf[0];
    buf[0]=buf[1];
    buf[1]=t1;
    t2=buf[2];
    buf[2]=buf[3];
    buf[3]=t2;
}
