#include "RealTimeTest.h"
#include "ui_RealTimeTest.h"
#include "NetDataStructure.h"
#include "MyDebug.cpp"
#include "Global.h"
#include <QClipboard>
#include <QDateTime>
#include "Revision.h"

#define  TEXT_STYLE(x) QString("<font size=5 color=#0000ff>%1</font>").arg(x)

RealTimeTest::RealTimeTest(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RealTimeTest)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);

    m_systemStatus = SystemStatus::getInstance();
    m_isSavingBg = false;

    g_connectButtonSignal(this);
    setEnabledWidgets(false);

    ui->groupBox_debug->hide();

    m_progressDialog.m_isShowingCancelBtn = true;
    connect(&m_progressDialog, SIGNAL(cancelClicked()),
            this, SLOT(cancelSavingBgSpectrum()));

    m_saveBgSpectrumTimer.setSingleShot(true);
    connect(&m_saveBgSpectrumTimer, SIGNAL(timeout()),
            this, SLOT(saveBgSpectrum()));

//    ui->checkBox_clear_gas->hide();
    g_clearPushButtonFocus(this);
}

RealTimeTest::~RealTimeTest()
{
    delete ui;
}

void RealTimeTest::hasClient(bool value)
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

    setEnabledWidgets(value);

    QTimer::singleShot(1000, this, SLOT(updateTesterInfo()));
}

void RealTimeTest::updateTesterInfo()
{
    g_updateTesterInfo(ui->label_tester_info);
}

//根据Socket连接与否，使能控件
void RealTimeTest::setEnabledWidgets(bool value)
{
    ui->checkBox_clear_gas->setEnabled(value);
    ui->checkBox_inflate_NO->setEnabled(value);
    ui->checkBox_real_time_test->setEnabled(value);
    ui->checkBox_inflate_HC_CO_CO2->setEnabled(value);
    ui->pushButton_save_bg_spectrum->setEnabled(value);
}

void RealTimeTest::buttonReleased(QWidget *w)
{
    QString buttonName = w->objectName();
    if(buttonName == "pushButton_copy")
    {
        QString str = ui->plainTextEdit->toPlainText();
        QClipboard *board = QApplication::clipboard();
//        if(ui->comboBox_show_opacity_channel->currentIndex() != 0 )
//        {
//            str.remove(" ");
//            str.remove("\t");
//        }
        board->setText(str);
    }
    else if(buttonName == "pushButton_clear_data")
    {
        ui->plainTextEdit->clear();

        ui->label_counter->setText(QString().setNum(0));
    }
    else if(buttonName == "pushButton_show_debug_widget")
    {
        bool isOk = g_isClickTimesOK(3);
        if(isOk)
        {
            ui->groupBox_debug->setVisible(!ui->groupBox_debug->isVisible());
        }
    }
    else if(buttonName == "pushButton_revision")
    {
        Revision *revision = Revision::getInstance();
        revision->show();
        revision->activateWindow();
    }
//    else if(buttonName == "pushButton_multi_cali")
//    {
//        m_multiCaliDialog.show();
//        m_multiCaliDialog.activateWindow();
//    }
    else if(buttonName == "pushButton_close")
    {
        m_progressDialog.close();
        this->cancelSavingBgSpectrum();
        this->close();
        emit send(NET_CMD_QUIT_RT_TEST_MODE, NET_DATA_TYPE_CMD);
        ui->checkBox_inflate_NO->setChecked(false);
        ui->checkBox_inflate_HC_CO_CO2->setChecked(false);
        ui->checkBox_clear_gas->setChecked(false);
        on_checkBox_clear_gas_clicked();
    }
}

