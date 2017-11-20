#include "ControlPanel.h"
#include "ui_ControlPanel.h"
#include "NetDataStructure.h"
#include <QMessageBox>
#include "MyDebug.cpp"
#include "Global.h"
#include "SocketClient.h"
#include <windows.h>

ControlPanel::ControlPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlPanel)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    ui->widget_parent->setAutoFillBackground(true);

    m_multiCaliDialog = NULL;

    m_hasClient = false;

    m_originalSize = this->size();

    g_connectButtonSignal(this);

    //发送Socket数据
    connect(&m_calibrationData, SIGNAL(send(QByteArray,QByteArray)),
            this, SIGNAL(send(QByteArray,QByteArray)));

    //接收Socket数据
    connect(this, SIGNAL(dataPacketReady2(SocketPacket)),
            &m_calibrationData, SLOT(dataPacketReady(SocketPacket)));

    SocketClient *client = SocketClient::getInstance();
    connect(client, SIGNAL(serverIpReady(QList<QString>)),
            this, SLOT(testerIpReady(QList<QString>)));

    setOtherToolsVisible();

    this->layout()->setSizeConstraint(QLayout::SetFixedSize); // 窗口根据所有子控件调节尺寸大小

    g_connectCheckBoxSignal(this);
    g_clearPushButtonFocus(this);
}

ControlPanel::~ControlPanel()
{
    delete ui;
}

void ControlPanel::buttonReleased(QWidget *w)
{
    QString buttonName = w->objectName();

    if(!m_hasClient && buttonName != "pushButton_close" &&
            buttonName != "pushButton_show_tools")
    {
        g_showTipsDialog(tr("测量主机未连接！"), this);
//        return;
    }

    if(buttonName == "pushButton_set_integration_time")
    {
        NetAccessConfig netAccessConfig;
        netAccessConfig.doasAverageNum     =  ui->spinBox_doas_average_num->value();
        netAccessConfig.integrationTime    =  ui->spinBox_integration_time->value();

        netAccessConfig.opacityAverageNum  =  ui->spinBox_opacity_average_num->value();
        netAccessConfig.tdlasAverageNum    =  ui->spinBox_tdlas_average_num->value();

//        netAccessConfig.current  =  ui->spinBox_current->value();
//        netAccessConfig.gain     =  ui->spinBox_gain->value();
//        netAccessConfig.phase    =  ui->spinBox_phase->value();

        QByteArray ba((char *)&netAccessConfig, sizeof(NetAccessConfig));
        emit send(ba,NET_DATA_TYPE_INTEGRATION_TIME);
    }
    else if(buttonName == "pushButton_get_integration_time")
    {
        emit send(NET_CMD_GET_INTEGRATION_TIME, NET_DATA_TYPE_CMD);
    }
    else if(buttonName == "pushButton_communication_cmd")
    {
        QString text  =  ui->comboBox_tdlas_cmd->currentText();
        NetCommunicationCmd netCommunicationCmd;
        netCommunicationCmd.tdlasCmd  =  text.left(1).toInt();

        QByteArray ba((char *)&netCommunicationCmd, sizeof(NetCommunicationCmd));
        emit send(ba,NET_DATA_TYPE_COMMUNICATION_CMD);
    }
    else if(buttonName == "pushButton_reboot_tester")
    {
        emit send(NET_CMD_REBOOT_TESTER, NET_DATA_TYPE_CMD);
    }
    else if(buttonName == "pushButton_view_test_spectrum")
    {
        m_testSpectrum.close();
        m_testSpectrum.showMaximized();
    }
    else if(buttonName == "pushButton_view_cali_data")
    {
        emit send(NET_CMD_GET_ALL_CALI_DATA, NET_DATA_TYPE_CMD);
        m_calibrationData.close();
        m_calibrationData.showMaximized();
    }
    else if(buttonName == "pushButton_connect_tester")
    {
        if(ui->comboBox_tester_ip->count())
        {
            SocketClient *client = SocketClient::getInstance();
            client->connectTcpServer(ui->comboBox_tester_ip->currentIndex());
        }
    }
    else if(buttonName == "pushButton_search_tester_ip")
    {
        SocketClient *client = SocketClient::getInstance();
        client->findAllTcpServer();
        ui->comboBox_tester_ip->clear();
    }
    else if(buttonName == "pushButton_read_cali_data")
    {
        emit send(NET_CMD_READ_CALI_DATA, NET_DATA_TYPE_CMD);
    }
    else if(buttonName == "pushButton_tdlas_settings")
    {
        m_tdlasSettings.show();
        m_tdlasSettings.activateWindow();
    }
    else if(buttonName == "pushButton_factory_cali")
    {
        if(m_multiCaliDialog == NULL)
        {
            m_multiCaliDialog = new MultiCaliDialog();
        }
        m_multiCaliDialog->show();
        m_multiCaliDialog->activateWindow();
    }
    else if(buttonName == "pushButton_close")
    {
        g_closeWithAnim(this);
    }
    else if(buttonName == "pushButton_set_tester_name")
    {
        QString text = ui->lineEdit_set_tester_name->text();
        if(text.isEmpty())
        {
            g_showTipsDialog(tr("测量主机名称不能为空。"), this, QMessageBox::Warning);
            return;
        }

        text.remove('-');
        text.remove(QRegExp("[A-Z]"));
        text.remove(QRegExp("[a-z]"));
        text.remove(QRegExp("[0-9]"));
        if(!text.isEmpty())
        {
            g_showTipsDialog(tr("只能包含字母、数字、横杆。"), this);
            return;
        }
        text = ui->lineEdit_set_tester_name->text();
        NetTesterName netTesterName;
        int bufferSize = sizeof(netTesterName.name);//netCalibrationStatus.text 空间只有100字节
        MY_DEBUG("bufferSize="<<bufferSize);
        if(text.count() > bufferSize)
        {
            text = text.mid(0, bufferSize);
        }
        memset(netTesterName.name,'\0',bufferSize);
        memcpy(netTesterName.name, text.toUtf8().data(), text.count());

        QByteArray data((char *)&netTesterName, sizeof(NetTesterName));
        emit send(data, NET_DATA_TYPE_SET_TESTER_NAME);

        g_showTipsDialog(tr("操作成功。"), this, QMessageBox::Warning);
    }
//    else if(buttonName == "pushButton_show_tools")
//    {
//        bool isOk = g_isClickTimesOK(3);
//        if(isOk)
//        {
//            g_isDebugMode = !g_isDebugMode;
//            this->setOtherToolsVisible();
//        }
//    }
}

