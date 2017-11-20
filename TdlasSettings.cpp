#include "TdlasSettings.h"
#include "ui_TdlasSettings.h"
#include "Global.h"
#include "SystemStatus.h"

TdlasSettings::TdlasSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TdlasSettings)
{
    ui->setupUi(this);

    m_isSpinValueChanged = true;

    g_connectButtonSignal(this);

    setSpinBoxEnabled(false);

    SocketClient *client = SocketClient::getInstance();
    connect(client, SIGNAL(dataPacketReady(SocketPacket)),
            this, SLOT(dataPacketReady(SocketPacket)));

    QList<QDoubleSpinBox *> dSpinBoxList = this->findChildren<QDoubleSpinBox *>();
    foreach(QDoubleSpinBox *dSpinBox, dSpinBoxList)
    {
        connect(dSpinBox, SIGNAL(valueChanged(double)),
                this, SLOT(dSpinBoxValueChanged(double)));
    }
    g_clearPushButtonFocus(this);

    connect(client, SIGNAL(hasClient(bool)),
            this, SLOT(hasClient(bool)));
//    void hasClient(bool value)
    connect(&m_updateTimer, SIGNAL(timeout()),
            this, SLOT(updateSettings()));
    m_updateTimer.setSingleShot(true);
}

TdlasSettings::~TdlasSettings()
{
    delete ui;
}

void TdlasSettings::buttonReleased(QWidget *button)
{
    QString buttonName = button->objectName();

    if(buttonName == "pushButton_get_settings")
    {
        addOperationRecord("获取系统配置");
        setSpinBoxEnabled(false);

        NetCommunicationCmd netCommunicationCmd;
        netCommunicationCmd.tdlasCmd  =  4;

        QByteArray ba((char *)&netCommunicationCmd, sizeof(NetCommunicationCmd));

        SocketClient *client = SocketClient::getInstance();
        client->send(ba, NET_DATA_TYPE_COMMUNICATION_CMD);
    }
    else if(buttonName == "pushButton_backup_settings")
    {
        if(m_currentSettings.isEmpty())
        {
            g_showTipsDialog(tr("尚未获取系统配置参数"));
            return;
        }
        addOperationRecord("备份系统配置");
        m_backupSettings = this->m_currentSettings;
    }
    else if(buttonName == "pushButton_recover_settings")
    {
        if(m_backupSettings.isEmpty())
        {
            g_showTipsDialog(tr("尚未备份系统配置参数"));
            return;
        }
        addOperationRecord("恢复系统配置");
        TdlasSettingsData data = this->parseData(m_backupSettings);
        this->showSettings(data);

        SocketClient *client = SocketClient::getInstance();
        client->send(m_backupSettings, NET_DATA_TYPE_TDLAS_SETTINGS);
    }
    else if(buttonName == "pushButton_clear_plainText")
    {
        ui->plainTextEdit->clear();
    }

}

void TdlasSettings::addOperationRecord(QString text)
{
    text = g_getNowTime()+ " " + text;
    ui->plainTextEdit->appendPlainText(text);
}

void TdlasSettings::setSpinBoxEnabled(bool enabled)
{
    QList<QDoubleSpinBox *> dSpinBoxList = this->findChildren<QDoubleSpinBox *>();
    foreach(QDoubleSpinBox *dSpinBox, dSpinBoxList)
    {
        dSpinBox->setEnabled(false);
    }
    qApp->processEvents();
    foreach(QDoubleSpinBox *dSpinBox, dSpinBoxList)
    {
        if(!enabled)
        {
            dSpinBox->clear();
        }

        dSpinBox->setEnabled(enabled);
    }
    qApp->processEvents();
}

void TdlasSettings::dataPacketReady(SocketPacket packet)
{

    if(packet.dataType == NET_DATA_TYPE_TDLAS_SETTINGS)
    {
        MY_DEBUG("packet.dataType="<<packet.dataType);
        MY_DEBUG("packet.data.count="<<packet.data.count());
        NetTdlasSettings tdlasSettings;
        memcpy(&tdlasSettings, packet.data.data(), sizeof(NetTdlasSettings));

        QByteArray settings = packet.data;

        TdlasSettingsData data = this->parseData(settings);
        MY_DEBUG("data.hasData="<<data.hasData);

        if(data.hasData)
        {
            setSpinBoxEnabled(true);
            this->showSettings(data);
        }
    }
}