void RealTimeTest::dataPacketReady(SocketPacket packet)
{
    if(packet.dataType == NET_DATA_TYPE_RT_TEST_RESULT)
    {
        QByteArray data = packet.data;
        NetExhaustResults exhaustResults;
        memcpy(&exhaustResults, data.data(), sizeof(NetExhaustResults));

        Revision *revision = Revision::getInstance();
        exhaustResults = revision->reviseR(exhaustResults);

        QStringList resultText;
        QString str;
        str = str.setNum((int)exhaustResults.doasTestResults.R_NO_Test_C);
        ui->label_NO->setText(str);
        resultText.append(str);

        str = str.setNum((int)exhaustResults.doasTestResults.R_HC_Test_C);
        ui->label_HC->setText(str);
        resultText.append(str);

        float value = g_setPrecision(exhaustResults.tdlasTestResults.R_CO_Test_C, 2);
        str = str.setNum(value);
        ui->label_CO->setText(str);
        resultText.append(str);

        value = g_setPrecision(exhaustResults.tdlasTestResults.R_CO2_Test_C, 2);
        str = str.setNum(value);
        ui->label_CO2->setText(str);
        resultText.append(str);

        QList<QLabel*> labels;
        labels.append(ui->label_opacity_1);
        labels.append(ui->label_opacity_2);
        labels.append(ui->label_opacity_3);
        labels.append(ui->label_opacity_4);
        labels.append(ui->label_opacity_5);
        labels.append(ui->label_opacity_6);
        labels.append(ui->label_opacity_7);
        labels.append(ui->label_opacity_8);
        labels.append(ui->label_opacity_9);
        labels.append(ui->label_opacity_10);

        for(int i = 0; i < 10; i++)
        {
            double d = g_setPrecision(exhaustResults.opacityResults.opacities[i], 1);
            labels.at(i)->setText(QString().setNum(d));
            resultText.append(QString().setNum(d));
        }

        resultText.append(QString().setNum(
                              exhaustResults.doasTestResults.R_HC_Test_A));
        resultText.append(QString().setNum(
                              exhaustResults.doasTestResults.R_NO_Test_A));
        resultText.append(QString().setNum(
                              exhaustResults.tdlasTestResults.R_CO2_Test_P));
        resultText.append(QString().setNum(
                              exhaustResults.tdlasTestResults.R_CO_Test_P));

        showResultText(resultText);

//        qDebug()<<"R_HC_Test_A ="<<exhaustResults.doasTestResults.R_HC_Test_A
//                <<"\tR_NO_Test_A ="<<exhaustResults.doasTestResults.R_NO_Test_A
//                <<"\tR_CO2_Test_P ="<<exhaustResults.tdlasTestResults.R_CO2_Test_P
//                <<"\tR_CO_Test_P ="<<exhaustResults.tdlasTestResults.R_CO_Test_P;
//        qDebug()<<"";
        if(m_isSavingBg && m_progressDialog.isVisible())
        {
            MY_DEBUG("m_isSavingBg="<<m_isSavingBg);
            m_progressDialog.close();
            m_isSavingBg = false;

            ui->checkBox_clear_gas->setChecked(false);
            on_checkBox_clear_gas_clicked();
        }
    }
}

void RealTimeTest::showResultText(QStringList resultText )
{
    QString text = ui->plainTextEdit->toPlainText();
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss.zzz     ");

    //显示表头NO、HC...
    if(text.isEmpty())
    {
        QStringList strList;

        strList<<"NO"<<"HC"<<"CO"<<"CO2";
        for(int i = 1; i <= 10; i++)
        {
            strList<<QString("Opacity%1").arg(i);
        }
        strList<<"HC_A"<<"NO_A"<<"CO2_P"<<"CO_P";

        strList.insert(0, time);

        foreach(QString str, strList)
        {
            char buffer[100] = {0};
            sprintf(buffer, "%-5s\t", str.toUtf8().data());
            text.append(QString(buffer));
        }

        ui->plainTextEdit->appendPlainText(text);

    }

    //显示表格数据
    text.clear();
    resultText.insert(0, time);
    foreach(QString str, resultText)
    {
        char buffer[100] = {0};
        sprintf(buffer, "%-5s\t", str.toUtf8().data());
        text.append(QString(buffer));
    }
    ui->plainTextEdit->appendPlainText(text);

    text = ui->label_counter->text();
    int count = text.toInt();
    ui->label_counter->setText(QString().setNum(count+1));
}

