/***********************************************************
**
** class: 无
**
** 作者：lth
** 日期：2016-03-21
**
** Socket发送数据时，使用这个文件定义的数据结构;
** 这个文件中的数据结构是PC端和测量主机端共同使用的，
** 因此其中一边更新后，复制到另一边，同步更新 060704
**
************************************************************/

#ifndef DATASTRUCTURE
#define DATASTRUCTURE
#include <QList>
#include <QDataStream>
#include <QPoint>
#include <QDateTime>

//系统状态信息，包括紫外、红外光谱、绿光光强等
class NetTesterStatus:public QObject
{
public:
    QList<int>    m_opacityADResults; //10路AD实时测量值
    QList<int>    m_tdlasSpectrum;    //
    QList<int>    m_tdlasRawSpectrum; //用于计算产生二次谐波的原始信号
    QList<float>  m_doasSpectrum;     //紫外光谱,使用double后QDataStream无法还原正确
    int           m_workMode;
//    int           m_preheatingRemainingTime; //测量主机预热剩余时间
//    QByteArray    m_data;
    int           m_tdlasIntensity;   //红外光强
    int           m_doasIntensity;    //紫外光强
    int           m_opacityIntensity; //绿光光强
    int           m_startRunSecs;     //启动后运行时间
    int           m_caliRunSecs;      //标定后运行时间
    QString       m_softwareVersion;  //软件版本号
    QString       m_hostIP;           //IP地址
    QString       m_hostName;
    bool          m_isTdlasConnected; //红外模块是否已经连接
    bool          m_isDoasConnected;
    bool          m_isOpacityConnected;

//    QDateTime     m_dateTime;

    NetTesterStatus()
    {
        m_tdlasIntensity = 0;   //红外光强
        m_doasIntensity  = 0;    //紫外光强
        m_opacityIntensity = 0; //绿光光强
        m_startRunSecs = 0;
        m_caliRunSecs  = 0;
        m_isTdlasConnected = false;
        m_isDoasConnected  = false;
        m_isOpacityConnected = false;
    }


    //写数据到类数据成员中
    void write(QByteArray *data)
    {
        QDataStream streamWriter(data, QIODevice::WriteOnly);
        streamWriter.setVersion(QDataStream::Qt_4_3);

        streamWriter << m_opacityADResults;
        streamWriter << m_doasSpectrum;
        streamWriter << m_tdlasSpectrum;
        streamWriter << m_tdlasRawSpectrum;
        streamWriter << m_workMode;
//        streamWriter << m_preheatingRemainingTime;
//        streamWriter << m_data;
        streamWriter << m_tdlasIntensity;   //红外光强
        streamWriter << m_doasIntensity;    //紫外光强
        streamWriter << m_opacityIntensity; //绿光光强
        streamWriter << m_startRunSecs;
        streamWriter << m_caliRunSecs;
        streamWriter << m_softwareVersion;
        streamWriter << m_hostIP;
        streamWriter << m_hostName;
        streamWriter << m_isTdlasConnected;
        streamWriter << m_isDoasConnected;
        streamWriter << m_isOpacityConnected;
    }

    //读数据到类数据成员中
    void read(QByteArray &data)
    {
        QDataStream streamReader(&data, QIODevice::ReadOnly);
        streamReader.setVersion(QDataStream::Qt_4_3);

        streamReader >> m_opacityADResults;
        streamReader >> m_doasSpectrum;
        streamReader >> m_tdlasSpectrum;
        streamReader >> m_tdlasRawSpectrum;
        streamReader >> m_workMode;
//        streamReader >> m_preheatingRemainingTime;
//        streamReader >> m_data;
        streamReader >> m_tdlasIntensity;   //红外光强
        streamReader >> m_doasIntensity;    //紫外光强
        streamReader >> m_opacityIntensity; //绿光光强
        streamReader >> m_startRunSecs;
        streamReader >> m_caliRunSecs;
        streamReader >> m_softwareVersion;
        streamReader >> m_hostIP;
        streamReader >> m_hostName;
        streamReader >> m_isTdlasConnected;
        streamReader >> m_isDoasConnected;
        streamReader >> m_isOpacityConnected;
    }

    //清除类数据成员
    void clear()
    {
        m_opacityADResults.clear();
        m_doasSpectrum.clear();
        m_tdlasSpectrum.clear();
    }

};

//出厂标定数据
class NetFactoryCaliData:public QObject
{
public:
    QByteArray m_caliData;
    QString    m_fileName;

