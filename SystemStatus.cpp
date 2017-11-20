#include "SystemStatus.h"
#include "ui_SystemStatus.h"
#include "Global.h"
#include <QDesktopWidget>
#include "Camera.h"
#include "SocketClient.h"
#include "Weather.h"
#include "MyDebug.cpp"

#define  TEXT_STYLE(x) QString("<font color=#ff0000>%1</font>").arg(x)

SystemStatus::SystemStatus(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SystemStatus)
{
    ui->setupUi(this);//切勿执行两次，会在左上角多出一些东西
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    ui->widget_parent->setAutoFillBackground(true);

    QRect rect = qApp->desktop()->availableGeometry();
    move((rect.width() - this->width())/2,
         (rect.height() - this->height())/2);//移到桌面中间


    Camera *camera = Camera::getInstance();
    connect(camera, SIGNAL(loginOk(bool)),
            this, SLOT(cameraStatus(bool)));
    m_isCameraConnected = camera->m_connected; //测量主机连接状态
    cameraStatus(m_isCameraConnected);

    SocketClient *client = SocketClient::getInstance();
    connect(client, SIGNAL(hasClient(bool)),
            this, SLOT(testHostStatus(bool)));
    connect(client, SIGNAL(dataPacketReady(SocketPacket)),
            this, SLOT(dataPacketReady(SocketPacket)));
    m_isTestHostConnected = client->isConnected();
    testHostStatus(m_isTestHostConnected);

    Weather *weather = Weather::getInstance();
    connect(weather, SIGNAL(weatherStatus(bool)),
            this, SLOT(weatherStatus(bool)));
    g_clearPushButtonFocus(this);
}

SystemStatus::~SystemStatus()
{
    delete ui;
}

//使用单例模式
SystemStatus *SystemStatus::getInstance()
{
    static SystemStatus instance; //局部静态变量，若定义为指针，则需要手动释放内容
    return &instance;
}


void SystemStatus::paintEvent(QPaintEvent *event)
{
    g_drawShadowBorder(this);
}

void SystemStatus::on_pushButton_close_clicked()
{
    g_closeWithAnim(this);
}

void SystemStatus::mousePressEvent(QMouseEvent *event)
{
    m_relativePos = this->pos() - event->globalPos();
}

void SystemStatus::mouseMoveEvent(QMouseEvent *event)
{
    this->move(event->globalPos() +  m_relativePos);
}


void SystemStatus::cameraStatus(bool isConnected)
{
    MY_DEBUG("isConnected="<<isConnected);
    m_isCameraConnected = isConnected;
    ui->label_camera_connection->setText( getConnectionText(isConnected) );
}

void SystemStatus::testHostStatus(bool isConnected)
{
    m_isTestHostConnected = isConnected;
    ui->label_testhost_connection->setText( getConnectionText(isConnected) );
}

void SystemStatus::weatherStatus(bool isConnected)
{
    m_isWeatherConnected = isConnected;
    ui->label_weather_connection->setText( getConnectionText(isConnected) );
}

void SystemStatus::dataPacketReady(SocketPacket packet)
{
//    MY_DEBUG("");

    if(packet.dataType == NET_DATA_TYPE_TESTER_STATUS)
    {
        NetTesterStatus testerStatus;
        testerStatus.read(packet.data);

        m_testerStatus.read(packet.data);

        ui->label_tdlas_intensity->setText(
                    QString("%1").arg(testerStatus.m_tdlasIntensity));
        ui->label_doas_intensity->setText(
                    QString("%1").arg(testerStatus.m_doasIntensity));
        ui->label_opacity_intensity->setText(
                    QString("%1").arg(testerStatus.m_opacityIntensity));

        ui->label_software_version->setText(testerStatus.m_softwareVersion);

        int hour= testerStatus.m_startRunSecs / 3600;
        int min = testerStatus.m_startRunSecs / 60 - hour * 60;
        int secs = testerStatus.m_startRunSecs % 60;
        QString text = QString("%1 %2 %3 %4 %5 %6")
                        .arg(hour)
                        .arg(tr("时"))
                        .arg(min)
                        .arg(tr("分"))
                        .arg(secs )
                        .arg(tr("秒"));
        ui->label_start_runtime->setText(text);

//        if(testerStatus.m_caliRunSecs >0)
        {
            int hour= testerStatus.m_caliRunSecs / 3600;
            int min = testerStatus.m_caliRunSecs / 60 - hour * 60;
            int secs = testerStatus.m_caliRunSecs % 60;
            QString text = QString("%1 %2 %3 %4 %5 %6")
                            .arg(hour)
                            .arg(tr("时"))
                            .arg(min)
                            .arg(tr("分"))
                            .arg(secs )
                            .arg(tr("秒"));
            ui->label_cali_runtime->setText(text);
        }

        ui->label_tdlas_connection->setText(
                    getConnectionText(testerStatus.m_isTdlasConnected));
        ui->label_doas_connection->setText(
                    getConnectionText(testerStatus.m_isDoasConnected));
        ui->label_opacity_connection->setText(
                    getConnectionText(testerStatus.m_isOpacityConnected));
    }
}


QString SystemStatus::getConnectionText(bool connected)
{
    if(connected)
    {
        return tr("已连接");
    }
    else
    {
        return TEXT_STYLE( tr("未连接") );
    }
}