TdlasSettingsData TdlasSettings::parseData(QByteArray settings)
{
    if(settings.count() < 10)
    {
        TdlasSettingsData data;
        data.hasData = false;
        return data;
    }
    TdlasSettingsData data;
    data.workMode = (uchar)settings.at(0);
    data.temperatre = 0.01 * ( ((uchar)settings.at(1) << 8) | (uchar)settings.at(2) ) ;
    data.scanCurrentStart = (uchar)settings.at(3);
    data.scanCurrentRange = (uchar)settings.at(4);
    data.current = (uchar)settings.at(5);
    data.phase = (settings.at(6) << 8) | settings.at(7);
    data.gain = (uchar)settings.at(8);
    data.averageNum = (uchar)settings.at(9);

    data.hasData = true;

    m_currentSettings = settings;

    MY_DEBUG("settings=" << settings.toHex());
    MY_DEBUG("workMode ="<<data.workMode );
    MY_DEBUG("temperature ="<<data.temperatre );
    MY_DEBUG("scan_current_start="<<data.scanCurrentStart);
    MY_DEBUG("scan_current_range="<<data.scanCurrentRange);
    MY_DEBUG("current="<<data.current);
    MY_DEBUG("gain="<<data.gain);
    MY_DEBUG("phase="<< data.phase );
    MY_DEBUG("averageNum="<< data.averageNum );

    return data;
}

void TdlasSettings::showSettings(TdlasSettingsData data)
{
    m_isSpinValueChanged = false;
    ui->doubleSpinBox_work_mode->setValue(data.workMode);
    ui->doubleSpinBox_temperature->setValue(data.temperatre);
    ui->doubleSpinBox_scan_current_start->setValue(data.scanCurrentStart);
    ui->doubleSpinBox_scan_current_range->setValue(data.scanCurrentRange);
    ui->doubleSpinBox_current->setValue(data.current);
    ui->doubleSpinBox_phase->setValue(data.phase);
    ui->doubleSpinBox_gain->setValue(data.gain);
    ui->doubleSpinBox_average_num->setValue(data.averageNum);

    qApp->processEvents();

    m_isSpinValueChanged = true;

}

void TdlasSettings::dSpinBoxValueChanged(double value)
{
    if(m_isSpinValueChanged == false)
    {
        MY_DEBUG("");
        return;
    }

    m_updateTimer.start(1500);
}

void TdlasSettings::updateSettings()
{
    TdlasSettingsData data;
    data.workMode = ui->doubleSpinBox_work_mode->value();
    data.temperatre = ui->doubleSpinBox_temperature->value() ;
    data.scanCurrentStart = ui->doubleSpinBox_scan_current_start->value();
    data.scanCurrentRange = ui->doubleSpinBox_scan_current_range->value();
    data.current = ui->doubleSpinBox_current->value();
    data.phase = ui->doubleSpinBox_phase->value();
    data.gain = ui->doubleSpinBox_gain->value();
    data.averageNum = ui->doubleSpinBox_average_num->value();

    data.hasData = true;

    QByteArray settings;
    settings.append(data.workMode);
    int temperature = data.temperatre * 100;
    settings.append((unsigned char)(temperature >> 8));//
    settings.append((unsigned char)(temperature));//
    settings.append(data.scanCurrentStart);
    settings.append(data.scanCurrentRange);
    settings.append(data.current);
    settings.append((unsigned char)(data.phase >> 8));//
    settings.append((unsigned char)(data.phase));//
    settings.append(data.gain);
    settings.append(data.averageNum);
    MY_DEBUG("settings=" << settings.toHex());

    m_currentSettings.remove(0, settings.count());
    m_currentSettings.insert(0, settings);

    SocketClient *client = SocketClient::getInstance();
    client->send(m_currentSettings, NET_DATA_TYPE_TDLAS_SETTINGS);
}

void TdlasSettings::hasClient(bool value)
{
    if(!value)
    {
        setSpinBoxEnabled(false);
    }
    QTimer::singleShot(1000, this, SLOT(updateTesterInfo()));
}

void TdlasSettings::updateTesterInfo()
{
    g_updateTesterInfo(ui->label_tester_info);
}