    //写数据到类数据成员中
    void write(QByteArray *data)
    {
        QDataStream streamWriter(data, QIODevice::WriteOnly);
        streamWriter.setVersion(QDataStream::Qt_4_3);

        streamWriter << m_caliData;
        streamWriter << m_fileName;
    }

    //读数据到类数据成员中
    void read(QByteArray &data)
    {
        QDataStream streamReader(&data, QIODevice::ReadOnly);
        streamReader.setVersion(QDataStream::Qt_4_3);

        streamReader >> m_caliData;
        streamReader >> m_fileName;
    }
};

//发送文件数据包
class NetFileData:public QObject
{
public:
    float      m_fileSize;
    QString    m_fileName;
    QByteArray m_data;         //每个包数据大小（字节）
    int        m_amountPacket; //包数量
    int        m_packetNum;    //几个包

    NetFileData()
    {
        m_fileSize = 0;
    }

    //写数据到类数据成员中
    void write(QByteArray *data)
    {
        QDataStream streamWriter(data, QIODevice::WriteOnly);
        streamWriter.setVersion(QDataStream::Qt_4_3);

        streamWriter << m_fileSize;
        streamWriter << m_fileName;
        streamWriter << m_data;
        streamWriter << m_amountPacket;
        streamWriter << m_packetNum;
    }

    //读数据到类数据成员中
    void read(QByteArray &data)
    {
        QDataStream streamReader(&data, QIODevice::ReadOnly);
        streamReader.setVersion(QDataStream::Qt_4_3);

        streamReader >> m_fileSize;
        streamReader >> m_fileName;
        streamReader >> m_data;
        streamReader >> m_amountPacket;
        streamReader >> m_packetNum;
    }
};

//记录紫外、红外、绿光测量、背景光谱
class NetTestSpectrum: public QObject
{
public:
    QList<int>    m_opacityADResults; //10路AD实时测量值
    QList<int>    m_tdlasSpectrum;
    QList<float>  m_doasSpectrum;
    QList<QList<float> >   m_doasSpectrumList;     //紫外测量光谱数据,用于实际道路车辆测量
    QList<QList<int> >     m_tdlasSpectrumList;    //红外测量光谱数据
    QList<QList<int> >     m_opacityADResultsList; //绿光测量10路AD值

    QList<int>    m_opacityADResultsBg; //10路AD背景值
    QList<int>    m_tdlasSpectrumBg;
    QList<float>  m_doasSpectrumBg;
    QList<QList<float> >   m_doasSpectrumListBg;     //紫外背景光谱数据,用于实际道路车辆测量
    QList<QList<int> >     m_tdlasSpectrumListBg;    //红外背景光谱数据
    QList<QList<int> >     m_opacityADResultsListBg; //绿光背景10路AD值

    //写数据到类数据成员中
    void write(QByteArray *data)
    {
        QDataStream streamWriter(data, QIODevice::WriteOnly);
        streamWriter.setVersion(QDataStream::Qt_4_3);

        streamWriter << m_opacityADResults;
        streamWriter << m_doasSpectrum;
        streamWriter << m_tdlasSpectrum;
        streamWriter << m_opacityADResultsBg;
        streamWriter << m_doasSpectrumBg;
        streamWriter << m_tdlasSpectrumBg;

        streamWriter << m_opacityADResultsList;
        streamWriter << m_doasSpectrumList;
        streamWriter << m_tdlasSpectrumList;
        streamWriter << m_opacityADResultsListBg;
        streamWriter << m_doasSpectrumListBg;
        streamWriter << m_tdlasSpectrumListBg;
    }

    //读数据到类数据成员中
    void read(QByteArray &data)
    {
        QDataStream streamReader(&data, QIODevice::ReadOnly);
        streamReader.setVersion(QDataStream::Qt_4_3);

        streamReader >> m_opacityADResults;
        streamReader >> m_doasSpectrum;
        streamReader >> m_tdlasSpectrum;
        streamReader >> m_opacityADResultsBg;
        streamReader >> m_doasSpectrumBg;
        streamReader >> m_tdlasSpectrumBg;

        streamReader >> m_opacityADResultsList;
        streamReader >> m_doasSpectrumList;
        streamReader >> m_tdlasSpectrumList;
        streamReader >> m_opacityADResultsListBg;
        streamReader >> m_doasSpectrumListBg;
        streamReader >> m_tdlasSpectrumListBg;
    }
};

