/***********************************************************
**
** 作者：lth
** 日期：2017-07-25
**
** 声明全局变量
**
************************************************************/

#ifndef COMMONFUNCTIONS_H
#define COMMONFUNCTIONS_H
#include <QWidget>
#include <QSignalMapper>
#include <qdebug.h>
#include <QSettings>
#include <QLabel>
#include <QPainter>
#include <QPropertyAnimation>
#include <QSerialPort>
#include <QTableWidget>
#include <QSqlDatabase>
#include <QMessageBox>
#include <QSqlQuery>
#include <QPainter>
#include <QPrinter>
#include <QFileDialog>
#include <QParallelAnimationGroup>
#include "NetDataStructure.h"
#include "Constant.h"
#include "tools/MyMessageDialog.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <QVector>
#include "Windows.h"
#include "HCNetSDK.h"
#include "MyDebug.cpp"
#include "user_management/AddUserDialog.h"
#include "site_management/SiteManagement.h"
#include "Weather.h"

//测量主机工作模式
enum TesterWorkMode
{
        E_WorkMode_Testing,     //测量模式，正常测量模式
        E_WorkMode_RealTimeTesting,     //实时测量模式，用于计量检定，计算仪器误差、重复性、稳定性
        E_WorkMode_Calibrating, //标定模式
        E_WorkMode_Preheating,  //预热模式
        E_WorkMode_Standby,     //待机模式
};

class PropertyAndText
{
public:
    QString m_property;
    QString m_text;

    PropertyAndText(QString text,QString property)
    {
        m_property = property;
        m_text     = text;
    }
};

class PropertyAndVariant
{
public:
    QString  m_property;
    QVariant m_variant;

    PropertyAndVariant(QString property,QVariant variant)
    {
        m_property = property;
        m_variant  = variant;
    }
};

class MySignal :public QObject
{
    Q_OBJECT
public:
    ~MySignal()
    {
        qDebug()<<"MyQSignalMapper destroyed !";
    }

signals:
    void showTips(QString tips);
};

//抓拍结果数据结构
class CaptureResults
{
public:
    QPixmap vehiclePic;   //机动车场景图
    QPixmap licensePic;   //车牌小图片
    QString license;      //车牌号
    QString licenseColor; //车牌背景颜色
    int     believability;//车牌置信度
    qint64  receivedTime; //接收到的时间
    NET_VCA_RECT struPlateRect;//车牌位置(在场景图中的位置)

};

//单条测量结果
class CarTestResults
{
public:
    CaptureResults    captureResults; //车牌信息
    QVector <CaptureResults>    captureResults_vector;//车牌存储队列
    NetExhaustResults exhaustResults; //尾气、测速等检测结果
    WeatherReport     weatherReport;  //气象信息

    float vsp;
    bool  isPass;      //是否合格
    bool  isValid;     //测量数据是否有效
    bool  isDiesel;    //是否为柴油车
    QString statusText; //显示有超标、合格、无效
    QString passTime;   //通过时间
};


//P1代表合格框左上角，P2代表合格框右上角，P3代表合格框右下角
//m_P_Top代表在合格框上边框滑动的点
extern QPoint g_PlateRecogRegion_P1,
              g_PlateRecogRegion_P2,
              g_PlateRecogRegion_P3,
              g_PlateRecogRegion_P4;
extern QPoint g_PlateRecogRegion_Top,
              g_PlateRecogRegion_Right;

extern MySignal    g_mySignal;
extern QSettings   g_settings; //软件中所有的设置信息均通过该对象保存/读取
extern QSerialPort g_weatherCom;
extern QList<PropertyAndText> g_propertyList;

extern bool g_isSavingTestResult; //用于临时测试，是否保存测量结果 161013
extern bool g_isDebugMode; // 是否处于调试模式
extern bool g_ignoringCamera; //是否忽略抓拍机模块，用于调试环境没有抓拍机时

extern UserStruct g_currentUser;
extern SiteStruct g_currentSite;

extern int g_car_result_index;//车牌队列序号


/*************************************************************************
                                以下是函数
*************************************************************************/
extern void g_connectButtonSignal(QWidget *w);
extern void g_connectCheckBoxSignal(QWidget *w);
extern void g_msleep(int msec);
extern void g_createPath(QString path);
extern void g_showLabelTextWithAnim(QLabel *label,QString text,bool isUpToDown=true);
extern void g_print(QPrinter *printer,QPixmap pixmap);
extern void g_printMultipage(QPrinter *printer,QList<QPixmap> pixmapList);
extern void g_qAppInit();
extern void g_log(QString msg);
extern void g_drawShadowBorder(QWidget *w);

extern bool g_exportExcel(QString fileName,QTableWidget *tableWidget);
extern bool g_showAskingDialog(QString msg,QWidget *w);
extern bool g_isClickTimesOK(int times);

extern QString g_getSaveFileName(QWidget *parent
                                 ,QString HKEY
                                 ,QString title
                                 ,QString format
                                 ,QString filter);
extern QString g_codeMd5(QString data);
extern QString g_decodeMd5(QString data);
extern QString g_getNowTime(QString format = "yyyy-MM-dd hh:mm:ss.zzz");

extern QStringList g_getAvailableComList();

extern QPropertyAnimation *g_switchPage(QPixmap oldPixmap
                                        , QPixmap newPixmap
                                        , QWidget *parent
                                        , bool isGoLeft
                                        , int duration = 880);
extern QParallelAnimationGroup *g_showWithAnim(QWidget *widget);
extern QParallelAnimationGroup *g_closeWithAnim(QWidget *widget);


extern double g_setPrecision(double d,int precision);

extern int g_getRandomNum(int max);

extern MyMessageDialog *g_showTipsDialog(QString msg,
                                         QWidget *parent = 0,
                                         QMessageBox::Icon = QMessageBox::Information);

extern bool g_scanPort(char *Ip, int port);

extern void g_clearPushButtonFocus(QWidget *widget);
extern void g_updateTesterInfo(QLabel *label);

#endif // COMMONFUNCTIONS_H
