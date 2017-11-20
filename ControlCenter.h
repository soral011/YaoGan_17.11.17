/***********************************************************
**
** class: ControlCenter
**
** 作者：lth
** 日期：2016-03-21
**
** 作为整个软件的控制中心，负责收发、存储检测数据
**
************************************************************/

#ifndef CONTROLCENTER_H
#define CONTROLCENTER_H
#include <QObject>
#include "SocketClient.h"
#include "MainWindow.h"
#include "AccessDatabase.h"
#include "Weather.h"
#include "Global.h"
//#include <QtXlsx/QtXlsx>
#include "xlsxdocument.h"
#include "led_control.h"

#include "Login.h"
#include "BroadcastCaliStatus.h"

//保存测量结果对应的光谱光强
class SaveTestSpectrum:public QThread
{
public:
    NetTestSpectrum m_testSpectrum;
    QString m_license;
    QString m_passTime;
    QXlsx::Document *m_xlsxSpectrum;
    QXlsx::Document *m_xlsxSpectrumList;

    SaveTestSpectrum()
    {
        QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd");
        QString xlsxName = RESULT_SAVE_PATH + QString("Spectrum_%1.xlsx").arg(date);
        m_xlsxSpectrum = new QXlsx::Document(xlsxName);

        xlsxName = RESULT_SAVE_PATH + QString("SpectrumList_%1.xlsx").arg(date);
        m_xlsxSpectrumList = new QXlsx::Document(xlsxName);
    }

    ~SaveTestSpectrum()
    {
        MY_DEBUG("");
        delete m_xlsxSpectrum;
        delete m_xlsxSpectrumList;
    }

private:
    void run()
    {
        saveSpectrum();
        saveSpectrumList();
    }

    void saveSpectrum()
    {
        int rowCount = m_xlsxSpectrum->read("W1").toInt(); //记录总数存储在“W1”
        if(rowCount == 0)
        {
           //车牌号	DOAS	DOAS_Bg	TDLAS	TDLAS_Bg	Opacity	Opacity_Bg
           m_xlsxSpectrum->write("A1", "时间");
           m_xlsxSpectrum->write("B1", "车牌号");
           m_xlsxSpectrum->write("C1", "DOAS");
           m_xlsxSpectrum->write("D1", "DOAS_Bg");
           m_xlsxSpectrum->write("E1", "TDLAS");
           m_xlsxSpectrum->write("F1", "TDLAS_Bg");
           m_xlsxSpectrum->write("G1", "Opacity");
           m_xlsxSpectrum->write("H1", "Opacity_Bg");

           rowCount = 1;
        }

        rowCount += 1;
        int row = rowCount ; //使用列名称A、B、C...时，row是从1开始的

//        QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        m_xlsxSpectrum->write(QString("A%1").arg(row),m_passTime);

        if(m_license.isEmpty())
        {
            m_license = "---";
        }
        m_xlsxSpectrum->write(QString("B%1").arg(row),m_license);


        QString str;
        foreach(float f, m_testSpectrum.m_doasSpectrum)
        {
            str.append(QString().setNum(f) + " ");
        }
        m_xlsxSpectrum->write(QString("C%1").arg(row), str);


        str.clear();
        foreach(float f, m_testSpectrum.m_doasSpectrumBg)
        {
            str.append(QString().setNum(f) + " ");
        }
        m_xlsxSpectrum->write(QString("D%1").arg(row), str);

        str.clear();
        foreach(int i, m_testSpectrum.m_tdlasSpectrum)
        {
            str.append(QString().setNum(i) + " ");
        }
        m_xlsxSpectrum->write(QString("E%1").arg(row), str);

        str.clear();
        foreach(int i, m_testSpectrum.m_tdlasSpectrumBg)
        {
            str.append(QString().setNum(i) + " ");
        }
        m_xlsxSpectrum->write(QString("F%1").arg(row), str);

        str.clear();
        foreach(int i, m_testSpectrum.m_opacityADResults)
        {
            str.append(QString().setNum(i) + " ");
        }
        m_xlsxSpectrum->write(QString("G%1").arg(row), str);

        str.clear();
        foreach(int i, m_testSpectrum.m_opacityADResultsBg)
        {
            str.append(QString().setNum(i) + " ");
        }
        m_xlsxSpectrum->write(QString("H%1").arg(row), str);


        m_xlsxSpectrum->write(QString("W1"), row);

        bool isOk = m_xlsxSpectrum->save();
        if(isOk == false)
        {
            QString text("保存测量光谱失败！ \n"
                           + m_xlsxSpectrum->objectName()
                           + "文件或许已经打开，请关闭！");
            MY_DEBUG(text);
        }

//        m_license.clear();

        MY_DEBUG("end");
    }