class NetCalibrationData:public QObject
{
public:
    QList<float>  m_F_NO_AC_Args; //NO [出厂] 吸光度浓度3项式曲线系数，A=f(C)=[0] + [1]*C + [2]*C^2 + [3]*C^3
    QList<float>  m_NO_CA_Args;   //修正后浓度吸光度3项式曲线系数，C=f(A)=...

    QList<float>  m_F_HC_AC_Args; //HC [出厂] 吸光度浓度3项式曲线系数，A=f(C)=[0] + [1]*C + [2]*C^2 + [3]*C^3
    QList<float>  m_HC_CA_Args;   //修正后浓度吸光度3项式曲线系数，C=f(A)=...

    QList<float>  m_doasSpectrumCaliBg;   //紫外标定背景光谱
    QList<int>    m_tdlasSpectrumCaliBg;  //红外标定背景光谱

    QList<float>  m_doasSpectrumCaliNO;   //NO标定光谱
    QList<float>  m_doasSpectrumCaliHC;   //HC标定光谱
    QList<int>    m_tdlasSpectrumCali;    //红外标定光谱
    QList<int>    m_opacityADResultsCali; //10路不透光度标定AD值

    /* NO [出厂] 补偿曲线数据点，A_无HC影响 = f( A_有HC影响 ),
     * 测量主机度取的时候放大了10000倍，使用的时候应除以10000
     * 2016-07-04
     */
    QList<QPoint> m_F_NO_AA_Points;

    float m_F_NO_HC_C; //NO [出厂] 补偿3项式曲线对应的HC浓度值
    float m_F_NO_C;    //NO [出厂] 标定浓度值（吸光度浓度曲线中最大浓度值）

    float m_F_HC_Test_K;    //HC [出厂] 当HC测量吸光度比标定吸光度大时，采用 y = 949.14*x + b的算法,x表示吸光度

    float m_HC_Cali_A; //HC标定吸光度
    float m_NO_Cali_A; //NO标定吸光度 add by lth 20160624

    float m_CO_Cali_P; //CO波峰减波谷值
    float m_CO2_Cali_P;//CO2波峰减波谷值

    float m_HC_Cali_C; //HC标定浓度值
    float m_NO_Cali_C; //NO标定浓度值
    float m_CO_Cali_C; //CO标定浓度值
    float m_CO2_Cali_C;//CO2标定浓度值

    float m_HC_Cali_B;

    //采样点波长在截取的光谱中的位置
    int   m_spectrumIndexHC; //229.02nm
    int   m_spectrumIndexNO; //226.21nm

    QDateTime m_dateTime; //标定时间点

    QString m_text;

    //写数据到类数据成员中
    void write(QByteArray *data)
    {
        QDataStream streamWriter(data, QIODevice::WriteOnly);
        streamWriter.setVersion(QDataStream::Qt_4_3);

        streamWriter << m_F_NO_AC_Args;
        streamWriter << m_NO_CA_Args;

        streamWriter << m_F_HC_AC_Args;
        streamWriter << m_HC_CA_Args;

        streamWriter << m_doasSpectrumCaliBg;
        streamWriter << m_tdlasSpectrumCaliBg;

        streamWriter << m_doasSpectrumCaliNO;   //NO标定光谱
        streamWriter << m_doasSpectrumCaliHC;   //HC标定光谱
        streamWriter << m_tdlasSpectrumCali;    //红外标定光谱
        streamWriter << m_opacityADResultsCali; //10路不透光度标定AD值

        streamWriter << m_F_NO_AA_Points;      //NO [出厂] 补偿曲线数据点，A_无HC影响 = f( A_有HC影响 )

        streamWriter << m_F_NO_HC_C; //NO [出厂] 补偿3项式曲线对应的HC浓度值
        streamWriter << m_F_NO_C;    //NO [出厂] 标定浓度值（吸光度浓度曲线中最大浓度值）

        streamWriter << m_F_HC_Test_K;    //NO [出厂]

        streamWriter << m_HC_Cali_A; //HC标定吸光度
        streamWriter << m_NO_Cali_A; //NO标定吸光度 add by lth 20160624

        streamWriter << m_CO_Cali_P; //CO波峰减波谷值
        streamWriter << m_CO2_Cali_P;//CO2波峰减波谷值

        streamWriter << m_HC_Cali_C; //HC标定浓度值
        streamWriter << m_NO_Cali_C; //NO标定浓度值
        streamWriter << m_CO_Cali_C; //CO标定浓度值
        streamWriter << m_CO2_Cali_C;//CO2标定浓度值

        streamWriter << m_HC_Cali_B;

        //采样点波长在截取的光谱中的位置
        streamWriter << m_spectrumIndexHC; //229.02nm
        streamWriter << m_spectrumIndexNO; //226.21nm

        streamWriter << m_dateTime;

        streamWriter << m_text;
    }

