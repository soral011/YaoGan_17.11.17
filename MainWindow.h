/***********************************************************
**
** class: MainWindow
**
** 作者：lth
** 日期：2017-07-25
**
** 测量结果显示主界面
**
************************************************************/

#ifndef TESTWINDOW_H
#define TESTWINDOW_H

#include <QWidget>
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QMenu>
#include <QAction>
#include <QSignalMapper>
#include <QThread>
#include <QMutexLocker>
#include "tools/MyMenu.h"
#include "Settings.h"
#include "tools/ProgressDialog.h"
#include "Camera.h"
#include "PlateRecogRegion.h"

#include "Weather.h"
#include "results/Results.h"
#include "RunningStatus.h"
#include "led_control.h"

#include "Constant.h"
#include "About.h"
#include "AccessDatabase.h"
#include "Global.h"
#include "ControlPanel.h"
#include "Spectrum.h"
#include "tools/MyMessageDialog.h"
#include "RealTimeTest.h"
#include "operation_flow/OperationFlow.h"
#include "operation_flow/AdjustCamera.h"
#include "operation_flow/CaliDialog.h"
#include "SystemStatus.h"
#include "SocketClient.h"
#include "UpdateSoftwareARM.h"
#include "user_management/UserManagement.h"
#include "site_management/SiteManagement.h"

//菜单选项
#define MENU_ITEM_DISCONNECT             "断开"
#define MENU_ITEM_CONNECT                "连接"
#define MENU_ITEM_QUIT                   "退出"
#define MENU_ITEM_BASIC                  "常规"
#define MENU_ITEM_PREFERENCE             "偏好"
#define MENU_ITEM_OTHER                  "超标门限"
#define MENU_ITEM_VIEW_RESULTS           "历史记录"
#define MENU_ITEM_MANUAL_SNAP            "手动抓拍"
#define MENU_ITEM_MANUAL_CAPTURE_FULL    "截取软件界面"
#define MENU_ITEM_MANUAL_CAPTURE_VEDIO   "截取图像浏览窗口"
#define MENU_ITEM_SPECTRUM               "光谱光强"
#define MENU_ITEM_CONTROL_PANEL          "控制面板"
#define MENU_ITEM_REAL_TIME_TEST         "实时测量"
#define MENU_ITEM_ABOUT                  "关于"
#define MENU_ITEM_UPDATE_ARM_SOFTWARE    "更新测量主机软件"
#define MENU_ITEM_INSTRUCTION_MANUAL     "使用帮助"
#define MENU_ITEM_SYSTEM_STATUS          "系统状态"
#define MENU_ITEM_USER_MANAGE            "用户管理"
#define MENU_ITEM_TEST_SITE              "监测地点"

#define CAPTURE_PATH  "Capture/"

#define IS_SHOW_WITH_ANIMATION  false

namespace Ui {
class TestWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    Camera       *m_camera;
    Spectrum      m_spectrum;
    ControlPanel  m_controlPanel;
    RealTimeTest  m_realTimeTest;


private:
    Ui::TestWindow *ui;

    MyMenu *m_toolsMenu;
    MyMenu *m_manageMenu;
    MyMenu *m_helpMenu;

    OperationFlow    *m_operationFlow;
    AdjustCamera     *m_adjustCamera;
    CaliDialog        m_caliDialog;
    ConnectionStatus  m_connectionStatus;
    SystemStatus     *m_systemStatus;
    led_control      *m_led;


    QSignalMapper m_signalMapper;

    QString  m_selectedStyle;
    QString  m_notSelectedStyle;

    Settings m_settingsDialog;
    ProgressDialog m_progressDialog;

    QList<PropertyAndText>    m_propertyList;

    Results          *m_testResults;
//    RunningStatus     m_runningStatus;
    CaptureResults    m_captureResults;

    CarTestResults    m_carTestResults;

    bool m_cameraConnected;
    bool m_testerConnected;

    int  m_testerWorkMode;

    void initMenu();
    void hideButtonMenu();

    void switchCarPic(QPixmap carPixmap);
    void updateTestResultText();
    void addOneRowTestResult();
    int  getColumnPosition(QString property);



signals:


private slots:
    void buttonReleased(QWidget *w);
    void menuItemClicked(QString item);
    void closeEvent(QCloseEvent * event);
    void closeWithAnimation();
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent *event);
    void showEvent(QShowEvent * event);

    void receiveCaptureInfo(CaptureResults captureResults);
    void receiveTestResult(CarTestResults oneCarResults);

    void on_tableWidget_test_result_list_itemClicked(QTableWidgetItem *item);
    void loginFinished(bool value);
    void updateWeather(WeatherReport data);
    void showTipsWidget();
    void hasClient(bool value);

    void dataPacketReady(SocketPacket packet);

    void on_checkBox_is_save_test_result_clicked(bool checked);
    void operationFlowFinished(bool value);
    void deviceConnection(bool isConnected);
    void showAdjustCamera();
    void updateMainWindow();
    void updateVerticalHeaderHeight(QVariant var);
};

#endif // TESTWINDOW_H
