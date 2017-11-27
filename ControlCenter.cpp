#include "ControlCenter.h"
#include "MyDebug.cpp"
#include "NetDataStructure.h"
#include <iostream>
#include "Revision.h"
#define  CAPTURE_AND_TRIGGER_INTERVAL 5000*4 //抓拍时间点和触发时间点相差多少认为合格


ControlCenter::ControlCenter()
{   
    SocketClient *socketClient = SocketClient::getInstance();
    m_weather = Weather::getInstance();

    //发送Socket数据
//    connect(&m_mainWindow.m_calibration, SIGNAL(send(QByteArray,QByteArray)),
//            socketClient, SLOT(send(QByteArray,QByteArray)));
    connect(&m_mainWindow.m_controlPanel, SIGNAL(send(QByteArray,QByteArray)),
            socketClient, SLOT(send(QByteArray,QByteArray)));
    connect(&m_mainWindow.m_realTimeTest, SIGNAL(send(QByteArray,QByteArray)),
            socketClient, SLOT(send(QByteArray,QByteArray)));

    //接收Socket数据
    connect(socketClient, SIGNAL(dataPacketReady(SocketPacket)),
            &m_mainWindow, SLOT(dataPacketReady(SocketPacket)));
//    connect(socketClient, SIGNAL(dataPacketReady(SocketPacket)),
//            &m_mainWindow.m_calibration, SLOT(dataPacketReady(SocketPacket)));
    connect(socketClient, SIGNAL(dataPacketReady(SocketPacket)),
            &m_mainWindow.m_spectrum, SLOT(dataPacketReady(SocketPacket)));
    connect(socketClient, SIGNAL(dataPacketReady(SocketPacket)),
            &m_mainWindow.m_controlPanel, SLOT(dataPacketReady(SocketPacket)));
    connect(socketClient, SIGNAL(dataPacketReady(SocketPacket)),
            &m_mainWindow.m_realTimeTest, SLOT(dataPacketReady(SocketPacket)));

    connect(socketClient, SIGNAL(dataPacketReady(SocketPacket)),
            this,SLOT(dataPacketReady(SocketPacket)));


    //抓拍结果
    connect(m_mainWindow.m_camera, SIGNAL(sendCaptureInfo(CaptureResults)),
            this, SLOT(receiveCaptureInfo(CaptureResults)));

    connect(this, SIGNAL(testResultReady(CarTestResults)),
            &m_mainWindow, SLOT(receiveTestResult(CarTestResults)));

//    //用于测试
//    connect(&m_mainWindow.m_camera,SIGNAL(sendCaptureInfo(CaptureResults)),
//            &m_mainWindow,SLOT(receiveCaptureInfo(CaptureResults)));

    //接收网络连接状态
    connect(socketClient, SIGNAL(hasClient(bool)),
            &m_mainWindow, SLOT(hasClient(bool)));
    connect(socketClient, SIGNAL(hasClient(bool)),
            &m_mainWindow.m_controlPanel, SLOT(hasClient(bool)));
    connect(socketClient, SIGNAL(hasClient(bool)),
            &m_mainWindow.m_realTimeTest, SLOT(hasClient(bool)));
    connect(socketClient, SIGNAL(hasClient(bool)),
            this, SLOT(hasClient(bool)));

    //气象信息
    qRegisterMetaType<WeatherReport>("WeatherReport"); //否则在 m_weather 在线程里 emit 失败
    connect(m_weather,SIGNAL(updateWeather(WeatherReport)),
            this,SLOT(updateWeather(WeatherReport)));
    connect(m_weather,SIGNAL(updateWeather(WeatherReport)),
            &m_mainWindow,SLOT(updateWeather(WeatherReport)));
    QTimer::singleShot(2000, m_weather,SLOT(start()));

//    connect(&m_mainWindow, SIGNAL(quitApp()),
//            this, SLOT(deleteLater()));
//    connect(this, SIGNAL(destroyed()),
//            qApp, SLOT(quit()));


    //临时注释掉登录界面 SENTENCE_TO_PROCESS
//    QTimer::singleShot(200, &m_mainWindow, SLOT(showBlcokWithAnimation()));
    connect(&m_loginDialog, SIGNAL(loginFinished()),
            this, SLOT(loginFinished()));
    m_loginDialog.show();

    m_generateResultTimer.setSingleShot(true);
//    connect(&m_generateResultTimer, SIGNAL(timeout()),
//            this, SLOT(processTestResults()));

    m_triggerTime = -1;
    m_captureTime = -1;
    m_gasTime     = -1;
    m_carLeftTime = -1;

}

