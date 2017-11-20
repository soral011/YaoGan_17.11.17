#include "CaliDialog.h"
#include "ui_CaliDialog.h"
#include "MyDebug.cpp"
#include <QMouseEvent>
#include "SystemStatus.h"

#define  TEXT_STYLE(x) QString("<font size=5 color=#0000ff>%1</font>").arg(x)

CaliDialog::CaliDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CaliDialog)
{
    ui->setupUi(this);//使用ui成员时，先执行这一句

    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
//    MY_DEBUG("");
    ui->verticalWidget_parent->setAutoFillBackground(true);
//    MY_DEBUG("");

    m_socketClient = SocketClient::getInstance();

    m_oldSize = this->size();

    ui->horizontalWidget_debug->hide();
    ui->pushButton_calibrate->setEnabled(false);

    connect(this, SIGNAL(send(QByteArray,QByteArray)),
            m_socketClient, SLOT(send(QByteArray,QByteArray)));

    m_progressDialog.m_isShowingCancelBtn = true;
    connect(&m_progressDialog, SIGNAL(cancelClicked()),
            this, SLOT(cancelCalibration()));

    connect(m_socketClient, SIGNAL(dataPacketReady(SocketPacket)),
            this, SLOT(dataPacketReady(SocketPacket)));

    connect(m_socketClient, SIGNAL(hasClient(bool)),
            this, SLOT(testHostStatus(bool)));

    g_connectButtonSignal(this);

    ui->doubleSpinBox_C_CO->setValue(g_settings.value(HKEY_CaliData_CO, 5.24).toDouble());
    ui->doubleSpinBox_C_CO2->setValue(g_settings.value(HKEY_CaliData_CO2, 8.36).toDouble());
    ui->doubleSpinBox_C_HC->setValue(g_settings.value(HKEY_CaliData_HC, 105).toDouble());
    ui->doubleSpinBox_C_NO->setValue(g_settings.value(HKEY_CaliData_NO, 2500).toDouble());

    g_clearPushButtonFocus(this);
}

CaliDialog::~CaliDialog()
{
    MY_DEBUG("");
    delete ui;
    MY_DEBUG("");
}

void CaliDialog::buttonReleased(QWidget *w)
{
    QString buttonName = w->objectName();

    if(buttonName == "pushButton_calibrate")
    {
        if( SocketClient::getInstance()->isConnected() == false )
        {
            g_showTipsDialog(tr("测量主机未连接!"), this);
            return;
        }

        QString errorStr = "";
        SystemStatus *systemStatus = SystemStatus::getInstance();
        if(systemStatus->m_testerStatus.m_tdlasIntensity == -1)
        {
            errorStr = tr("红外 ");
        }
        if(systemStatus->m_testerStatus.m_doasIntensity == -1)
        {
            errorStr += tr("紫外 ");
        }
        if(systemStatus->m_testerStatus.m_opacityIntensity == -1)
        {
            errorStr += tr("绿光 ");
        }
        if(errorStr.count())
        {
            g_showTipsDialog(errorStr + tr("传感器未连接!"), this, QMessageBox::Warning);
            return;
        }

        NetCaliGasConcentration netCaliGasC;
        netCaliGasC.HC_C = ui->doubleSpinBox_C_HC->value();
        netCaliGasC.NO_C = ui->doubleSpinBox_C_NO->value();
        netCaliGasC.CO_C = ui->doubleSpinBox_C_CO->value();
        netCaliGasC.CO2_C = ui->doubleSpinBox_C_CO2->value();
        netCaliGasC.clearGasTime = ui->doubleSpinBox_clear_gas_time->value();
        QByteArray ba((char *)&netCaliGasC, sizeof(NetCaliGasConcentration));
        emit send(ba, NET_DATA_TYPE_CALI_GAS_C);

        m_progressDialog.showMessage(tr("正在标定..."), this, "bottom");

        g_msleep(20);//和上一条命令间隔一会
        emit send(NET_CMD_CALIBRATE_AUTOMATICALLY, NET_DATA_TYPE_CMD);

        g_settings.setValue(HKEY_CaliData_CO, ui->doubleSpinBox_C_CO->value());
        g_settings.setValue(HKEY_CaliData_CO2, ui->doubleSpinBox_C_CO2->value());
        g_settings.setValue(HKEY_CaliData_HC, ui->doubleSpinBox_C_HC->value());
        g_settings.setValue(HKEY_CaliData_NO, ui->doubleSpinBox_C_NO->value());
    }
    else if(buttonName == "pushButton_close")
    {
        g_closeWithAnim(this);
    }
}

void CaliDialog::cancelCalibration()
{
    bool isSure = g_showAskingDialog(tr("确定要退出吗？"), this);
    if(isSure)
    {
//        m_progressBar.hide();
//        ui->label_calibration_status->setText(tr(""));
        stopCalibration();
    }
}