void RealTimeTest::on_checkBox_real_time_test_clicked(bool checked)
{
    if(m_hasClient == false)
    {
        g_showTipsDialog(tr("网络未连接！"), this);
        return;
    }

    if(m_systemStatus->m_testerStatus.m_caliRunSecs <= 0 && checked)
    {
        g_showTipsDialog(tr("未标定！"), this);
        return;
    }

    if(checked)
    {
        emit send(NET_CMD_SET_RT_TEST_MODE, NET_DATA_TYPE_CMD);
    }
    else
    {
        emit send(NET_CMD_QUIT_RT_TEST_MODE, NET_DATA_TYPE_CMD);
    }
}

void RealTimeTest::on_checkBox_clear_gas_clicked()
{
    if(m_hasClient == false)
    {
        g_showTipsDialog(tr("网络未连接！"), this);
        return;
    }
    controlIO();
}

void RealTimeTest::on_checkBox_inflate_NO_clicked()
{
    if(m_hasClient == false)
    {
        g_showTipsDialog(tr("网络未连接！"), this);
        return;
    }
    ui->checkBox_clear_gas->setChecked(false);
    ui->checkBox_inflate_HC_CO_CO2->setChecked(false);
    controlIO();
}

void RealTimeTest::on_checkBox_inflate_HC_CO_CO2_clicked()
{
    if(m_hasClient == false)
    {
        g_showTipsDialog(tr("网络未连接！"), this);
        return;
    }
    ui->checkBox_clear_gas->setChecked(false);
    ui->checkBox_inflate_NO->setChecked(false);
    controlIO();
}

//控制气泵、阀门
void RealTimeTest::controlIO()
{
    //NO和HC/CO/CO2某一时刻只能通入其中一瓶，若两瓶同时通入，所测量浓度减半 160910

    NetIO_Control netIO_Control;
    if(ui->checkBox_clear_gas->isChecked())
    {
        netIO_Control.pumpEnabled        =  ui->checkBox_clear_gas->isChecked();
        netIO_Control.valveOneEnabled    =  false;
        netIO_Control.valveTwoEnabled    =  false;

        ui->checkBox_inflate_NO->setChecked(false);
        ui->checkBox_inflate_HC_CO_CO2->setChecked(false);
    }
    else
    {
        netIO_Control.pumpEnabled        =  false;
        netIO_Control.valveOneEnabled    =  ui->checkBox_inflate_NO->isChecked();
        netIO_Control.valveTwoEnabled    =  ui->checkBox_inflate_HC_CO_CO2->isChecked();
    }
//        netIO_Control.valveThreeEnabled  =  ui->checkBox_valve3->isChecked();

    QByteArray ba((char *)&netIO_Control, sizeof(NetIO_Control));
    emit send(ba,NET_DATA_TYPE_IO_CONTROL);
}


void RealTimeTest::on_pushButton_save_bg_spectrum_clicked()
{
    if(m_hasClient == false)
    {
//        m_tips3DDialog.showMessage(tr("网络未连接！"), ui->label_network_status);
        g_showTipsDialog(tr("网络未连接！"), this);
        return;
    }

    if(ui->checkBox_real_time_test->isChecked() == false)
    {
        m_tips3DDialog.showMessage(tr("实时测量时，该操作才起作用！")
                                   , ui->checkBox_real_time_test);
        return;
    }

//    bool isOk = g_showAskingDialog(tr("确定已经排空气室？"), this);
//    if(isOk)
//    {
//        emit send(NET_CMD_SAVE_BG_SPECTRUM, NET_DATA_TYPE_CMD);
//    }

    QString text = TEXT_STYLE(tr("保存背景光谱...")) + "<br><br>";
    text += tr("正在排空气室");
    m_progressDialog.showMessage(text, this);
    ui->checkBox_clear_gas->setChecked(true);
    on_checkBox_clear_gas_clicked();
    m_isSavingBg = false;
    m_saveBgSpectrumTimer.start(ui->spinBox_clear_gas_secs->value() * 1000);
}

