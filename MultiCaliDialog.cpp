#include "MultiCaliDialog.h"
#include "ui_MultiCaliDialog.h"
#include "MyDebug.cpp"
#include "tools/Ployfit.h"
#include "SystemStatus.h"
#include <QClipboard>

MultiCaliDialog::MultiCaliDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MultiCaliDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::WindowCloseButtonHint);

    m_isSavingCaliData = false;

    m_mutiCaliSettings = new QSettings(MUTI_CALI_SETTINGS_FILE, QSettings::IniFormat);

    readSettings(ui->verticalWidget_HC_cali);
    readSettings(ui->verticalWidget_NO_cali);

    hasClient(SystemStatus::getInstance()->m_isTestHostConnected);

    setCsliWidgetEnebled(false);
    connect(&m_caliDialog, SIGNAL(calibrationFinished(bool)),
            this, SLOT(caliFinished(bool)));

    m_socketClient = SocketClient::getInstance();

    connect(m_socketClient, SIGNAL(dataPacketReady(SocketPacket)),
            this, SLOT(dataPacketReady(SocketPacket)));
    connect(m_socketClient, SIGNAL(hasClient(bool)),
            this, SLOT(hasClient(bool)));

    m_progressDialog.m_isShowingCancelBtn = true;
    connect(&m_progressDialog, SIGNAL(cancelClicked()),
            this, SLOT(cancelSaving()));

    g_connectButtonSignal(this);

    m_buttonAndeditMap.insert(ui->pushButton_HC_Cali_l, ui->lineEdit_HC_A_1);
    m_buttonAndeditMap.insert(ui->pushButton_HC_Cali_2, ui->lineEdit_HC_A_2);
    m_buttonAndeditMap.insert(ui->pushButton_HC_Cali_3, ui->lineEdit_HC_A_3);

    m_buttonAndeditMap.insert(ui->pushButton_NO_Cali_l, ui->lineEdit_NO_A_1);
    m_buttonAndeditMap.insert(ui->pushButton_NO_Cali_2, ui->lineEdit_NO_A_2);
    m_buttonAndeditMap.insert(ui->pushButton_NO_Cali_3, ui->lineEdit_NO_A_3);
//    m_buttonAndeditMap.insert(ui->pushButton_NO_Cali_4, ui->lineEdit_NO_A_4);

    m_buttonAndeditMap.insert(ui->pushButton_CO2_Cali_l, ui->lineEdit_CO2_P_1);
    m_buttonAndeditMap.insert(ui->pushButton_CO2_Cali_2, ui->lineEdit_CO2_P_2);

    g_clearPushButtonFocus(this);
}

MultiCaliDialog::~MultiCaliDialog()
{
    delete ui;
}

