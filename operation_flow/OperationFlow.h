/***********************************************************
**
** class: OperationFlow
**
** 作者：lth
** 日期：2017-07-25
**
** 操作引导流程控制界面，流程包括设备连接状态、抓拍机调节、监测地点选择、标定
**
************************************************************/

#ifndef OPERATIONFLOW_H
#define OPERATIONFLOW_H

#include <QWidget>
#include "operation_flow/AdjustCamera.h"
//#include "operation_flow/CaliWidget.h"
#include "operation_flow/ConnectionStatus.h"
#include "operation_flow/CaliDialog.h"
#include <QMouseEvent>
#include <QList>
#include "Camera.h"
#include "SocketClient.h"
#include "SystemStatus.h"

#define PAGE_INDEX_STATUE   0
#define PAGE_INDEX_CAMERA   1
#define PAGE_INDEX_SITE     2
#define PAGE_INDEX_CALI     3
#define PAGE_INDEX_FINISH   4

class FlowPage
{
public:
    QWidget *m_widget;
    QString  m_title;
};

namespace Ui {
class OperationFlow;
}

//若使用QDialog作为parent的话，当使用动画关闭窗口时，不理想
//若想将窗口始终显示在最上层，可通过QTimer来实现
class OperationFlow : public QWidget
{
    Q_OBJECT

public:
    explicit OperationFlow(QWidget *parent = 0);
    ~OperationFlow();

private:
    Ui::OperationFlow *ui;

    QList<FlowPage> m_flowPageList;
    int      m_currentPageIndex;

    bool m_isCameraConnected; //测量主机连接状态
    bool m_isTestHostConnected;

    QPoint   m_relativePos;

    CaliDialog       m_caliDialog;
    AdjustCamera    *m_adjustCamera;
    ConnectionStatus m_connectionStatus;
    SystemStatus    *m_systemStatus;

    QTimer  m_showTimer;

signals:
    void operationFinished(bool value);

private:
    void showSite(QList<SiteStruct> siteList);

private slots:
    void buttonReleased(QWidget *w);
    void paintEvent(QPaintEvent *event);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void showEvent(QShowEvent * event);
    void closeEvent(QCloseEvent * event);
    void showPage(int pageIndex);
    void cameraStatus(bool isConnected);
    void testHostStatus(bool isConnected);
    void calibrationFinished(bool succeeded);
    void showOnTop();
    void updatetestHostInfo();
};

#endif // OPERATIONFLOW_H
