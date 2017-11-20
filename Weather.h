/***********************************************************
**
** class:Weather
**
** 作者：lth
** 日期：2017-07-25
**
** 气象站通讯
**
************************************************************/

#ifndef THREADGETENVIROMENTALPARAMETER_H
#define THREADGETENVIROMENTALPARAMETER_H

#include <QThread>
#include <QSerialPort>
#include <stdlib.h>
#include <QMutex>
#include "Constant.h"

#define MODBUS_READ_WIND_SPEED          "A2 04 00 01 00 01"
#define MODBUS_READ_WIND_DIRECTION      "A2 04 00 02 00 01"

#define MODBUS_READ_TEMPERATURE         "1A 04 00 01 00 01"
#define MODBUS_READ_HUMIDITY            "1A 04 00 02 00 01"
#define MODBUS_READ_ATMOSPHERE          "1A 04 00 03 00 01"

#define MODBUS_READ_ALL_DATA            "01 03 00 00 00 0A"   //智翔宇气象站报文

class WeatherReport
{
public:
    QString windSpeed;
    QString windDirection;
    QString temperature;
    QString humidity;   //湿度
    QString atmosphere; //大气压

    WeatherReport()
    {
        QString org = "--"/*QString().setNum(ERROR_NUMBER)*/;
        windSpeed     = org;
        windDirection = org;
        temperature   = org;
        humidity      = org;
        atmosphere    = org;
    }
};

class Weather : public QThread
{
    Q_OBJECT

protected:
    Weather();
    ~Weather();

public:

    QMutex m_metex;
    volatile bool m_isStopThread;

public:
    static Weather *getInstance();
    void stop();

private:
//    QSerialPort g_environmentParameterCom;
    volatile bool m_isReceiveFeedback;
    QByteArray m_historyCommand;

    WeatherReport m_weatherData;


    bool isCRCOk(QByteArray byteArray);
    uint getCRC16(uchar *ptr,  uchar len);
    void changeUPDown(uchar *in);//高低位转换

protected:
    void run();

signals:
    void writeData(QByteArray data);//写数据到串口对象
    void initSerialPortSignal();
    void updateWeather(WeatherReport data);
    void weatherStatus(bool isConnected);

private slots:
    void readSerialPortData();
    void sendData(QByteArray data);
    int  byteArrayToInt(QByteArray *byte_array,int n,uchar &CS);
    void sendDataOnSerialPort(QByteArray data);
    bool initSerialPort();
    bool myMSleep(int msecs);
    void weatherComError(QSerialPort::SerialPortError error);
};

#endif // THREADGETENVIROMENTALPARAMETER_H