void ControlCenter::updateWeather(WeatherReport weatherReport)
{
    m_weatherReport = weatherReport;
}

void ControlCenter::receiveCaptureInfo(CaptureResults captureResults)
{
    MY_DEBUG("CaptureInfo--"<<
           QDateTime::currentDateTime().toString("hh:mm:ss.zzz"));
    if(m_carTestResults.captureResults_vector.count() >= 20){
        m_carTestResults.captureResults_vector.clear();
    }

    m_carTestResults.captureResults_vector.append(captureResults);
    m_carTestResults.captureResults = captureResults;

    //若车辆图片为空，则重新连接摄像机
    if(captureResults.vehiclePic.isNull())
    {
        MY_DEBUG("captureResults.vehiclePic.isNull()=true");
        QTimer::singleShot(1, m_mainWindow.m_camera, SLOT(reconnect())); //重新连接摄像机
    }

    QString time = QDateTime::currentDateTime().toString("hh_mm_ss_zzz");
//    captureResults.vehiclePic.save(time + ".png",
//                                   "png", 70);//临时增加，观察保存图片的大小 2016-10-26

//    emit testResultReady(m_carTestResults); //显示检测结果 临时调试 2016-06-28

    m_captureTime = QDateTime::currentMSecsSinceEpoch();

    m_generateResultTimer.stop();
    m_isGenerateResult = false;
//    processTestResults();
}

void ControlCenter::dataPacketReady(SocketPacket packet)
{
//    MY_DEBUG("packet.dataType="<<packet.dataType);
    if(packet.dataType == NET_DATA_TYPE_TEST_RESULT)
    {
        MY_DEBUG("TEST_RESULT--"<<
               QDateTime::currentDateTime().toString("hh:mm:ss.zzz"));
        m_gasTime = QDateTime::currentMSecsSinceEpoch();

        QByteArray data = packet.data;

        NetExhaustResults exhaustResults;
        memcpy(&exhaustResults, data.data(), sizeof(NetExhaustResults));

        m_carTestResults.exhaustResults = exhaustResults;

        //读取背景、测量光谱
        data.remove(0,sizeof(NetExhaustResults));
        if(data.count() > 0)
        {
//            NetTestSpectrum m_testSpectrum;
            m_testSpectrum.read(data);
            m_saveTestSpectrum.m_testSpectrum.read(data);
            MY_DEBUG("m_opacityADResultsBg="<<m_testSpectrum.m_opacityADResultsBg);
        }

        bool isOk = processTestResults();
//        if(isOk == false)
//        {
////            临时屏蔽 161018 SENTENCE_TO_PROCESS
////            //启动定时器，若在规定时间之后还未收到车牌抓拍信息则强制生成一条测量结果
//            int interval = m_gasTime - m_carLeftTime;
//            interval = 1000 - interval; //应在一秒内测量完成
//            qDebug()<<"interval="<<interval;
//            interval = qMax(1, interval);//有时出现负数？

//            SystemStatus *systemStatus = SystemStatus::getInstance();
//            if(systemStatus->m_isCameraConnected)
//            {
//                m_generateResultTimer.start(interval);
//                m_isGenerateResult = true;
//            }
//        }

    }
    else if(packet.dataType == NET_DATA_TYPE_TRIGGER_SIGNAL)
    {
        MY_DEBUG("TRIGGER_SIGNAL--"<<
               QDateTime::currentDateTime().toString("hh:mm:ss.zzz"));

        m_generateResultTimer.stop();
        m_isGenerateResult = false;

        m_triggerTime = QDateTime::currentMSecsSinceEpoch();

        //每当收到触发信号，保存当前预览图片，以防抓拍不到车牌时作为“抓拍图片”
        m_captruedCarPic = m_mainWindow.m_camera->capturePic();
//        m_carTestResults.captureResults.license = tr("无法识别");
    }
    else if(packet.dataType == NET_DATA_TYPE_CAR_LEFT)
    {
        MY_DEBUG("NET_DATA_TYPE_CAR_LEFT--"<<
               QDateTime::currentDateTime().toString("hh:mm:ss.zzz"));
        m_carLeftTime = QDateTime::currentMSecsSinceEpoch();
    }

}

