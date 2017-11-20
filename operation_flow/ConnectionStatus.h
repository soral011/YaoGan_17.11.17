/***********************************************************
**
** class: ConnectionStatus
**
** 作者：lth
** 日期：2017-07-25
**
** 测量主机、抓拍机连接状态显示界面
**
************************************************************/

#ifndef CONNECTIONSTATUS_H
#define CONNECTIONSTATUS_H

#include <QWidget>
#include "Camera.h"
#include "SocketClient.h"

namespace Ui {
class ConnectionStatus;
}

class ConnectionStatus : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionStatus(QDialog *parent = 0);
    ~ConnectionStatus();

private:
    Ui::ConnectionStatus *ui;
    bool m_isCameraConnected; //测量主机连接状态
    bool m_isTestHostConnected;

signals:
    void deviceConnection(bool value);

private slots:
    void cameraStatus(bool connected);
    void testHostStatus(bool connected);
};

#endif // CONNECTIONSTATUS_H