void ControlPanel::hasClient(bool value)
{
    m_hasClient = value;

    QString text = tr("网络状态：");
    if(value == true)
    {
        text += tr("已连接");
    }
    else
    {
        text += QString("<font color=red>%1</font>").arg(tr("连接中..."));
    }
    text = QString("<font color=blue>%1</font>").arg(text);
    ui->label_network_status->setText(text);

    ui->label_intensity->setText(tr("红外：--  紫外：--  绿光：--"));

    ui->label_tester_name->clear();
    ui->label_tester_ip->clear();
}

void ControlPanel::dataPacketReady(SocketPacket packet)
{
    emit dataPacketReady2(packet);
    if(packet.dataType == NET_DATA_TYPE_INTEGRATION_TIME)
    {
        MY_DEBUG("");

        NetAccessConfig accessConfig;
        memcpy(&accessConfig, packet.data.data(), sizeof(NetAccessConfig));

        MY_DEBUG("doasAverageNum="<<accessConfig.doasAverageNum);
        MY_DEBUG("integrationTime="<<accessConfig.integrationTime);

        ui->spinBox_doas_average_num->setValue(accessConfig.doasAverageNum);
        ui->spinBox_integration_time->setValue(accessConfig.integrationTime);

        ui->spinBox_opacity_average_num->setValue(accessConfig.opacityAverageNum);
        ui->spinBox_tdlas_average_num->setValue(accessConfig.tdlasAverageNum);

//        ui->spinBox_current->setValue(accessConfig.current);
//        ui->spinBox_gain->setValue(accessConfig.gain);
//        ui->spinBox_phase->setValue(accessConfig.phase);
    }
    else if(packet.dataType == NET_DATA_TYPE_TESTER_STATUS)
    {
        NetTesterStatus testerStatus;
        testerStatus.read(packet.data);

        ui->label_tester_ip->setText(testerStatus.m_hostIP);

        QString text = QString("%1  %2  %3")
                     .arg(tr("红外：") + QString().setNum(testerStatus.m_tdlasIntensity))
                     .arg(tr("紫外：") + QString().setNum(testerStatus.m_doasIntensity))
                     .arg(tr("绿光：") + QString().setNum(testerStatus.m_opacityIntensity));
        ui->label_intensity->setText(text);

        ui->label_tester_name->setText(testerStatus.m_hostName);
    }
}

