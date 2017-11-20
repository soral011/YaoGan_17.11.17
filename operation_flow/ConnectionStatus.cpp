#include "ConnectionStatus.h"
#include "ui_ConnectionStatus.h"

ConnectionStatus::ConnectionStatus(QDialog *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionStatus)
{
    ui->setupUi(this);

    SocketClient *client = SocketClient::getInstance();
    connect(client, SIGNAL(hasClient(bool)),
            this, SLOT(testHostStatus(bool)));

    Camera *camera = Camera::getInstance();
    connect(camera, SIGNAL(loginOk(bool)),
            this, SLOT(cameraStatus(bool)));

    m_isCameraConnected = camera->m_connected;
    m_isTestHostConnected = client->isConnected();
    testHostStatus(m_isTestHostConnected);
    cameraStatus(m_isCameraConnected);

    g_clearPushButtonFocus(this);
}

ConnectionStatus::~ConnectionStatus()
{
    delete ui;
}

void ConnectionStatus::cameraStatus(bool connected)
{
    QString style;
    if(connected)
    {
        style = "border-image: url(:/images/cross.png);";
    }
    else
    {
        style = "border-image: url(:/images/tick.png);";
    }

    ui->label_camera_connection->setStyleSheet(style);
    this->raise();//否则窗口没有处于最上层时，不会及时更新
    m_isCameraConnected = connected;
    emit deviceConnection(m_isCameraConnected && m_isTestHostConnected);
}

void ConnectionStatus::testHostStatus(bool connected)
{
    QString style;
//    connected=true;
    if(connected)
    {
        style = "border-image: url(:/images/cross.png);";
    }
    else
    {
        style = "border-image: url(:/images/tick.png);";
    }

    ui->label_test_host_connection->setStyleSheet(style);
    this->raise();//否则窗口没有处于最上层时，不会及时更新

    m_isTestHostConnected = connected;

    emit deviceConnection(m_isCameraConnected && m_isTestHostConnected);
}