    //读数据到类数据成员中
    void read(QByteArray &data)
    {
        QDataStream streamReader(&data, QIODevice::ReadOnly);
        streamReader.setVersion(QDataStream::Qt_4_3);

        streamReader >> m_F_NO_AC_Args;
        streamReader >> m_NO_CA_Args;

        streamReader >> m_F_HC_AC_Args;
        streamReader >> m_HC_CA_Args;

        streamReader >> m_doasSpectrumCaliBg;
        streamReader >> m_tdlasSpectrumCaliBg;

        streamReader >> m_doasSpectrumCaliNO;   //NO标定光谱
        streamReader >> m_doasSpectrumCaliHC;   //HC标定光谱
        streamReader >> m_tdlasSpectrumCali;    //红外标定光谱
        streamReader >> m_opacityADResultsCali; //10路不透光度标定AD值

        streamReader >> m_F_NO_AA_Points;      //NO [出厂] 补偿曲线数据点，A_无HC影响 = f( A_有HC影响 )

        streamReader >> m_F_NO_HC_C; //NO [出厂] 补偿3项式曲线对应的HC浓度值
        streamReader >> m_F_NO_C;    //NO [出厂] 标定浓度值（吸光度浓度曲线中最大浓度值）

        streamReader >> m_F_HC_Test_K;    //NO [出厂]

        streamReader >> m_HC_Cali_A; //HC标定吸光度
        streamReader >> m_NO_Cali_A; //NO标定吸光度 add by lth 20160624

        streamReader >> m_CO_Cali_P; //CO波峰减波谷值
        streamReader >> m_CO2_Cali_P;//CO2波峰减波谷值

        streamReader >> m_HC_Cali_C; //HC标定浓度值
        streamReader >> m_NO_Cali_C; //NO标定浓度值
        streamReader >> m_CO_Cali_C; //CO标定浓度值
        streamReader >> m_CO2_Cali_C;//CO2标定浓度值

        streamReader >> m_HC_Cali_B;

        //采样点波长在截取的光谱中的位置
        streamReader >> m_spectrumIndexHC; //229.02nm
        streamReader >> m_spectrumIndexNO; //226.21nm

        streamReader >> m_dateTime;

        streamReader >> m_text;
    }

};

//设置积分时间等
struct NetAccessConfig
{
    //绿光
    int  opacityAverageNum;//不透光度平均次数

    //紫外
    int  doasAverageNum;
    int  integrationTime;//积分时间,毫秒

    //红外
    int  tdlasAverageNum;
    int  gain;    //增益0-255
    int  current; //电流0-50
    int  phase;   //相位0-100
};

struct NetTdlasSettings
{
    char data[40];
};

//控制气泵阀门等
struct NetIO_Control
{
    bool pumpEnabled;
    bool valveOneEnabled;
    bool valveTwoEnabled;
    bool valveThreeEnabled;
    bool deuteriumLampEnabled; //氘灯开关
};

//发送通讯命令，如TDLAS命令
struct NetCommunicationCmd
{
    int  opacityCmd;
    int  tdlasCmd;
};

//开启、关闭远程协助工具
struct NetRemoteAssistance
{
    bool sshEnabled;
    bool sambaEnabled;
};

struct NetRemoteControl
{
    /* 当PC和ARM定义的结构体没有同步时，怎么判别06.05？
     * 方法如下，可以先结构体的大小structSize是否一致
     * memcpy(&remoteControl,ba.data(),sizeof(int));
     */
    int          structSize;

    NetAccessConfig      accessConfig;
    NetIO_Control        io_control;
    NetCommunicationCmd  commands;
};

struct NetDOASTestResults
{
    float R_HC_Test_A; //HC测量吸光度
    float R_HC_Test_C; //HC测量浓度
    float R_NO_Bg_A;   //NO背景吸光度
    float R_NO_Test_A; //NO测量吸光度
    float R_NO_Compensation_A; //NO补偿吸光度
    float R_NO_Test_C; //NO测量吸光度

    float HC_C; //经过不完全燃烧方程计算浓度
    float NO_C;
};

struct NetTDLASTestResults
{
    float R_CO_Test_P;  //CO测量波峰减波谷值
    float R_CO_Test_C;  //CO测量浓度
//    float R_CO_Bg_A;    //CO背景吸光度

