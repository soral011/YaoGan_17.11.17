/***********************************************************
**
** class: Camera
**
** 作者：lth
** 日期：2017-07-25
**
** 抓拍机搜索、连接、云台调节、视频流截图
**
************************************************************/

#ifndef HAIKANGSDK_H
#define HAIKANGSDK_H

#include <QWidget>
#include <QMouseEvent>
#include <QPoint>
#include <QDebug>

#include <QTimer>

#include <stdio.h>
#include <iostream>
#include "Windows.h"
#include "HCNetSDK.h"
#include <time.h>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include "Global.h"
#include <QProcess>


//接收到抓拍数据后，发送抓拍结果数据结构
class SendCaptureInfo :public QObject
{
    Q_OBJECT
signals:
    void sendCaptureInfo(CaptureResults captureResults);
};


extern SendCaptureInfo g_SendCaptureInfo;


class Camera : public QThread
{
    Q_OBJECT

public:
    HWND m_hWnd;
    volatile bool m_connected; //连接状态

protected://构造函数是保护型的，此类使用单例模式
    explicit Camera();
    ~Camera();

private:
    volatile bool m_stopped;
    volatile bool m_isReconnect; //是否重新连接
    QMutex m_runningMutex;
    QMutex m_reconnectMutex;
    LONG   m_lRealPlayHandle;
    LONG   m_lAlarmHandle;
    LONG   m_lUserID;
    LONG   m_lListenHandle;
    QString m_cameraIp;
    QProcess *m_process;


public:
    QPixmap capturePic();
    static Camera *getInstance();  
    void reconnect(QString cameraIp);
    void reStartPreview(HWND hWnd);
    QStringList getAllOnlineCamera();
    QStringList getAllOnlineIP();
    QStringList getSameSectionIp(QString ip);
//    bool scanPort(char *Ip, int port);

private:
    void run();
    void stop();
    bool startPreview();
    bool startAlarm();
    bool isValidIp(QString ip);
    bool isStopNow();
    bool anyNetworkInterfaceRunning();

signals:
    void loginOk(bool );
    void sendCaptureInfo(CaptureResults captureResults);
    void availableCamera(QStringList cameraIpList);

private slots:
    bool loginCamera();
    void controlYunTai(DWORD dwPTZCommand,DWORD dwStop);
    void clearUP();
    void receiveCaptureInfo(CaptureResults captureResults);
    void connectionChanged(bool isConnected);
    void pingAllIp();
};

#endif // HAIKANGSDK_H