void MultiCaliDialog::buttonReleased(QWidget *w)
{
    QString buttonName = w->objectName();
//    MY_DEBUG("buttonName="<<buttonName);
    if(buttonName.contains(QRegExp("pushButton_(HC|NO|CO2)_Cali_(l|2|3|4)")))
    {
        if(buttonName.contains("HC"))
        {
            if(ui->doubleSpinBox_HC_C_1->value() >= ui->doubleSpinBox_HC_C_2->value() ||
               ui->doubleSpinBox_HC_C_2->value() >= ui->doubleSpinBox_HC_C_3->value())
            {
                g_showTipsDialog(tr("操作有误，请确保：浓度1 < 浓度2 < 浓度3"), this);
                return;
            }
            if(buttonName.contains("1")){
                QList<float> listX,listY;
                QByteArray data;
                QString str;

                listX.append(0);
                listX.append(ui->doubleSpinBox_HC_C_1->value()); //浓度C
                listX.append(ui->doubleSpinBox_HC_C_2->value());
                listX.append(ui->doubleSpinBox_HC_C_3->value());

                listY.append(0);
                listY.append(ui->lineEdit_HC_A_1->text().toFloat()*100);
                listY.append(ui->lineEdit_HC_A_2->text().toFloat()*100);
                listY.append(ui->lineEdit_HC_A_3->text().toFloat()*100);



                str = "1,"+ QString::number(listX.at(1)*100);
                data.append(str);
                m_socketClient->send(data,NET_DATA_TYPE_SET_THREE_POINT);
            }
            else if(buttonName.contains("2")){
                QList<float> listX,listY;
                QByteArray data;
                QString str;

                listX.append(0);
                listX.append(ui->doubleSpinBox_HC_C_1->value()); //浓度C
                listX.append(ui->doubleSpinBox_HC_C_2->value());
                listX.append(ui->doubleSpinBox_HC_C_3->value());

                listY.append(0);
                listY.append(ui->lineEdit_HC_A_1->text().toFloat()*100);
                listY.append(ui->lineEdit_HC_A_2->text().toFloat()*100);
                listY.append(ui->lineEdit_HC_A_3->text().toFloat()*100);



                str = "1,"+ QString::number(listX.at(2)*100);
                data.append(str);
                m_socketClient->send(data,NET_DATA_TYPE_SET_THREE_POINT);
            }
            else if(buttonName.contains("3")){
                QList<float> listX,listY;
                QByteArray data;
                QString str;

                listX.append(0);
                listX.append(ui->doubleSpinBox_HC_C_1->value()); //浓度C
                listX.append(ui->doubleSpinBox_HC_C_2->value());
                listX.append(ui->doubleSpinBox_HC_C_3->value());

                listY.append(0);
                listY.append(ui->lineEdit_HC_A_1->text().toFloat()*100);
                listY.append(ui->lineEdit_HC_A_2->text().toFloat()*100);
                listY.append(ui->lineEdit_HC_A_3->text().toFloat()*100);



                str = "1,"+ QString::number(listX.at(3)*100);
                data.append(str);
                m_socketClient->send(data,NET_DATA_TYPE_SET_THREE_POINT);
            }
        }
        if(buttonName.contains("NO"))
        {
            if(ui->doubleSpinBox_NO_C_1->value() >= ui->doubleSpinBox_NO_C_2->value() ||
               ui->doubleSpinBox_NO_C_2->value() >= ui->doubleSpinBox_NO_C_3->value())
            {
                g_showTipsDialog(tr("操作有误，请确保：浓度1 < 浓度2 < 浓度3"), this);
                return;
            }
        }
        m_progressDialog.showMessage(tr("正在标定..."), this);
        m_button = (QPushButton *)w;
        m_caliDataList.clear();
        m_isSavingCaliData = true;
    }
    else if(buttonName == "pushButton_HC_Cali_clear")
    {
        ui->lineEdit_HC_A_1->clear();
        ui->lineEdit_HC_A_2->clear();
        ui->lineEdit_HC_A_3->clear();
    }
    else if(buttonName == "pushButton_NO_Cali_clear")
    {
        ui->lineEdit_NO_A_1->clear();
        ui->lineEdit_NO_A_2->clear();
        ui->lineEdit_NO_A_3->clear();
//        ui->lineEdit_NO_A_4->clear();
    }
    else if(buttonName == "pushButton_CO2_Cali_clear")
    {
        ui->lineEdit_CO2_P_1->clear();
        ui->lineEdit_CO2_P_2->clear();
    }
    else if(buttonName == "pushButton_init_cali")
    {
        if(SystemStatus::getInstance()->m_isTestHostConnected)
        {
            this->m_caliDialog.exec();
        }
        else
        {
            g_showTipsDialog(tr("测量主机未连接！"));
        }
    }
    else if(buttonName == "pushButton_copy_plain_text")
    {
        QString str = ui->plainTextEdit->toPlainText();
        QClipboard *board = QApplication::clipboard();
        board->setText(str);
    }
    else if(buttonName == "pushButton_refresh")
    {
        SystemStatus *systemStatus = SystemStatus::getInstance();
        if(systemStatus->m_testerStatus.m_caliRunSecs >0 )
        {
            m_socketClient->send(NET_CMD_SET_RT_TEST_MODE, NET_DATA_TYPE_CMD);
        }
    }
    else if(buttonName == "pushButton_export_factory_cali_data")
    {
        MY_DEBUG("pushButton_export_factory_cali_data");
        m_socketClient->send(NET_CMD_EXPORT_FACTORY_CALI_DATA, NET_DATA_TYPE_CMD);
    }

}