    void saveSpectrumList()
    {
        MY_DEBUG("start");
        m_xlsxSpectrumList->addSheet("Log");
        m_xlsxSpectrumList->addSheet("DOAS");
        m_xlsxSpectrumList->addSheet("DOAS_Bg");
        m_xlsxSpectrumList->addSheet("TDLAS");
        m_xlsxSpectrumList->addSheet("TDLAS_Bg");
        m_xlsxSpectrumList->addSheet("Opacity");
        m_xlsxSpectrumList->addSheet("Opacity_Bg");

        m_xlsxSpectrumList->selectSheet("Log");

        int rowCount = m_xlsxSpectrumList->read("W1").toInt(); //记录总数存储在“W1”
        if(rowCount == 0)
        {
           //车牌号	DOAS	DOAS_Bg	TDLAS	TDLAS_Bg	Opacity	Opacity_Bg
            m_xlsxSpectrumList->write("A1", "时间");
            m_xlsxSpectrumList->write("B1", "车牌号");

           rowCount = 1;
        }

        rowCount += 1;
        int row = rowCount ; //使用列名称A、B、C...时，row是从1开始的

//        QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        m_xlsxSpectrumList->write(QString("A%1").arg(row),m_passTime);

        if(m_license.isEmpty())
        {
            m_license = "---";
        }
        m_xlsxSpectrumList->write(QString("B%1").arg(row),m_license);


        m_xlsxSpectrumList->selectSheet("DOAS");
        QString str;
        for(int i = 0; i < m_testSpectrum.m_doasSpectrumList.count(); i++)
        {
            str.clear();
            QList<float> spectrum = m_testSpectrum.m_doasSpectrumList.at(i);
            foreach(float f, spectrum)
            {
                str.append(QString().setNum(f) + " ");
            }
            m_xlsxSpectrumList->write(row, i + 1, str);
        }


        m_xlsxSpectrumList->selectSheet("DOAS_Bg");
        for(int i = 0; i < m_testSpectrum.m_doasSpectrumListBg.count(); i++)
        {
            str.clear();
            QList<float> spectrum = m_testSpectrum.m_doasSpectrumListBg.at(i);
            foreach(float f, spectrum)
            {
                str.append(QString().setNum(f) + " ");
            }
            m_xlsxSpectrumList->write(row, i + 1, str);
        }

        m_xlsxSpectrumList->selectSheet("TDLAS");
        for(int i = 0; i < m_testSpectrum.m_tdlasSpectrumList.count(); i++)
        {
            str.clear();
            QList<int> spectrum = m_testSpectrum.m_tdlasSpectrumList.at(i);
            foreach(int num, spectrum)
            {
                str.append(QString().setNum(num) + " ");
            }
            m_xlsxSpectrumList->write(row, i + 1, str);
        }

        m_xlsxSpectrumList->selectSheet("TDLAS_Bg");
        for(int i = 0; i < m_testSpectrum.m_tdlasSpectrumListBg.count(); i++)
        {
            str.clear();
            QList<int> spectrum = m_testSpectrum.m_tdlasSpectrumListBg.at(i);
            foreach(int num, spectrum)
            {
                str.append(QString().setNum(num) + " ");
            }
            m_xlsxSpectrumList->write(row, i + 1, str);
        }

        m_xlsxSpectrumList->selectSheet("Opacity");
        for(int i = 0; i < m_testSpectrum.m_opacityADResultsList.count(); i++)
        {
            str.clear();
            QList<int> spectrum = m_testSpectrum.m_opacityADResultsList.at(i);
            foreach(int num, spectrum)
            {
                str.append(QString().setNum(num) + " ");
            }
            m_xlsxSpectrumList->write(row, i + 1, str);
        }

        m_xlsxSpectrumList->selectSheet("Opacity_Bg");
        for(int i = 0; i < m_testSpectrum.m_opacityADResultsListBg.count(); i++)
        {
            str.clear();
            QList<int> spectrum = m_testSpectrum.m_opacityADResultsListBg.at(i);
            foreach(int num, spectrum)
            {
                str.append(QString().setNum(num) + " ");
            }
            m_xlsxSpectrumList->write(row, i + 1, str);
        }


        m_xlsxSpectrumList->selectSheet("Log");
        m_xlsxSpectrumList->write(QString("W1"), row);

        if(rowCount % 20 == 0)
        {
            m_xlsxSpectrumList->save();
        }

//        bool isOk = m_xlsxSpectrumList->save();
//        if(isOk == false)
//        {
//            QString text("保存测量光谱失败！ \n"
//                           + m_xlsxSpectrumList->objectName()
//                           + "文件或许已经打开，请关闭！");
//            MY_DEBUG(text);
//        }

//        m_license.clear();

        MY_DEBUG("end");
    }
};

class ControlCenter:public QObject
{
    Q_OBJECT

private:
    AccessDatabase m_accessDatabase;
    MainWindow     m_mainWindow;
    Weather       *m_weather;
    Login          m_loginDialog;

    WeatherReport  m_weatherReport;

    CarTestResults m_carTestResults;
    NetTestSpectrum m_testSpectrum;
    led_control    m_led;

    BroadcastCaliStatus m_broadcastCaliStatus;

    QPixmap        m_captruedCarPic;

    QList<PropertyAndVariant> m_testResultList;

    qint64         m_triggerTime; //触发时间
    qint64         m_captureTime; //抓拍时间
    qint64         m_gasTime;     //接收到尾气检测结果时间
    qint64         m_carLeftTime; //车尾离开光路触发时间

    SaveTestSpectrum m_saveTestSpectrum; //用于保存测量、背景光谱

    QTimer m_generateResultTimer;
    bool   m_isGenerateResult; //是否强制生成一条测量结果

public:
    ControlCenter();

private:
    bool isTimeReasonable();
    bool isPassed(NetExhaustResults exhaustResults, bool isDiesel);
    bool isValid(bool isDiesel, int vsp, int acc);
    void appendResult(QString property, QVariant variant);
    void processTestResults(NetExhaustResults *exhaustResults);
    float calcVSP(NetSpeedResults speedResults);

signals:
    void testResultReady(CarTestResults oneCarResults);

private slots:
    void updateWeather(WeatherReport data);
    void receiveCaptureInfo(CaptureResults my_NET_ITS_PLATE_RESULT);
    void dataPacketReady(SocketPacket socketPacket);
    void loginFinished();
    bool processTestResults();
    void hasClient(bool value);
};

#endif // CONTROLCENTER_H