    float R_CO2_Test_P; //CO2测量波峰减波谷值
    float R_CO2_Test_C; //CO2测量浓度
//    float R_CO2_Bg_A;   //CO2背景吸光度

    float CO_C; //经过不完全燃烧方程计算浓度
    float CO2_C;
    float tdlasPeakSpectrum[2300];
};

struct NetSpeedResults
{
    float  L;  //前后轮外边距 cm
    float  V;  //速度  km/h
    float  a;  //加速度 m/s^2
    float T1;  //时间间隔毫秒
    float T2;
    float T3;
};

struct NetOpacityResults
{
    float opacities[10]; //10路不透光度值
    float opacityMax;//最大不透光度
    float opacityAvg;//平均不透光度
    float opacity;   //黑度
    float K;//K值，光吸收系数
};

struct NetExhaustResults
{
    NetDOASTestResults  doasTestResults;  //NO、HC检测结果
    NetTDLASTestResults tdlasTestResults; //NO、HC检测结果
    NetOpacityResults   opacityResults;   //CO、CO2检测结果

    NetSpeedResults     speedResults;    //不透光度检测结果
};

//标定气体浓度
struct NetCaliGasConcentration
{
    float HC_C;
    float NO_C;
    float CO_C;
    float CO2_C;
    int   clearGasTime; //排空气室需要的时间，单位秒
};

//标定完成状态
struct NetCalibrationStatus
{
    bool finished;  //标定完成
    bool succeeded; //标定成败
    int  percent;   //完成进度百分比
    char text[100]; //提示信息
};

//多点标定命令
struct NetMultiCalibration
{
    bool  isCalibratingHC; //标定HC
    bool  isCalibratingNO; //标定NO
    float gasC;  //标定浓度
    float caliA; //吸光度
};

//设置下位机时间，与PC时间保持一致
class NetSyncTime
{
public:
    QDateTime m_time;

    //写数据到类数据成员中
    void write(QByteArray *data)
    {
        QDataStream streamWriter(data, QIODevice::WriteOnly);
        streamWriter.setVersion(QDataStream::Qt_4_3);

        streamWriter << m_time;
    }

    //读数据到类数据成员中
    void read(QByteArray &data)
    {
        QDataStream streamReader(&data, QIODevice::ReadOnly);
        streamReader.setVersion(QDataStream::Qt_4_3);

        streamReader >> m_time;
    }
};

//多点标定数据
class NetMultiCaliData
{
public:
    QString m_F_NO_AC_Args;
    QString m_F_HC_AC_Args;
    QString m_F_HC_TEST_K;
    QString m_F_CO2_TEST_K;
    float   m_HC_Cali_A; //HC标定吸光度
    float   m_NO_Cali_A; //NO标定吸光度
    float   m_CO2_Cali_P;//CO2波峰减波谷值
    float   m_HC_Cali_B;
    float   m_CO2_Cali_B;

    //写数据到类数据成员中
    void write(QByteArray *data)
    {
        QDataStream streamWriter(data, QIODevice::WriteOnly);
        streamWriter.setVersion(QDataStream::Qt_4_3);

        streamWriter << m_F_NO_AC_Args;
        streamWriter << m_F_HC_AC_Args;
        streamWriter << m_F_HC_TEST_K;
        streamWriter << m_F_CO2_TEST_K;
        streamWriter << m_HC_Cali_A; //
        streamWriter << m_NO_Cali_A; //
        streamWriter << m_CO2_Cali_P;//
        streamWriter << m_HC_Cali_B;
        streamWriter << m_CO2_Cali_B;
    }

    //读数据到类数据成员中
    void read(QByteArray &data)
    {
        QDataStream streamReader(&data, QIODevice::ReadOnly);
        streamReader.setVersion(QDataStream::Qt_4_3);

        streamReader >> m_F_NO_AC_Args;
        streamReader >> m_F_HC_AC_Args;
        streamReader >> m_F_HC_TEST_K;
        streamReader >> m_F_CO2_TEST_K;
        streamReader >> m_HC_Cali_A; //
        streamReader >> m_NO_Cali_A; //
        streamReader >> m_CO2_Cali_P;//
        streamReader >> m_HC_Cali_B;
        streamReader >> m_CO2_Cali_B;
    }
};

//使用模拟光谱
struct NetUseSimulationSpectrum
{
    bool usingTdlas;
    bool usingDoas;
    bool usingOpacity;
};

struct NetTesterName
{
    char name[100];
};

#endif // DATASTRUCTURE