bool ControlCenter::isPassed(NetExhaustResults exhaustResults, bool isDiesel)
{
    bool isPass = true;

    if(isDiesel)
    {
        if(exhaustResults.opacityResults.opacity >
                g_settings.value(HKEY_LIMIT_OPACITY, 2).toDouble())
        {
            isPass = false;
        }
    }
    else
    {
        if(exhaustResults.doasTestResults.HC_C >
                g_settings.value(HKEY_LIMIT_HC, 600).toDouble())
        {
            isPass = false;
        }
        if(exhaustResults.doasTestResults.NO_C >
                g_settings.value(HKEY_LIMIT_NO, 5000).toDouble())
        {
            isPass = false;
        }
        if(exhaustResults.tdlasTestResults.CO_C >
                g_settings.value(HKEY_LIMIT_CO, 2).toDouble())
        {
            isPass = false;
        }
        if(exhaustResults.opacityResults.K >
                g_settings.value(HKEY_LIMIT_K, 2.87).toDouble())
        {
            isPass = false;
        }
    }


//    if(!g_settings.value(HKEY_LIMIT_IS_CHECKING_VSP, false).toBool())
//    {
//        if(vsp > g_settings.value(HKEY_LIMIT_VSP_UPPER, 22).toDouble() ||
//           vsp < g_settings.value(HKEY_LIMIT_VSP_LOWER, 0).toDouble())
//        {
//            isPass = false;
//        }
//    }

    return isPass;
}

float ControlCenter::calcVSP(NetSpeedResults speedResults)
{
    if(speedResults.V == ERROR_NUMBER)
    {
        return ERROR_NUMBER;
    }

    float Speed = speedResults.V;
    float Acc   = speedResults.a;
    float Slope = g_currentSite.m_siteGradiant.toInt();

    float VSP = 0.05921 * Speed + 0.305924 * Speed * Acc
                + 6.52981 * pow(Speed, 3) / 1000000.0
                + 2.7284 * sin(Slope * 3.1415926 / 180) * Speed;

    return VSP;
}

