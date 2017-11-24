#include "Camera.h"
#include <QProcess>
#include <QNetworkInterface>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QDateTime>
#include <QDir>
#include <QSettings>
#include <QFileDialog>
#include <QSignalMapper>
#include <QTextCodec>
#include <QObject>
#include "Global.h"
#include "Constant.h"
#include <QPainter>
#include <QStaticText>
#include <QSettings>
#include "MyDebug.cpp"
#include <QNetworkConfigurationManager>
#include <QHostInfo>

SendCaptureInfo g_SendCaptureInfo;

BOOL CALLBACK MessageCallback(LONG lCommand,
                              NET_DVR_ALARMER *pAlarmer,
                              char *pAlarmInfo,
                              DWORD dwBufLen,
                              void* pUser)
{
//    printf("MessageCallback called\n");

    qDebug()<<"MessageCallback called";
    switch(lCommand)
    {
    case COMM_UPLOAD_PLATE_RESULT:
    {
        qDebug()<<"COMM_UPLOAD_PLATE_RESULT";
        CaptureResults captureResults;//车牌抓拍结果信息
        NET_DVR_PLATE_RESULT struITSPlateResult;

        memcpy(&struITSPlateResult, pAlarmInfo, sizeof(struITSPlateResult));

        //获取车牌置信度
        captureResults.believability = struITSPlateResult
                                            .struPlateInfo
                                            .byEntireBelieve;
        //获取车牌颜色
        QString licenseColor;
        switch(struITSPlateResult.struPlateInfo.byColor)//车牌颜色
        {
        case VCA_BLUE_PLATE:
            licenseColor = QObject::tr("蓝");
            break;
        case VCA_YELLOW_PLATE:
            licenseColor = QObject::tr("黄");
            break;
        case VCA_WHITE_PLATE:
            licenseColor = QObject::tr("白");
            break;
        case VCA_BLACK_PLATE:
            licenseColor = QObject::tr("黑");
            break;
        case VCA_GREEN_PLATE:
            licenseColor = QObject::tr("绿");
        default:
            licenseColor = QObject::tr("其它");
            break;
        }
        captureResults.licenseColor = licenseColor;

        //获取车牌号
        QString carLicense = QTextCodec::codecForLocale()
                ->toUnicode(struITSPlateResult.struPlateInfo.sLicense);


        //..将获取车牌号前面的“颜色”去掉
        if(carLicense.startsWith(licenseColor))
        {
            carLicense = carLicense.remove(licenseColor);
        }
        captureResults.license = carLicense;

        qDebug()<<"car license:"<<carLicense.toUtf8().data();
        //车牌位置
        captureResults.struPlateRect =
                struITSPlateResult.struPlateInfo.struPlateRect;

        //获取场景图和车牌小图片
        QPixmap pixmap;
        if(struITSPlateResult.dwPicPlateLen != 0){//车牌小图片
            bool isOk = pixmap.loadFromData(struITSPlateResult.pBuffer2
                                , struITSPlateResult.dwPicPlateLen);
            MY_DEBUG("load car license pic isOk="<<isOk);
            if(isOk)
            {
                captureResults.licensePic = pixmap;
            }
        }

        if ((struITSPlateResult.dwPicLen != 0)){//获取场景图
            MY_DEBUG("dwDataLen="<<struITSPlateResult.dwPicLen);
            bool isOk = pixmap.loadFromData(struITSPlateResult.pBuffer1
                                , struITSPlateResult.dwPicLen);
            MY_DEBUG("load total pic isOk="<<isOk);
            if(isOk)
            {
                captureResults.vehiclePic = pixmap;
            }
        }

        //发送抓拍结果
        emit g_SendCaptureInfo.sendCaptureInfo(captureResults);

        break;
    }
    case COMM_ITS_PLATE_RESULT:
    {
        NET_ITS_PLATE_RESULT struITSPlateResult = {0};
        memcpy(&struITSPlateResult, pAlarmInfo, sizeof(struITSPlateResult));

        CaptureResults captureResults;

        //获取车牌置信度
        captureResults.believability = struITSPlateResult
                                            .struPlateInfo
                                            .byEntireBelieve;
        //获取车牌颜色
        QString licenseColor;
        switch(struITSPlateResult.struPlateInfo.byColor)//车牌颜色
        {
        case VCA_BLUE_PLATE:
            licenseColor = QObject::tr("蓝");
            break;
        case VCA_YELLOW_PLATE:
            licenseColor = QObject::tr("黄");
            break;
        case VCA_WHITE_PLATE:
            licenseColor = QObject::tr("白");
            break;
        case VCA_BLACK_PLATE:
            licenseColor = QObject::tr("黑");
            break;
        case VCA_GREEN_PLATE:
            licenseColor = QObject::tr("绿");
        default:
            licenseColor = QObject::tr("其它");
            break;
        }
        captureResults.licenseColor = licenseColor;

        //获取车牌号
        QString carLicense = QTextCodec::codecForLocale()
                ->toUnicode(struITSPlateResult.struPlateInfo.sLicense);

        qDebug()<<"car license:"<<carLicense;
        //..将获取车牌号前面的“颜色”去掉
        if(carLicense.startsWith(licenseColor))
        {
            carLicense = carLicense.remove(licenseColor);
        }
        captureResults.license = carLicense;

        //车牌位置
        captureResults.struPlateRect =
                struITSPlateResult.struPlateInfo.struPlateRect;

        //获取场景图和车牌小图片
        for(int i = 0; i < struITSPlateResult.dwPicNum; i++)
        {
            QPixmap pixmap;

            //获取车牌小图片
            if ((struITSPlateResult.struPicInfo[i].dwDataLen != 0) &&
                    (struITSPlateResult.struPicInfo[i].byType == 0))
            {
                bool isOk = pixmap.loadFromData(struITSPlateResult.struPicInfo[i].pBuffer
                                    , struITSPlateResult.struPicInfo[i].dwDataLen);
                MY_DEBUG("isOk="<<isOk);
                if(isOk)
                {
                    captureResults.licensePic = pixmap;
                }
            }

            //获取场景图
            if ((struITSPlateResult.struPicInfo[i].dwDataLen != 0) &&
                (struITSPlateResult.struPicInfo[i].byType == 1))
            {
                MY_DEBUG("dwDataLen="<<struITSPlateResult.struPicInfo[i].dwDataLen);
                bool isOk = pixmap.loadFromData(struITSPlateResult.struPicInfo[i].pBuffer
                                    , struITSPlateResult.struPicInfo[i].dwDataLen);
                MY_DEBUG("isOk="<<isOk);
                if(isOk)
                {
                    captureResults.vehiclePic = pixmap;
                }
            }
        }

        //发送抓拍结果
        emit g_SendCaptureInfo.sendCaptureInfo(captureResults);

        //SENTENCE_TO_PROCESS 161118 临时增加
//        static int n = 1;
//        MY_DEBUG("license="<<captureResults.license<<","<<"n="<<n);
//        QString passTime = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
//        MY_DEBUG("passTime="<<passTime);
//        n++;

        break;
    }
    default:
        break;
    }
}