void CaliDialog::dataPacketReady(SocketPacket packet)
{
//    MY_DEBUG("");
//    emit dataPacketReady2(packet);
    if(this->isVisible() == false)
    {
        return;
    }

    if(packet.dataType == NET_DATA_TYPE_TESTER_STATUS)
    {
        NetTesterStatus testerStatus;
        testerStatus.read(packet.data);

        ui->lcdNumber_tdlas->display(testerStatus.m_tdlasIntensity);
        ui->lcdNumber_doas->display(testerStatus.m_doasIntensity);
        ui->lcdNumber_opacity->display(testerStatus.m_opacityIntensity);
//        MY_DEBUG("m_tdlasIntensity="<<testerStatus.m_tdlasIntensity);

        QString text;
        if(testerStatus.m_caliRunSecs == 0)
        {
            int preheatingSecs = /*30 * 60*/1;//预热时间30分钟，秒
            preheatingSecs -= testerStatus.m_startRunSecs;
            if(preheatingSecs > 0)
            {
                int min = preheatingSecs / 60;
                int secs = preheatingSecs % 60;
                text = QString("%1 %2 %3 %4 %5...").arg(tr("测量主机正在预热，请等待"))
                        .arg(TEXT_STYLE( min ))
                        .arg(tr("分"))
                        .arg(TEXT_STYLE( secs ))
                        .arg(tr("秒"));
                ui->pushButton_calibrate->setEnabled(false);
            }
            else
            {
                text =  tr("预热完成，请标定。");
                text = TEXT_STYLE(text);
                ui->pushButton_calibrate->setEnabled(true);
            }
        }
        else
        {
            int min = testerStatus.m_caliRunSecs / 60;
            int secs = testerStatus.m_caliRunSecs % 60;
            text = QString("%1 %2 %3 %4 %5。").arg(tr("标定后已运行"))
                        .arg(TEXT_STYLE( min ))
                        .arg(tr("分"))
                        .arg(TEXT_STYLE( secs ))
                        .arg(tr("秒"));
            ui->pushButton_calibrate->setEnabled(true);
        }
        ui->label_tips->setText(text);
    }
    else if(packet.dataType == NET_DATA_TYPE_CALI_STATUS)
    {

        if(packet.data.count() != sizeof(NetCalibrationStatus))
        {
            MY_DEBUG("packet.data.count() != sizeof(NetCalibrationStatus)");
            return;
        }

        NetCalibrationStatus caliStatus;
        memcpy(&caliStatus, packet.data.data(), sizeof(NetCalibrationStatus));

//        MY_DEBUG("QString(caliStatus.text)="<<QString(caliStatus.text));

        QString msg = tr("正在标定...");
        msg = TEXT_STYLE( msg ) + "<br><br>";

        QString text(caliStatus.text);
//        msg += "<font color='#bebebe'>" + text + "</font>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
        msg += text + "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
        m_progressDialog.showMessage(msg, this, "bottom");

        if(caliStatus.finished == true)
        {
            MY_DEBUG("");
            m_progressDialog.close();
            if(caliStatus.succeeded)
            {
                g_showTipsDialog(tr("标定成功！"), this);
                if(this->parent() == NULL)
                {
                    g_closeWithAnim(this);
                }
            }
            else
            {
                g_showTipsDialog(tr("标定失败！"), this);
            }

            emit calibrationFinished(caliStatus.succeeded);
        }

    }
}

void CaliDialog::paintEvent(QPaintEvent *event)
{
    if(this->parent() == NULL)
    {

        g_drawShadowBorder(this);
    }
    else
    {

    }
}

void CaliDialog::showEvent(QShowEvent * event)
{
    if(this->parent() == NULL) //主界面点击显示
    {
        ui->verticalWidget_parent->setAutoFillBackground(true);
        ui->widget_menu->show();
        this->resize(m_oldSize.width() + 16,
                     m_oldSize.height() + 29 + 16);//29是标题栏高度，16阴影边框宽度
    }
    else//操作引导流程显示
    {
        ui->widget_menu->hide();
        ui->verticalWidget_parent->setAutoFillBackground(false);
    }
    qApp->processEvents();
}

void CaliDialog::mousePressEvent(QMouseEvent *event)
{
    m_relativePos = this->pos()-event->globalPos();
}

void CaliDialog::mouseMoveEvent(QMouseEvent *event)
{
    if(this->parent() == NULL)
    {
        this->move(event->globalPos()+ m_relativePos);
    }
}

void CaliDialog::testHostStatus(bool isConnected)
{
    if(isConnected == false)
    {
        m_progressDialog.close();
        ui->pushButton_calibrate->setEnabled(false);
        QString text = "<font size=5 color=#ff0000>" + tr("测量主机未连接！") + "</font>";
        ui->label_tips->setText(text);
    }
}

void CaliDialog::keyPressEvent(QKeyEvent *event)
{
    MY_DEBUG("event->key()="<<event->key());
    switch (event->key())
    {
    case Qt::Key_Escape:
    case Qt::Key_Enter:
    case Qt::Key_Return:
        event->ignore();
        break;
    default:
        QDialog::keyPressEvent(event);
    }
}


//避免在状态栏点击关闭时并且正在标定时，直接关闭
void CaliDialog::closeEvent(QCloseEvent * event)
{
    if( m_progressDialog.isVisible() )
    {
        this->activateWindow();
        event->ignore();
    }
}

void CaliDialog::stopCalibration()
{
    emit send(NET_CMD_STOP_CALIBRATING, NET_DATA_TYPE_CMD);
    m_progressDialog.close();
}

bool CaliDialog::isCalibrating()
{
    return m_progressDialog.isVisible();
}

void CaliDialog::mouseReleaseEvent(QMouseEvent * event)
{
    bool isOk = g_isClickTimesOK(3);
    if(isOk)
    {
        ui->horizontalWidget_debug->setVisible(
                    !ui->horizontalWidget_debug->isVisible());
    }
}