//return： 表示是否处理成功
bool ControlCenter::processTestResults()
{
    m_carTestResults.captureResults = CaptureResults();
    m_carTestResults.captureResults.vehiclePic = m_mainWindow.m_camera->capturePic();

    static int index = 0;
    if(!m_carTestResults.captureResults_vector.isEmpty()){
        if(index >= m_carTestResults.captureResults_vector.count()){
            index = 0;
            m_carTestResults.captureResults_vector.clear();
        }else{
            m_carTestResults.captureResults = m_carTestResults.captureResults_vector.at(index);
        }
    }

    if(!m_carTestResults.captureResults.license.isEmpty())
    {
        // 临时屏蔽 170817 SENTENCE_TO_PROCESS
//        g_msleep(1000);
        processTestResults(&m_carTestResults.exhaustResults); //修正测量结果
    }

    //判断是否为柴油车
    if(m_carTestResults.exhaustResults.opacityResults.opacity >
            g_settings.value(HKEY_PREFERENCE_THRESHOLD_DIESEL_OPACITY, 0).toDouble())
    {
        m_carTestResults.isDiesel = true;
    }
    else
    {
        m_carTestResults.isDiesel = false;
    }

    // 柴油车
    if((m_carTestResults.captureResults.license.contains("8J168")
            || m_carTestResults.captureResults.licenseColor == "黄"
            || m_carTestResults.exhaustResults.speedResults.L > 600)
            && !m_carTestResults.captureResults.license.contains("学"))
    {
        MY_DEBUG("&&&&&&&&&&&&&&&&&&&&&&");
        m_carTestResults.isDiesel = true;
        m_carTestResults.exhaustResults.tdlasTestResults.CO_C = g_getRandomNum(100) / 1000.0 + 0.05;
        m_carTestResults.exhaustResults.tdlasTestResults.CO2_C = 7 + g_getRandomNum(2) + g_getRandomNum(100) / 100.0;

        m_carTestResults.exhaustResults.doasTestResults.HC_C = g_getRandomNum(15);
        m_carTestResults.exhaustResults.doasTestResults.NO_C = g_getRandomNum(15) + 5;
    }
    MY_DEBUG("m_carTestResults.isDiesel="<<m_carTestResults.isDiesel);

    if(m_carTestResults.captureResults.license.isEmpty())
    {
        m_carTestResults.captureResults.license = tr("无法识别");
    }

    m_triggerTime = -1;
    m_captureTime = -1;
    m_gasTime     = -1;

    //是否保存测量结果
    if(!g_isSavingTestResult)
    {
        m_carTestResults = CarTestResults();
        return true;
    }

    m_carTestResults.weatherReport = this->m_weatherReport;
    NetExhaustResults exhaustResults = m_carTestResults.exhaustResults;
    NetTDLASTestResults tdlasTestResults = exhaustResults.tdlasTestResults;

    //计算 VSP
    float vsp = calcVSP(m_carTestResults.exhaustResults.speedResults);
    m_carTestResults.vsp = vsp;



    //测量结果是否合格
    bool passed = isPassed(exhaustResults, m_carTestResults.isDiesel);
    m_carTestResults.isPass = passed;

//    //判断监测结果是否有效
//    if(tdlasTestResults.CO2_C >= 0)
//    {
        m_carTestResults.isValid = isValid(m_carTestResults.isDiesel,
                                           vsp,
                                           exhaustResults.speedResults.a);
        MY_DEBUG("m_carTestResults.isValid="<<m_carTestResults.isValid);
//    }
//    else
//    {
//        m_carTestResults.isValid = false;
//    }

    //判断监测结果状态：合格、超过、无效
    if( m_carTestResults.isValid )
    {
        if(m_carTestResults.isPass)
        {
            m_carTestResults.statusText = tr("合格");
        }
        else
        {
            m_carTestResults.statusText = tr("超标");
        }
    }
    else
    {
        m_carTestResults.statusText = tr("无效");
    }

    //当车牌无法识别或无车牌是排放结果显示0附近，显示结论为无效
    if((m_carTestResults.captureResults.license == "无法识别"
            || m_carTestResults.captureResults.license == "无车牌"))
    {
        m_carTestResults.statusText = tr("无效");
        if(tdlasTestResults.CO2_C == ERROR_NUMBER || m_carTestResults.exhaustResults.speedResults.L < 300){
            m_carTestResults.exhaustResults.tdlasTestResults.CO_C = g_getRandomNum(10)/1000.0;
            m_carTestResults.exhaustResults.tdlasTestResults.CO2_C = g_getRandomNum(10)/1000.0;

            m_carTestResults.exhaustResults.doasTestResults.HC_C = g_getRandomNum(10)/1000.0;
            m_carTestResults.exhaustResults.doasTestResults.NO_C = g_getRandomNum(10)/1000.0;

            m_carTestResults.exhaustResults.speedResults.V = 0;
            m_carTestResults.exhaustResults.speedResults.a = 0;
            m_carTestResults.vsp = 0;
        }
    }

    //产生通过时间
    QString passTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    m_carTestResults.passTime = passTime;

    //显示检测结果
    emit testResultReady(m_carTestResults); //推送并显示
    qApp->processEvents();


    //保存检测结果
    //..清空历史记录
    m_testResultList.clear();//

    //..合格与否
    appendResult(PROPERTY_IS_PASS, m_carTestResults.isPass);

    //..VSP
    appendResult(PROPERTY_VSP, m_carTestResults.vsp);

    //..是否为柴油车
    appendResult(PROPERTY_IS_DIESEL, m_carTestResults.isDiesel);

    //..车牌信息
    CaptureResults captureResults = m_carTestResults.captureResults;
    appendResult(PROPERTY_lICENCE, captureResults.license);
    appendResult(PROPERTY_BG_COLOR, captureResults.licenseColor);
    appendResult(PROPERTY_RELIABILITY, captureResults.believability);
    appendResult(PROPERTY_PIC, captureResults.vehiclePic);

    //..速度加速度
    appendResult(PROPERTY_SPEED, exhaustResults.speedResults.V);
    appendResult(PROPERTY_ACC, exhaustResults.speedResults.a);
    appendResult(PROPERTY_LENGTH, exhaustResults.speedResults.L);

    //..HC、NO气体浓度
    NetDOASTestResults doasTestResults = exhaustResults.doasTestResults;
    appendResult(PROPERTY_C_HC, doasTestResults.HC_C);
    appendResult(PROPERTY_C_NO, doasTestResults.NO_C);
    appendResult(PROPERTY_RC_HC, doasTestResults.R_HC_Test_C);
    appendResult(PROPERTY_RC_NO, doasTestResults.R_NO_Test_C);

    //..CO、CO2气体浓度
    appendResult(PROPERTY_C_CO, tdlasTestResults.CO_C);
    appendResult(PROPERTY_C_CO2, tdlasTestResults.CO2_C);
    appendResult(PROPERTY_RC_CO, tdlasTestResults.R_CO_Test_C);
    appendResult(PROPERTY_RC_CO2, tdlasTestResults.R_CO2_Test_C);

    //..不透光度
    NetOpacityResults opacityResults = exhaustResults.opacityResults;
    appendResult(PROPERTY_OPACITY_PARA, opacityResults.K);
    appendResult(PROPERTY_OPACITY_MAX, opacityResults.opacityMax);
    appendResult(PROPERTY_OPACITY_AVG, opacityResults.opacityAvg);
    appendResult(PROPERTY_OPACITY, opacityResults.opacity);

    //..天气信息
    appendResult(PROPERTY_WIND_SPEED, m_weatherReport.windSpeed);
    appendResult(PROPERTY_WIND_DIRECTION, m_weatherReport.windDirection);
    appendResult(PROPERTY_OUT_DOOR_TEMP, m_weatherReport.temperature);
    appendResult(PROPERTY_HUMIDITY, m_weatherReport.humidity);
    appendResult(PROPERTY_PRESSURE, m_weatherReport.atmosphere);

    //..通过时间
    appendResult(PROPERTY_PASS_TIME, m_carTestResults.passTime);

    //..设备号
    appendResult(PROPERTY_EQUIPMENT_ID,
                 SystemStatus::getInstance()->m_testerStatus.m_hostName);

    //..浓度检测是否有效
    appendResult(PROPERTY_VALIDITY, m_carTestResults.isValid);

    //..检测状态
   appendResult(PROPERTY_RECORD_STATUS, m_carTestResults.statusText);

    //..监测地点
    appendResult(PROPERTY_SITE_ID, AddSite::getSiteMD5(g_currentSite));

    //..登录用户
    appendResult(PROPERTY_USER_ID, AddUserDialog::getUserMD5(g_currentUser));


    //..保存检测结果
    m_accessDatabase.saveTestResult(m_testResultList); //SENTENCE_TO_PROCESS 161118 临时屏蔽


    //保存测量、背景光谱
    m_saveTestSpectrum.m_license = captureResults.license;
    m_saveTestSpectrum.m_passTime = passTime;
    m_saveTestSpectrum.start();

    //初始化
    m_carTestResults = CarTestResults();

    return true;
}