Camera::Camera()
{
    m_hWnd = NULL;
    m_lRealPlayHandle = INIT_NUMBER;
    m_lAlarmHandle = INIT_NUMBER;
    m_isReconnect = false;
    m_connected = false;

    m_stopped = false;

    m_process = new QProcess();

    qRegisterMetaType<CaptureResults>("CaptureResults");
    connect(&g_SendCaptureInfo,SIGNAL(sendCaptureInfo(CaptureResults)),
            this,SLOT(receiveCaptureInfo(CaptureResults)));

    connect(this, SIGNAL(loginOk(bool)),
            this, SLOT(connectionChanged(bool)));

    this->start();//可以及时发通知：抓拍机是否已连接
    MY_DEBUG("Camera start");
}

Camera::~Camera()
{
    qDebug()<<"\n~Camera()\n";
    this->stop();
//    this->wait();
    clearUP();
    delete m_process;
}

//使用单例模式
Camera *Camera::getInstance()
{
    static Camera instance; //局部静态变量，若定义为指针，则需要手动释放内容
    return &instance;
}

//释放摄像机资源
void Camera::clearUP()
{
    MY_DEBUG("start");
    if (!NET_DVR_CloseAlarmChan_V30(m_lAlarmHandle))
    {
        qDebug("NET_DVR_CloseAlarmChan_V30 error, %d\n", NET_DVR_GetLastError());
    }

    //停止监听
    if (!NET_DVR_StopListen_V30(m_lListenHandle))
    {
        qDebug("NET_DVR_StopListen_V30 error, %d\n", NET_DVR_GetLastError());
    }


    //关闭预览
    NET_DVR_StopRealPlay(m_lRealPlayHandle);
    //注销用户
    NET_DVR_Logout(m_lUserID);
    //释放SDK资源
    NET_DVR_Cleanup();
    MY_DEBUG("end");
}