void MultiCaliDialog::dataPacketReady(SocketPacket packet)
{
    if(packet.dataType == NET_DATA_TYPE_RT_TEST_RESULT)
    {
        QByteArray data = packet.data;
        NetExhaustResults exhaustResults;
        memcpy(&exhaustResults, data.data(), sizeof(NetExhaustResults));

        if(m_isSavingCaliData)
        {
            saveCliData(exhaustResults);
        }

        QStringList resultText;
        resultText.append(QString().setNum(
                              exhaustResults.doasTestResults.R_HC_Test_A));
        resultText.append(QString().setNum(
                              exhaustResults.doasTestResults.R_NO_Test_A));
        resultText.append(QString().setNum(
                              exhaustResults.tdlasTestResults.R_CO2_Test_P));
        resultText.append(QString().setNum(
                              exhaustResults.tdlasTestResults.R_CO_Test_P));

        showResultText(resultText);
    }
    else if(packet.dataType == NET_DATA_TYPE_FACTORY_CALI_DATA)
    {
        MY_DEBUG("NET_DATA_TYPE_FACTORY_CALI_DATA");
        NetFactoryCaliData netFactoryCaliData;
        netFactoryCaliData.read(packet.data);

        QString fileName = g_getNowTime("yyMMdd_") + netFactoryCaliData.m_fileName;
        fileName =  QFileDialog::getSaveFileName(this
                                                ,"导出出厂标定数据文件"
                                                ,fileName
                                                ,tr("ini文件 (*.ini)"));
        if(fileName.isEmpty())
        {
            g_showTipsDialog(tr("请选择合适的路径!"));
            return;
        }

        QFile file(fileName);
        if(file.open(QFile::WriteOnly))
        {
            file.write(netFactoryCaliData.m_caliData);
            file.flush();
        }
    }

    SystemStatus *systemStatus = SystemStatus::getInstance();
    if(systemStatus->m_testerStatus.m_caliRunSecs <=0 )
    {
        this->setCsliWidgetEnebled(false);
    }
    else
    {
        this->setCsliWidgetEnebled(true);
    }
}