void RealTimeTest::cancelSavingBgSpectrum()
{
    MY_DEBUG("");
//    m_progressDialog.close();
    m_isSavingBg = true;
    m_saveBgSpectrumTimer.stop();
    QTimer::singleShot(15 * 1000, &m_progressDialog, SLOT(close()));
}

void RealTimeTest::saveBgSpectrum()
{
    MY_DEBUG("");
    m_saveBgSpectrumTimer.stop();
    emit send(NET_CMD_SAVE_BG_SPECTRUM, NET_DATA_TYPE_CMD);
    QString text = TEXT_STYLE(tr("保存背景光谱..."))+ "<br><br>";
    text += tr("正在保存");
    m_progressDialog.showMessage(text, this);
    QTimer::singleShot(3*1000, this, SLOT(cancelSavingBgSpectrum()));
}

void RealTimeTest::closeEvent(QCloseEvent *event)
{
    MY_DEBUG("");
//    emit send(NET_CMD_QUIT_RT_TEST_MODE, NET_DATA_TYPE_CMD);
//    ui->checkBox_inflate_NO->setChecked(false);
//    ui->checkBox_inflate_HC_CO_CO2->setChecked(false);
//    ui->checkBox_clear_gas->setChecked(false);
//    on_checkBox_clear_gas_clicked();
}

void RealTimeTest::showEvent(QShowEvent * event)
{
    if(m_systemStatus->m_testerStatus.m_caliRunSecs > 0 && m_hasClient == true )
    {
        ui->checkBox_real_time_test->setChecked(true);
        emit send(NET_CMD_SET_RT_TEST_MODE, NET_DATA_TYPE_CMD);
    }
}

void RealTimeTest::mousePressEvent(QMouseEvent * event)
{
    qDebug()<<"mousePressEvent !";

    QList<QLabel*> labels;
    labels.append(ui->label_opacity_1);
    labels.append(ui->label_opacity_2);
    labels.append(ui->label_opacity_3);
    labels.append(ui->label_opacity_4);
    labels.append(ui->label_opacity_5);
    labels.append(ui->label_opacity_6);
    labels.append(ui->label_opacity_7);
    labels.append(ui->label_opacity_8);
    labels.append(ui->label_opacity_9);
    labels.append(ui->label_opacity_10);

    foreach(QLabel *label, labels)
    {
        QPoint pos = label->mapToGlobal(QPoint(0,0));
        int x = event->globalX() - pos.x();
        int y = event->globalY() - pos.y();
        QRect vedioWidgetRect = label->rect();

        if(vedioWidgetRect.contains(QPoint(x,y)))
        {
            bool isOk = g_isClickTimesOK(2);
            if(isOk)
            {
                QString  text = label->objectName().split("_").last();
                copyOpacityData("Opacity" +text);

                int minMidth = label->minimumWidth();
                label->setMinimumWidth( minMidth + 8);
                qApp->processEvents();
                g_msleep(100);
                label->setMinimumWidth( minMidth );

                break;
            }
        }
    }
}

void RealTimeTest::copyOpacityData(QString sectionName)
{
    QStringList strList = ui->plainTextEdit->toPlainText().split('\n');
    int column = 0;
    QStringList sectionNameList = strList.first().split("\t");
    for(int i = 0; i < sectionNameList.count(); i++)
    {
        if(sectionName == sectionNameList.at(i))
        {
            column = i;
            break;
        }
    }
    strList.removeFirst();

    QString text(sectionName  + "\n");
    for(int i = 0; i < strList.count(); i++)
    {
        QStringList sectionTextList = strList.at(i).split("\t");
        text.append( sectionTextList.at(column) + "\n" );
    }

    QClipboard *board = QApplication::clipboard();
    board->setText(text);
//    MY_DEBUG(text);
}