void ControlPanel::testerIpReady(QList<QString> ipList)
{
    ui->comboBox_tester_ip->clear();
    qApp->processEvents();
    ui->comboBox_tester_ip->addItems(ipList);
    qApp->processEvents();
//    ui->comboBox_tester_ip->raise();
    MY_LOG("ipList="<<ipList);
}


void ControlPanel::on_checkBox_pump_clicked(bool checked)
{
    ui->checkBox_valve1->setChecked(false);
    ui->checkBox_valve2->setChecked(false);
    ioControl();
}

void ControlPanel::on_checkBox_valve1_clicked(bool checked)
{
    ui->checkBox_pump->setChecked(false);
    ioControl();
}

void ControlPanel::on_checkBox_valve2_clicked(bool checked)
{
    ui->checkBox_pump->setChecked(false);
    ioControl();
}

void ControlPanel::on_checkBox_start_samba_clicked(bool checked)
{
    NetRemoteAssistance remoteAssistance;
    remoteAssistance.sambaEnabled = checked;
    remoteAssistance.sshEnabled = false;

    QByteArray ba((char *)&remoteAssistance, sizeof(NetRemoteAssistance));
    emit send(ba, NET_DATA_TYPE_REMOTE_ASSISTANCE);
}

void ControlPanel::ioControl()
{
    if(!this->m_hasClient)
    {
        g_showTipsDialog(tr("测量主机未连接！"), this);
    }

    NetIO_Control netIO_Control;
    netIO_Control.pumpEnabled        =  ui->checkBox_pump->isChecked();
    netIO_Control.valveOneEnabled    =  ui->checkBox_valve1->isChecked();
    netIO_Control.valveTwoEnabled    =  ui->checkBox_valve2->isChecked();
    //        netIO_Control.valveThreeEnabled  =  ui->checkBox_valve3->isChecked();

    QByteArray ba((char *)&netIO_Control, sizeof(NetIO_Control));
    emit send(ba,NET_DATA_TYPE_IO_CONTROL);
}


void ControlPanel::setOtherToolsVisible()
{
    ui->groupBox_slave_control->setVisible(g_isDebugMode);
    ui->groupBox_remote_control->setVisible(g_isDebugMode);
    ui->groupBox_local_tools->setVisible(g_isDebugMode);
    ui->groupBox_adjust_avg_num->setVisible(g_isDebugMode);
    ui->groupBox_use_simulation_spectrum->setVisible(g_isDebugMode);
    ui->groupBox_others->setVisible(g_isDebugMode);

//    this->repaint();
//    qApp->processEvents();

//    if(!g_isDebugMode && this->isVisible())
//    {
//        this->close();
//        qApp->processEvents();

//        QTimer::singleShot(1000, this, SLOT(show()));

//    }
}

void ControlPanel::paintEvent(QPaintEvent *event)
{
    g_drawShadowBorder(this);
}

void ControlPanel::mousePressEvent(QMouseEvent *event)
{
    m_relativePos = this->pos() - event->globalPos();
}

void ControlPanel::mouseMoveEvent(QMouseEvent *event)
{
    this->move(event->globalPos() + m_relativePos);
}

void ControlPanel::showEvent(QShowEvent *event)
{
    this->setOtherToolsVisible();
    if(this->size() != this->minimumSize())
    {
        this->resize(this->minimumSize());
        QDesktopWidget* desktop = QApplication::desktop();
        this->move((desktop->width() - this->width())/2,
                     (desktop->height() - this->height())/2); //移动到窗口中间
    }
//    ui->widget_parent->setFocus();
//    this->activateWindow();
}

void ControlPanel::checkBoxReleased(QWidget * w)
{
    QString checkBoxName = w->objectName();

    if(checkBoxName.contains(QRegExp("checkBox_simulation_(tdlas|doas|opacity)")))
    {
        NetUseSimulationSpectrum netUseSimulationSpectrum;
        netUseSimulationSpectrum.usingDoas = ui->checkBox_simulation_doas->isChecked();
        netUseSimulationSpectrum.usingTdlas = ui->checkBox_simulation_tdlas->isChecked();
        netUseSimulationSpectrum.usingOpacity = ui->checkBox_simulation_opacity->isChecked();

        QByteArray ba((char *)&netUseSimulationSpectrum, sizeof(NetUseSimulationSpectrum));
        emit send(ba, NET_DATA_TYPE_USE_SIMU_SPECTRUM);
    }
    else if(checkBoxName == "checkBox_ignore_camera")
    {
        g_ignoringCamera = ui->checkBox_ignore_camera->isChecked();
    }
}
