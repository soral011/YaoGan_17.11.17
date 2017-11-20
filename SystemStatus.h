/***********************************************************
**
** class: SystemStatus
**
** 作者：lth
** 日期：2017-07-25
**
** 实时存储遥感监测系统最新的状态信息
**
************************************************************/

#ifndef SYSTEMSTATUS_H
#define SYSTEMSTATUS_H

#include <QDialog>
#include <QMouseEvent>
#include "SocketClient.h"

namespace Ui {
class SystemStatus;
}

class SystemStatus : public QDialog
{
    Q_OBJECT

protected://构造函数是保护型的，此类使用单例模式
    explicit SystemStatus(QWidget *parent = 0);
    ~SystemStatus();

public:
    static SystemStatus *getInstance();
    NetTesterStatus m_testerStatus;
    bool m_isCameraConnected; //测量主机连接状态
    bool m_isTestHostConnected;
    bool m_isWeatherConnected;

private:
    Ui::SystemStatus *ui;
    QPoint m_relativePos;


    QString getConnectionText(bool connected);

private slots:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void on_pushButton_close_clicked();
    void cameraStatus(bool isConnected);
    void testHostStatus(bool isConnected);
    void weatherStatus(bool isConnected);
    void dataPacketReady(SocketPacket packet);
};

#endif // SYSTEMSTATUS_H