void MultiCaliDialog::showResultText(QStringList resultText )
{
    QString text = ui->plainTextEdit->toPlainText();
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss.zzz     ");

    //显示表头NO、HC...
    if(text.isEmpty())
    {
        QStringList strList;
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

void MultiCaliDialog::saveCliData(NetExhaustResults exhaustResults)
{
    double value = 0;
    if(m_button->objectName().contains("HC"))
    {
        value = exhaustResults.doasTestResults.R_HC_Test_A;
    }
    else if(m_button->objectName().contains("NO"))
    {
        value = exhaustResults.doasTestResults.R_NO_Test_A;
    }
    else if(m_button->objectName().contains("CO2"))
    {
        value = exhaustResults.tdlasTestResults.R_CO2_Test_P;
    }

    m_caliDataList.append(value);

    if(m_caliDataList.count() > ui->doubleSpinBox_cali_average_num->value())
    {
        double d = 0;
        foreach(double value, m_caliDataList)
        {
            d += value;
        }
        d = d / m_caliDataList.count();
        showCaliData(d);

        m_isSavingCaliData = false;
        m_progressDialog.close();

        g_showTipsDialog(tr("操作成功！"), this);
    }
}

void MultiCaliDialog::showCaliData(double value)
{
    QLineEdit *lineEdit = m_buttonAndeditMap.value(m_button);
    if(lineEdit != NULL)
    {
        lineEdit->setText(QString().setNum(value));
    }
}

void MultiCaliDialog::cancelSaving()
{
    m_progressDialog.close();
    m_isSavingCaliData = false;
}

/*多项式拟合
  poly_n：进行几项拟合
  args：存储拟合系数数组，长度应为poly_n+1
*/
void MultiCaliDialog::calcPloyfit(QList<float> xList,
                                  QList<float> yList,
                                           int poly_n,
                                        double *args)
{
    MY_LOG("start");
    double *x,*y;
    int n;

    n = yList.count();
    x = (double *)calloc(n, sizeof(double));
    y = (double *)calloc(n, sizeof(double));

    for(int i = 0;i<n;i++)
    {
        x[i] = xList.at(i);
        y[i] = yList.at(i);
    }

    polyfit(n,x,y,poly_n,args);

    free(x);
    free(y);
    MY_LOG("end");
}

void MultiCaliDialog::on_pushButton_HC_Cali_finish_clicked()
{
//    if(ui->lineEdit_HC_A_1->text().isEmpty() ||
//       ui->lineEdit_HC_A_2->text().isEmpty() ||
//       ui->lineEdit_HC_A_3->text().isEmpty() )
//    {
//        g_showTipsDialog(tr("请先完成所有标定点！"), this);
//        return;
//    }

//    if(ui->lineEdit_HC_A_1->text().toFloat() >= ui->lineEdit_HC_A_2->text().toFloat() ||
//       ui->lineEdit_HC_A_2->text().toFloat() >= ui->lineEdit_HC_A_3->text().toFloat())
//    {
//        g_showTipsDialog(tr("操作有误，请确保：标定值1 < 标定值2 < 标定值3"), this);
//        return;
//    }

    QList<float> listX,listY;
    QByteArray data;
    QString str;

    listX.append(0);
    listX.append(ui->doubleSpinBox_HC_C_1->value()); //浓度C
    listX.append(ui->doubleSpinBox_HC_C_2->value());
    listX.append(ui->doubleSpinBox_HC_C_3->value());

    listY.append(0);
    listY.append(ui->lineEdit_HC_A_1->text().toFloat()*100);
    listY.append(ui->lineEdit_HC_A_2->text().toFloat()*100);
    listY.append(ui->lineEdit_HC_A_3->text().toFloat()*100);



    str = "1,"+ QString::number(listX.at(1)*100);
    data.append(str);
    m_socketClient->send(data,NET_DATA_TYPE_SET_THREE_POINT);

    g_showTipsDialog(tr("操作成功！"), this);

    saveSettings(ui->verticalWidget_HC_cali);
}

void MultiCaliDialog::on_pushButton_NO_Cali_finish_clicked()
{
    if(ui->lineEdit_NO_A_1->text().isEmpty() ||
       ui->lineEdit_NO_A_2->text().isEmpty() ||
       ui->lineEdit_NO_A_3->text().isEmpty() /*||
       ui->lineEdit_NO_A_4->text().isEmpty()*/)
    {
        g_showTipsDialog(tr("请先完成所有标定点！"), this);
        return;
    }

    if(ui->lineEdit_NO_A_1->text().toFloat() >= ui->lineEdit_NO_A_2->text().toFloat() ||
       ui->lineEdit_NO_A_2->text().toFloat() >= ui->lineEdit_NO_A_3->text().toFloat())
    {
        g_showTipsDialog(tr("操作有误，请确保：标定值1 < 标定值2 < 标定值3"), this);
        return;
    }

    QList<float> listX,listY;

    listX.append(0);
    listX.append(ui->doubleSpinBox_NO_C_1->value()); //浓度C
    listX.append(ui->doubleSpinBox_NO_C_2->value());
    listX.append(ui->doubleSpinBox_NO_C_3->value());
//    listX.append(ui->doubleSpinBox_NO_C_4->value());

    listY.append(0);
    listY.append(ui->lineEdit_NO_A_1->text().toFloat());
    listY.append(ui->lineEdit_NO_A_2->text().toFloat());
    listY.append(ui->lineEdit_NO_A_3->text().toFloat());
//    listY.append(ui->lineEdit_NO_A_4->text().toFloat());

    //排序，从小到大，标定点应该是逐渐增大的
    qSort(listX);
    qSort(listY);

    int ploy_n = 3;
    double args[4],*p;
    p = args;

    calcPloyfit(listX, listY, ploy_n, p);//
    NetMultiCaliData netMultiCaliData;
    netMultiCaliData.m_F_NO_AC_Args = QString("%1,%2,%3,%4")
                                            .arg(args[3])
                                            .arg(args[2])
                                            .arg(args[1])
                                            .arg(args[0]);

    for(int i = 0; i < 4; i++)
    {
        MY_DEBUG(QString("args[%1]=").arg(i)<<args[i]);
    }

    QByteArray data;
    netMultiCaliData.write(&data);

    netMultiCaliData.m_NO_Cali_A = ui->lineEdit_NO_A_2->text().toFloat();

    m_socketClient->send(data, NET_DATA_TYPE_MULTI_CALI);

    g_showTipsDialog(tr("操作成功！"), this);

    saveSettings(ui->verticalWidget_NO_cali);
}

void MultiCaliDialog::on_pushButton_CO2_Cali_finish_clicked()
{
    if(ui->lineEdit_CO2_P_1->text().isEmpty() ||
       ui->lineEdit_CO2_P_1->text().isEmpty())
    {
        g_showTipsDialog(tr("请先完成所有标定点！"), this);
        return;
    }

    float k = (ui->doubleSpinBox_CO2_C_1->value() - ui->doubleSpinBox_CO2_C_2->value()) /
              (ui->lineEdit_CO2_P_1->text().toFloat() - ui->lineEdit_CO2_P_2->text().toFloat());

    MY_DEBUG("k="<<k);

    NetMultiCaliData netMultiCaliData;
    netMultiCaliData.m_F_CO2_TEST_K = QString("%1").arg(k);

    netMultiCaliData.m_CO2_Cali_P = ui->lineEdit_CO2_P_1->text().toFloat();

    netMultiCaliData.m_HC_Cali_B = ui->doubleSpinBox_NO_C_1->value() -
                                        k * netMultiCaliData.m_CO2_Cali_P;

    QByteArray data;
    netMultiCaliData.write(&data);
    m_socketClient->send(data, NET_DATA_TYPE_MULTI_CALI);

    g_showTipsDialog(tr("操作成功！"), this);
}

void MultiCaliDialog::on_pushButton_clear_plain_text_clicked()
{
    ui->plainTextEdit->clear();
    ui->label_counter->setText("0");
}

void MultiCaliDialog::setCsliWidgetEnebled(bool enabled)
{
    QList<QWidget *> widgetList = this->findChildren<QWidget *>();

    foreach(QWidget *widget, widgetList)
    {
        if(widget != ui->pushButton_init_cali &&
           widget != ui->pushButton_export_factory_cali_data &&
           !widget->isAncestorOf(ui->pushButton_init_cali))
        {
            widget->setEnabled(enabled);
        }
    }

//    ui->verticalWidget_NO_cali->setEnabled(enabled);
//    ui->verticalWidget_HC_cali->setEnabled(enabled);
//    ui->verticalWidget_CO2_cali->setEnabled(enabled);
//    ui->plainTextEdit->setEnabled(enabled);
}

void MultiCaliDialog::caliFinished(bool value)
{
    if(value)
    {
        m_socketClient->send(NET_CMD_SET_RT_TEST_MODE, NET_DATA_TYPE_CMD);
    }
    else
    {
        m_socketClient->send(NET_CMD_QUIT_RT_TEST_MODE, NET_DATA_TYPE_CMD);
    }
}

void MultiCaliDialog::hasClient(bool value)
{
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

    setCsliWidgetEnebled(value);
}

void MultiCaliDialog::showEvent(QShowEvent *event)
{
    m_socketClient->send(NET_CMD_SET_RT_TEST_MODE, NET_DATA_TYPE_CMD);
}

void MultiCaliDialog::closeEvent(QCloseEvent *event)
{
    m_socketClient->send(NET_CMD_QUIT_RT_TEST_MODE, NET_DATA_TYPE_CMD);
}

void MultiCaliDialog::saveSettings(QWidget *widget)
{
    if(widget == ui->verticalWidget_NO_cali)
    {
        m_mutiCaliSettings->beginGroup("NO");
        m_mutiCaliSettings->setValue("NO1",ui->doubleSpinBox_NO_C_1->value());
        m_mutiCaliSettings->setValue("NO2",ui->doubleSpinBox_NO_C_2->value());
        m_mutiCaliSettings->setValue("NO3",ui->doubleSpinBox_NO_C_3->value());
    }
    else if(widget == ui->verticalWidget_HC_cali)
    {
        m_mutiCaliSettings->beginGroup("HC");
        m_mutiCaliSettings->setValue("HC1",ui->doubleSpinBox_HC_C_1->value());
        m_mutiCaliSettings->setValue("HC2",ui->doubleSpinBox_HC_C_2->value());
        m_mutiCaliSettings->setValue("HC3",ui->doubleSpinBox_HC_C_3->value());
    }

    m_mutiCaliSettings->endGroup();
}

void MultiCaliDialog::readSettings(QWidget *widget)
{
    if(widget == ui->verticalWidget_NO_cali)
    {
        m_mutiCaliSettings->beginGroup("NO");
        ui->doubleSpinBox_NO_C_1->setValue(m_mutiCaliSettings->value("NO1",
                                        ui->doubleSpinBox_NO_C_1->value()).toDouble());
        ui->doubleSpinBox_NO_C_2->setValue(m_mutiCaliSettings->value("NO2",
                                        ui->doubleSpinBox_NO_C_2->value()).toDouble());
        ui->doubleSpinBox_NO_C_3->setValue(m_mutiCaliSettings->value("NO3",
                                        ui->doubleSpinBox_NO_C_3->value()).toDouble());
    }
    else if(widget == ui->verticalWidget_HC_cali)
    {
        m_mutiCaliSettings->beginGroup("HC");
        ui->doubleSpinBox_HC_C_1->setValue(m_mutiCaliSettings->value("HC1",
                                        ui->doubleSpinBox_HC_C_1->value()).toDouble());
        ui->doubleSpinBox_HC_C_2->setValue(m_mutiCaliSettings->value("HC2",
                                        ui->doubleSpinBox_HC_C_2->value()).toDouble());
        ui->doubleSpinBox_HC_C_3->setValue(m_mutiCaliSettings->value("HC3",
                                        ui->doubleSpinBox_NO_C_3->value()).toDouble());
    }

    m_mutiCaliSettings->endGroup();
}