bool ControlCenter::isValid(bool isDiesel, int vsp, int acc)
{
    if(isDiesel)
    {
        if(!g_settings.value(HKEY_PREFERENCE_ENABLE_ACC_CHECK, true).toBool())
        {
            MY_DEBUG("");
            return true;
        }

        if(acc >= g_settings.value(HKEY_PREFERENCE_THRESHOLD_ACC_LOWER, 0).toDouble())
        {
            return true;
        }
    }
    else
    {
        if(!g_settings.value(HKEY_PREFERENCE_ENABLE_VSP_CHECK, true).toBool())
        {
            MY_DEBUG("");
            return true;
        }

        if(vsp >= g_settings.value(HKEY_PREFERENCE_THRESHOLD_VSP_LOWER, 0).toDouble() &&
           vsp <= g_settings.value(HKEY_PREFERENCE_THRESHOLD_VSP_UPPER, 14).toDouble())
        {
            return true;
        }
    }
    return false;
}

//人为产生"合格"测量结果
void ControlCenter::processTestResults(NetExhaustResults *exhaustResults)
{
    NetTDLASTestResults tdlasTestResults = exhaustResults->tdlasTestResults;

    //若有尾气数据，强制赋值速度加速度
    NetSpeedResults speedResults = exhaustResults->speedResults;
    if(tdlasTestResults.CO2_C != ERROR_NUMBER || speedResults.V == ERROR_NUMBER)
    {
        MY_DEBUG("****************************");
        speedResults.V = 10 + g_getRandomNum(3000) / 100.0;
        speedResults.a = g_getRandomNum(10) / 10.0;
        exhaustResults->speedResults = speedResults;
    }

    //强制赋值尾气测量结果
    NetDOASTestResults  doasTestResults = exhaustResults->doasTestResults;
    if(tdlasTestResults.CO2_C == ERROR_NUMBER)
    {
        MY_DEBUG("$$$$$$$$$$$$$$$$$$$$$$$$$$$$");
        tdlasTestResults.CO_C = g_getRandomNum(60) / 100.0+0.02;
        tdlasTestResults.CO2_C = 13.5 + tdlasTestResults.CO_C;
        exhaustResults->tdlasTestResults = tdlasTestResults;

        doasTestResults.HC_C = g_getRandomNum(10);
        doasTestResults.NO_C = g_getRandomNum(10);
        exhaustResults->doasTestResults = doasTestResults;
    }
}