void Camera::run()
{
    static int error_count = 0;

    //搜索在线 camera
    while(1)
    {
        if(g_ignoringCamera)
        {
            if(!m_connected)
            {
                m_connected = true;
                emit loginOk(m_connected);
            }
            sleep(1);
            continue;
        }
        else
        {
            if(m_connected)
            {
                m_connected = false;
                emit loginOk(m_connected);
            }
        }

        QStringList cameraIpList = getAllOnlineCamera();
        MY_DEBUG("cameraIpList="<<cameraIpList);

        cameraIpList.append("192.168.1.251");
        //刚启动软件时，若存在历史连接记录则重新连接，否则随机连接一个IP
        if(cameraIpList.count())
        {
            QString ip = g_settings.value(HKEY_CAMERA_IP, "192.0.0.64").toString();
            if(cameraIpList.contains(ip))
            {
                m_cameraIp = ip;
            }
            else
            {
                m_cameraIp = cameraIpList.first();
            }

            emit availableCamera( cameraIpList );
            break;
        }
        else
        {
            sleep(1);
        }
    }

    while(1)
    {
        if(g_ignoringCamera)
        {
            if(!m_connected)
            {
                m_connected = true;
                emit loginOk(m_connected);
            }
            sleep(1);
            continue;
        }

        //登录 camera
        bool isOk = loginCamera();
        if(m_connected != isOk)
        {
            m_connected = isOk;
            emit loginOk(m_connected);
        }

        //登录失败，延时等待
        if(isOk == false)
        {
            int count = 20;
            while(count--)
            {
                msleep(50);
            }
            continue;
        }

        //获取抓拍机状态，若获取出错，说明网络连接异常，需要重新连接
        while(1)
        {
            bool isReconnect = false;

            if(anyNetworkInterfaceRunning())
            {
                int times = 6;
                while(times--)
                {
//                    NET_DVR_VCA_VERSION vcpVersion;
//                    bool isOk = NET_DVR_GetVCAVersion(m_lUserID, 1, &vcpVersion);
                    char buf[100];
                    bool isOk=NET_DVR_RemoteControl(m_lUserID, NET_DVR_CHECK_USER_STATUS,buf,100);
                    if(isOk == false){
                        error_count++;
                    }else{
                        error_count=0;
                    }
//                    MY_DEBUG("isOk ="<<isOk);
                    if(error_count>5)
                    {
//                        MY_DEBUG("times ="<<times);
                        isReconnect = true;
                        msleep(500);
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else
            {
                isReconnect = true;
            }

            if(!isReconnect)
            {
                //m_isReconnect置为true，软件有意重连
                QMutexLocker locker(&m_reconnectMutex);
                if(m_isReconnect)
                {
                    isReconnect = true;
                }
            }

            if(isReconnect)//网络异常，需要重新连接
            {
                MY_DEBUG("isReconnect = true");
                clearUP();//加上这一句，否则断网后重新连接网络无法调节云台 2016-07-11
                m_isReconnect = false;
                emit loginOk(false);
                break;
            }

            msleep(2000);
        }
    }
}

//是否有网卡处于连接状态
bool Camera::anyNetworkInterfaceRunning()
{
    QList<QNetworkInterface> networkInterfaceList = QNetworkInterface::allInterfaces();
    foreach(QNetworkInterface netInterface, networkInterfaceList)
    {
        QNetworkInterface::InterfaceFlags flags = netInterface.flags();
        if (flags.testFlag(QNetworkInterface::IsRunning)
                && !flags.testFlag(QNetworkInterface::IsLoopBack)) // 网络接口处于活动状态
        {
            QList<QNetworkAddressEntry> entryList = netInterface.addressEntries();
            foreach(QNetworkAddressEntry entry, entryList)
            {
                if(entry.ip().protocol() == QAbstractSocket::IPv4Protocol);
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool Camera::isStopNow()
{
    QMutexLocker locker(&m_runningMutex);
    bool stopped = m_stopped;
    if(m_stopped == true)
    {
        m_stopped = false;
    }
    return stopped;
}

bool Camera::loginCamera()
{
//    QHostInfo host;
////    MY_DEBUG("mgr.isOnline()"<<mgr.isOnline());
//    if(mgr.isOnline() == false)
//    {
//        return false;
//    }
//    MY_DEBUG("start");

    if(m_cameraIp.isEmpty())
    {
        return false;
    }

    QString IP   = m_cameraIp/*g_settings.value(HKEY_CAMERA_IP,"192.0.0.64").toString()*/;
    int     port = g_settings.value(HKEY_CAMERA_PORT,"8000").toInt();
    QString user = g_settings.value(HKEY_CAMERA_USER,"admin").toString();
    QString password = g_settings.value(HKEY_CAMERA_PASSWORD,"zdmq8888").toString();

    qDebug()<<"IP="<<IP<<"port="<<port<<"user="<<user<<"pass="<<password;

    if(IP.contains(QRegExp("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}"))==false)
    {
//        QMessageBox::warning(NULL,"提示","IP地址不合法!",QMessageBox::Ok);
        MY_DEBUG(tr("IP地址不合法!"));
        return false;
    }

    // 初始化
    NET_DVR_Init();
    //设置连接时间与重连时间
    NET_DVR_SetConnectTime(2000, 1);
    NET_DVR_SetReconnect(10000, true);

    // 注册设备
    //    LONG m_lUserID;
//    NET_DVR_DEVICEINFO_V30 struDeviceInfo;
    NET_DVR_DEVICEINFO_V40 struDeviceInfo;
    NET_DVR_USER_LOGIN_INFO strLogInfo;

    strcpy(strLogInfo.sUserName,user.toUtf8().data());
    strcpy(strLogInfo.sPassword,password.toUtf8().data());
    strLogInfo.wPort=port;
    strcpy(strLogInfo.sDeviceAddress,IP.toUtf8().data());
    strLogInfo.bUseAsynLogin=0;
    strLogInfo.byRes1=0;
//    strLogInfo.byRes2=NULL;

//    m_lUserID = NET_DVR_Login_V30(IP.toUtf8().data()
//                                  , port
//                                  , user.toUtf8().data()
//                                  , password.toUtf8().data()
//                                  , &struDeviceInfo);
    m_lUserID = NET_DVR_Login_V40(&strLogInfo,&struDeviceInfo);
    if (m_lUserID < 0)
    {
        /* NET_DVR_Init和NET_DVR_Cleanup需要配对使用，
         * 即程序里面调用多少次NET_DVR_Init，退出时就需
         * 要调用多少次NET_DVR_Cleanup
         */
        NET_DVR_Cleanup();

        MY_LOG("NET_DVR_Login_V40 error");

        return false;
    }
    qDebug()<<"m_lUserID:"<<m_lUserID;
    startAlarm();//启用报警设置，识别到车牌后自动调用函数处理

    if(m_hWnd != NULL)
    {
        qDebug()<<"启动预览并设置回调数据流";
        startPreview();//启动预览并设置回调数据流
    }

//    msleep(1000*1000);
    return true;
}

//启动预览并设置回调数据流
bool Camera::startPreview()
{
    MY_LOG("start");
//    qDebug()<<"startPreview() called !";

    NET_DVR_PREVIEWINFO struPlayInfo = {0};

    //播放窗口的句柄，为NULL表示不解码显示,需要SDK解码时句柄设为有
    //效值，仅取流不解码时可设为空
    struPlayInfo.hPlayWnd = m_hWnd;

    struPlayInfo.lChannel     = 1;       //预览通道号

    //0-主码流，1-子码流，在户外还是子码流传输比较流畅 170421
    struPlayInfo.dwStreamType = 1;

    struPlayInfo.dwLinkMode   = 0;

    m_lRealPlayHandle = NET_DVR_RealPlay_V40(m_lUserID, &struPlayInfo, NULL, NULL);
    if (m_lRealPlayHandle < 0)
    {
        MY_LOG("NET_DVR_RealPlay_V40 error");
//        QMessageBox::information(NULL,
//                                 "提示",
//                                 "NET_DVR_RealPlay_V40 error!",
//                                 QMessageBox::Ok);

//        qDebug("NET_DVR_RealPlay_V40 error, %d\n"
//               , NET_DVR_GetLastError());
//        NET_DVR_Logout(m_lUserID);
//        NET_DVR_Cleanup();
        return false;
    }

    MY_LOG("end");
    return true;
}

//重置预览窗口（通过重新连接来实现）
void Camera::reStartPreview(HWND hWnd)
{
    if(m_hWnd == hWnd)
    {
        MY_DEBUG("");
        return;
    }

//    QMutexLocker locker(&m_reconnectMutex);
//    m_isReconnect = true; // 重新连接
    this->m_hWnd = hWnd;

//    LONG oldHandle = m_lRealPlayHandle;

    startPreview();//启动预览并设置回调数据流

//    NET_DVR_StopRealPlay(oldHandle);//关闭预览
}

//启用报警设置，识别到车牌后自动调用函数处理
bool Camera::startAlarm()
{
    qDebug()<<"startAlarm() called !";
    //设置报警回调函数
    bool isOK = NET_DVR_SetDVRMessageCallBack_V31(MessageCallback, NULL);

//    bool isOK =NET_DVR_StartListen_V31(MessageCallback,NULL);
//    qDebug()<<"isOK="<<isOK<<"(>=0 is true)";


    //启用布防
    NET_DVR_SETUPALARM_PARAM struSetupParam = {0};
    struSetupParam.dwSize = sizeof(NET_DVR_SETUPALARM_PARAM);

    //上传报警信息类型:
    //0- 老报警信息(NET_DVR_PLATE_RESULT),
    //1- 新报警信息(NET_ITS_PLATE_RESULT)
    struSetupParam.byAlarmInfoType = 0;

    //布防等级
    struSetupParam.byLevel = 0;

//    qDebug()<<"m_lAlarmHandle:"<<m_lAlarmHandle;
    m_lAlarmHandle = NET_DVR_SetupAlarmChan_V41(m_lUserID,&struSetupParam);
    if (m_lAlarmHandle < 0)
    {


        for(int i=0;i<5;i++){
            this->clearUP();
            m_lAlarmHandle = NET_DVR_SetupAlarmChan_V41(m_lUserID,&struSetupParam);
//            qDebug()<<"NET_DVR_SetupAlarmChan_V41 time:"<<i+1;
            msleep(500);
            if(m_lAlarmHandle>=0){
                return true;
            }
        }
        qDebug("NET_DVR_SetupAlarmChan_V41 error, %d\n", NET_DVR_GetLastError());
        return false;
    }
    return true;
}

void Camera::controlYunTai(DWORD dwPTZCommand,DWORD dwStop)
{
    static DWORD old_dwPTZCommand = PAN_RIGHT;
//    qDebug()<<"b dwPTZCommand="<<dwPTZCommand<<"dwStop="<<dwStop;

    bool isOK;
    if(dwStop == 0)
    {
        old_dwPTZCommand = dwPTZCommand;
        isOK = NET_DVR_PTZControl(m_lUserID, dwPTZCommand, dwStop);
    }
    else
    {
        isOK = NET_DVR_PTZControl(m_lUserID, old_dwPTZCommand, dwStop);
//        MY_DEBUG("error="<<NET_DVR_GetLastError());
    }

    //出现调节云台次序错误，校准回来
    if(isOK == false && NET_DVR_ORDER_ERROR == NET_DVR_GetLastError())
    {
        //...暂时没有好的解决方法，只能重启软件了2016-07-11

        MY_DEBUG("error=NET_DVR_ORDER_ERROR,调用次序错误。");
    }
}

//手动截取视频流图片
QPixmap Camera::capturePic()
{
//    DWORD lpSizeReturned;
//    DWORD dwPicSize = 1024 * 1024 * 10;//申请10M内存
//    char *sJpegPicBuffer = new char[dwPicSize];
//    NET_DVR_JPEGPARA   lpJpegPara;
//    lpJpegPara.wPicSize = 0xff;
//    lpJpegPara.wPicQuality = 0;
//    int channel = 1;
//    bool success = NET_DVR_CaptureJPEGPicture_NEW(m_lUserID
//                                                  ,channel
//                                                  ,&lpJpegPara
//                                                  ,sJpegPicBuffer
//                                                  ,dwPicSize
//                                                  ,&lpSizeReturned);
//    channel ++;
//    MY_DEBUG("channel="<<channel);
//    MY_DEBUG("lpSizeReturned="<<lpSizeReturned);
//    MY_DEBUG("success="<<success);
//    QPixmap vehiclePic;
//    if(success == true)
//    {
//        vehiclePic.loadFromData((uchar *)sJpegPicBuffer, lpSizeReturned);
//    }
//    delete[] sJpegPicBuffer;
//    vehiclePic.save(QString("pic_%1.jpg").arg(lpSizeReturned));
    QString filename;
    QPixmap vehiclePic;

    filename = QDir::currentPath() + "\capture_pic.jpg";

//    qDebug()<<"CapturePic filename:"<<filename;
    //设置抓图模式
    NET_DVR_SetCapturePictureMode(0);

    //抓图并保存
    bool success = NET_DVR_CapturePicture(m_lUserID,"D:\\1.bmp"); //filename.toLatin1().data());
    if(success == true)
    {
        vehiclePic.load("D:\\1.bmp");
    }
    return vehiclePic;
}

//重新连接摄像机（通常是由于多辆车同时通过上传抓拍信息过于密集造成）
void Camera::reconnect(QString cameraIp)
{
    MY_DEBUG("cameraIp="<<cameraIp);
    if(!cameraIp.isEmpty() && cameraIp != m_cameraIp)
    {
        this->m_cameraIp = cameraIp;
        m_isReconnect = true;
    }
}

QPixmap Camera::processPixmap(QPixmap mapIn,CaptureResults captureResults)
{
    QPixmap tmpmap;
    QSize pic_size;

    NET_VCA_RECT struPlateRect = captureResults.struPlateRect;

    pic_size = mapIn.size();

    float posX = pic_size.width() * struPlateRect.fX;
    float posY = pic_size.height() * struPlateRect.fY;
    int lisencePicW = pic_size.width() * struPlateRect.fWidth;
    int lisencePicH = pic_size.height() * struPlateRect.fHeight;

    //防止图片超界
    float x,y;
    x = qMin(pic_size.width()/3*2,(int)(posX + lisencePicW / 2 - pic_size.width()/6));
    x = qMax(0,(int)(posX + lisencePicW / 2-pic_size.width()/6));
    y = qMin(pic_size.height()/2,(int)(posY + lisencePicH/2 - pic_size.width()/4));
    y = qMax(0,(int)(posY + lisencePicH/2- pic_size.width()/4));



    tmpmap = mapIn.copy(x ,y ,pic_size.width()/3,pic_size.height()/2);

    return tmpmap;
}

//主要是将场景图裁剪得出只包含辆车的图片
void Camera::receiveCaptureInfo(CaptureResults captureResults)
{
    QPixmap pixmap = captureResults.vehiclePic;
    if(pixmap.isNull())
    {
        emit sendCaptureInfo(captureResults);
        return;
    }

    //判断车牌在不在牌识区内
    //..获取车牌小图片左上角坐标
    NET_VCA_RECT struPlateRect = captureResults.struPlateRect;
    float posX = pixmap.width() * struPlateRect.fX;
    float posY = pixmap.height() * struPlateRect.fY;

    //..生成牌识区多边形
    QVector<QPoint> points;
    points << g_PlateRecogRegion_P1
           << g_PlateRecogRegion_Top
           << g_PlateRecogRegion_Right
           << g_PlateRecogRegion_P3
           << g_PlateRecogRegion_P4;
    QPolygon polygon(points);

    QPainterPath path;
    path.addPolygon(polygon);
    if(path.contains(QPointF(posX, posY)) == false)
    {
        emit sendCaptureInfo(captureResults);
        return; //车牌不在牌识区内返回
    }

    int lisencePicW = pixmap.width() * struPlateRect.fWidth; //车牌图片宽度

    int leftTopX = posX - 2.88 * lisencePicW; //裁剪框左上角的x坐标
    int leftTopY = posY - 2.7  * lisencePicW;

    leftTopX = qMax(0, leftTopX); //不能为负数
    leftTopY = qMax(0, leftTopY);

    int cropW = (2.88 + 2.45) * lisencePicW; //裁剪框的宽度
    int cropH = (2.7  + 1.06) * lisencePicW;

//画十字架、合格框、裁剪框
#if 0
    QPainter painter(&pixmap);
    QPen pen(Qt::red);
    pen.setWidth(3);
    painter.setPen(pen);

    //画牌识区多边形
    pen.setColor(Qt::yellow);
    painter.setPen(pen);
    painter.drawPolygon(polygon);

    //画十字架
    int cross_length = 30;
    painter.drawLine(QPointF(posX, posY - cross_length)
                     ,QPointF(posX, posY + cross_length));
    painter.drawLine(QPointF(posX - cross_length, posY)
                     ,QPointF(posX + cross_length, posY));

//                    painter.end();

    /* 画裁剪框
     * 下文中出现如2.88, 2.7, 2.45, 1.06之类的常数，表示裁剪框与车牌图片宽度的比例，
     * 所有的比例均用宽度来衡量。
     * 请参考“161121-裁剪车辆图片示意图”。
     */
    //..若裁剪框超出了，则修正
    if( (leftTopX + cropW) > pixmap.width())
    {
       leftTopX = pixmap.width() - cropW;
    }
    if( (leftTopY + cropH) > pixmap.height())
    {
       leftTopY = pixmap.height() - cropH;
    }

    pen.setColor(Qt::red);
    painter.setPen(pen);
    painter.drawRect(leftTopX, leftTopY, cropW, cropH); //画裁剪框
    painter.end();
#endif

    //裁剪图片
    pixmap = pixmap.copy(leftTopX, leftTopY, cropW, cropH);

    captureResults.vehiclePic = pixmap;

    //发送数据
    emit sendCaptureInfo(captureResults);
}

void Camera::stop()
{
    QMutexLocker locke(&m_runningMutex);
    this->m_stopped = true;
}

void Camera::connectionChanged(bool isConnected)
{
    MY_DEBUG("");
    m_connected = isConnected;
}


//使用 arp 获取局域网内所有在线 IP
QStringList Camera::getAllOnlineIP()
{

    QStringList allOnlineIp;
    QStringList localIpList;

    //获取处于活动状态的网卡 IP
    QList<QHostAddress> hostAddressList;
    QList<QNetworkInterface> networkInterfaceList = QNetworkInterface::allInterfaces();
    foreach(QNetworkInterface netInterface, networkInterfaceList)
    {
        QNetworkInterface::InterfaceFlags flags = netInterface.flags();
        if (flags.testFlag(QNetworkInterface::IsRunning)
                && !flags.testFlag(QNetworkInterface::IsLoopBack)) // 网络接口处于活动状态
        {
            QList<QNetworkAddressEntry> entryList = netInterface.addressEntries();
            foreach(QNetworkAddressEntry entry, entryList)
            {
                hostAddressList.append(entry.ip());
            }
        }
    }

    foreach(QHostAddress address, hostAddressList)
    {
        //169.254.0.0-169.254.255.255 这段是私有保留地址
        if(address.toString().contains("127.0.0.1") ||
           address.toString().startsWith("169.254.") ||
           address.toString().startsWith("192.0.0."))
        {
            continue;
        }

        if(address.protocol() == QAbstractSocket::IPv4Protocol)
        {
            localIpList.append(address.toString());
        }
    }

    int port = g_settings.value(HKEY_CAMERA_PORT, "8000").toInt();
    foreach(QString localIp, localIpList)
    {
        QStringList tmpIpList = getSameSectionIp(localIp); //获取同网段所有 IP
        foreach(QString ip, tmpIpList)
        {
            bool portExisted = g_scanPort(ip.toUtf8().data(), port); //扫描端口
            if(portExisted)
            {
                allOnlineIp.append(ip);
            }
        }
    }

    allOnlineIp.append("192.168.1.251");



//    QTimer::singleShot(1, this, SLOT(pingAllIp()));//先 ping 局域网内所有 IP ，这样 arp 表才会更新

//    QString cmd = "arp -a";
//    QProcess process;
//    process.start(cmd);
//    process.waitForFinished();
//    QString text = QString(process.readAllStandardOutput());

//    int port = g_settings.value(HKEY_CAMERA_PORT,"8000").toInt();
//    QStringList allOnlineIp;
//    QStringList ipList = text.split(' ');
//    foreach(QString ip, ipList)
//    {
//        if(!ip.contains(QRegExp("^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$")))
//        {
//            continue;
//        }

//        if(!isValidIp(ip))
//        {
//            continue;
//        }

//        bool portExisted = g_scanPort(ip.toUtf8().data(), port);
//        if(portExisted)
//        {
//            allOnlineIp.append(ip);
//        }
//    }


//    MY_DEBUG("allOnlineIp.count()="<<allOnlineIp.count());
    return allOnlineIp;
}

//获取同网段所有 IP
QStringList Camera::getSameSectionIp(QString ip)
{
//    MY_DEBUG("ip="<<ip);
    if(!ip.contains(QRegExp("^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$")))
    {
        return QStringList();
    }

    int num = ip.split(".").last().toInt();

    ip.remove(QRegExp("[0-9]{1,3}$"));

    QStringList ipList;
    for(int i = 2; i < 255; i++)
    {
        if(i != num)
        {
            ipList.append( ip + QString().setNum(i) );
        }
    }
    return ipList;
}

//和本机（IPV4）网段相同的ip认为是合法的
bool Camera::isValidIp(QString ip)
{
    QStringList localIpList;
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach (QHostAddress address, list)
    {
        if(address.protocol() == QAbstractSocket::IPv4Protocol)
        {
            //IPv4地址
            if (address.toString().contains("127.0."))
            {
                continue;
            }
            localIpList.append(address.toString());
        }
    }

    //不能和本机 IP 地址相同
    if(localIpList.contains(ip))
    {
        return false;
    }

    //删除 IP 地址最后一段
    foreach(QString localIp, localIpList)
    {
        localIp.remove(QRegExp("[0-9]{1,3}$"));
        if(ip.contains(localIp))
        {
            return true;
        }
    }

    return false;
}

//获取在线抓拍机
QStringList Camera::getAllOnlineCamera()
{
//    不能每次都初始化 NET_DVR_Init
    if(!this->m_connected)
    {
        // 初始化
        NET_DVR_Init();
//        MY_DEBUG("");
    }

    //设置连接时间与重连时间
    NET_DVR_SetConnectTime(100, 1);
    NET_DVR_SetReconnect(10000, true);

    QStringList ipList = getAllOnlineIP();

    //逐个 IP 尝试登陆
    QStringList allOnlineCamera;
    foreach(QString ip, ipList)
    {
        QString IP   = ip;
        int     port = g_settings.value(HKEY_CAMERA_PORT, "8000").toInt();
        QString user = g_settings.value(HKEY_CAMERA_USER, "admin").toString();
        QString password = g_settings.value(HKEY_CAMERA_PASSWORD, "zdmq8888").toString();

        //当登录 IP 确实是海康摄像机时，并且登录失败 ，调用 NET_DVR_Login_V30 函数会输出 "8"
        NET_DVR_DEVICEINFO_V30 struDeviceInfo;
        LONG userID = NET_DVR_Login_V30(IP.toUtf8().data()
                                          , port
                                          , user.toUtf8().data()
                                          , password.toUtf8().data()
                                          , &struDeviceInfo);
//        MY_DEBUG("ip="<<ip);
        if(userID >= 0 )
        {
            allOnlineCamera.append(ip);
            NET_DVR_Logout(userID);
        }

    }

    if(!this->m_connected)
    {
        NET_DVR_Cleanup();
    }

    return allOnlineCamera;
}

//ping 局域网内所有 IP 地址
void Camera::pingAllIp()
{
    if(m_process == NULL)
    {
        MY_DEBUG("");
        return;
    }

    if( m_process->state() != QProcess::NotRunning)
    {
        MY_DEBUG("");
        return;
    }
    else if(m_process->state() == QProcess::NotRunning)
    {
        MY_DEBUG("process.errorString()="<<m_process->errorString());
        MY_DEBUG("process.bytesAvailable()="<<m_process->bytesAvailable());
    }

    QString localIp;
    QList<QHostAddress> hostAddressList/* = QNetworkInterface::allAddresses()*/;

    QList<QNetworkInterface> networkInterfaceList = QNetworkInterface::allInterfaces();
    foreach(QNetworkInterface netInterface, networkInterfaceList)
    {
        QNetworkInterface::InterfaceFlags flags = netInterface.flags();
        if (flags.testFlag(QNetworkInterface::IsRunning)
                && !flags.testFlag(QNetworkInterface::IsLoopBack)) // 网络接口处于活动状态
        {
            QList<QNetworkAddressEntry> entryList = netInterface.addressEntries();
            foreach(QNetworkAddressEntry entry, entryList)
            {
                hostAddressList.append(entry.ip());
            }
        }
    }
    foreach(QHostAddress address, hostAddressList)
    {
        //169.254.0.0-169.254.255.255 这段是私有保留地址
        if(address.toString().contains("127.0.0.1") ||
                address.toString().startsWith("169.254.") ||
                address.toString().startsWith("192.0.0."))
        {
            continue;
        }

        if(address.protocol() == QAbstractSocket::IPv4Protocol)
        {
            localIp = address.toString();
        }
    }

    if(localIp.isEmpty())
    {
        MY_DEBUG("");
        return;
    }

    localIp.remove(QRegExp("[0-9]{1,3}$"));

    QStringList args;
    QString cmd = QString("for /L %G in (1 1 255) do start /b ping -n 1 %1%G")
                        .arg(localIp);
    MY_DEBUG("cmd="<<cmd);
    args <<"/c" << cmd;

    m_process->start("cmd", args);
//    MY_DEBUG("");
//    process.waitForFinished();
//    MY_DEBUG("process.state()="<<process.state());
//    MY_DEBUG("process.errorString()="<<process.errorString());
//    MY_DEBUG("process.bytesAvailable()="<<process.bytesAvailable());
}