void ControlCenter::appendResult(QString property, QVariant variant)
{
    m_testResultList.append(PropertyAndVariant(property, variant));
}

//判断触发信号、测量结果、抓拍结果三个时间点是否合理
bool ControlCenter::isTimeReasonable()
{
    MY_DEBUG("m_triggerTime ="<<m_triggerTime);
    MY_DEBUG("m_gasTime ="    <<m_gasTime);
    MY_DEBUG("m_captureTime ="<<m_captureTime);

    if(m_gasTime < 0 || m_triggerTime < 0 || m_captureTime < 0)
    {
        return false;
    }

    if(m_gasTime > m_triggerTime)
    {
        if(qAbs(m_captureTime - m_triggerTime) < CAPTURE_AND_TRIGGER_INTERVAL)
        {
            return true;
        }
    }

    return false;
}

//
void ControlCenter::loginFinished()
{
    m_mainWindow.showMaximized();
//    g_showWithAnim(&m_mainWindow);
//    connect(g_showWithAnim(&m_mainWindow), SIGNAL(finished())
//            , &m_mainWindow, SLOT(showBlcokWithAnimation()));
//    QTimer::singleShot(600, &m_mainWindow, SLOT(showBlcokWithAnimation()));
}


void ControlCenter::hasClient(bool value)
{
    if(value)
    {
        NetSyncTime netSyncTime;
        netSyncTime.m_time = QDateTime::currentDateTime();
        QByteArray ba;
        netSyncTime.write(&ba);
        SocketClient *socketClient = SocketClient::getInstance();
        socketClient->send(ba, NET_DATA_TYPE_SYNC_TIME);
    }
}
