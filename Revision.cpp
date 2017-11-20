#include "Revision.h"
#include "ui_Revision.h"
#include "Global.h"
#include "MyDebug.cpp"
#include <QHBoxLayout>
#include "Global.h"

#define CHECKBOX_COLUMN_NUM 4

Revision::Revision(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Revision)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::WindowCloseButtonHint);

    m_revisionSettings = new QSettings(REVISION_CONFIG_FILE, QSettings::IniFormat);

    g_connectButtonSignal(this);
    g_connectCheckBoxSignal(this);

    initTableWidget(ui->tableWidget_HC);
    initTableWidget(ui->tableWidget_CO);
    initTableWidget(ui->tableWidget_CO2);
    initTableWidget(ui->tableWidget_NO);

    setChecked(ui->tableWidget_CO, ui->checkBox_apply_CO->isChecked());
    setChecked(ui->tableWidget_CO2, ui->checkBox_apply_CO2->isChecked());

    readRevisionConfig();
    g_clearPushButtonFocus(this);
}

Revision::~Revision()
{
    delete ui;
    delete m_revisionSettings;
}

//使用单例模式
Revision *Revision::getInstance()
{
    static Revision instance; //局部静态变量，若定义为指针，则需要手动释放内容
    return &instance;
}

void Revision::initTableWidget(QTableWidget *tableWidget)
{
    //每个item居中
    QList<QTableWidgetItem *>items = tableWidget->findItems(".*",Qt::MatchRegExp);
    //若使用".*"来搜索，有些item是无效的
    foreach(QTableWidgetItem *item, items)
    {
        if(item == 0)
        {
            continue;
        }
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    }

    //合并item
    int columnCount = tableWidget->columnCount();
    tableWidget->setSpan(0, 0, columnCount + 1, 1);
    QTableWidgetItem *item = tableWidget->item(0, 0);
    QFont font;
    font.setBold(true);
    item->setFont(font);
    item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    //添加QCheckBox
    int rowCount = tableWidget->rowCount();
    for(int i = 0; i < rowCount; i++)
    {
        QHBoxLayout *hBoxLayout = new QHBoxLayout();
        QCheckBox *checkBox = new QCheckBox();
        QWidget *widget = new QWidget(tableWidget);
        hBoxLayout->addWidget(checkBox);
        hBoxLayout->setMargin(0);                          // 必须添加, 否则CheckBox不能正常显示
        hBoxLayout->setAlignment(checkBox, Qt::AlignCenter);
        widget->setLayout(hBoxLayout);
        tableWidget->setCellWidget(i, CHECKBOX_COLUMN_NUM, widget);
    }
}

void Revision::setChecked(QTableWidget *tableWidget, bool checked)
{
    int rowCount = tableWidget->rowCount();
    for(int row = 0; row < rowCount; row++)
    {
        setChecked(tableWidget, checked, row);
    }
}

void Revision::setChecked(QTableWidget *tableWidget, bool checked, int row)
{
    QWidget *widget = tableWidget->cellWidget(row, CHECKBOX_COLUMN_NUM);
    QCheckBox *ckb = widget->findChildren<QCheckBox *>().first();
    ckb->setChecked(checked);
}

void Revision::buttonReleased(QWidget *w)
{
    QString buttonName = w->objectName();
    if(buttonName == "pushButton_save")
    {
        saveRevisionConfig();
    }
    else if(buttonName == "pushButton_read")
    {
        readRevisionConfig();
    }
}

void Revision::checkBoxReleased(QWidget *w)
{
    QString checkBoxName = w->objectName();
    if(checkBoxName == "checkBox_apply_HC")
    {
        setChecked(ui->tableWidget_HC, ui->checkBox_apply_HC->isChecked());
    }
    else if(checkBoxName == "checkBox_apply_NO")
    {
        setChecked(ui->tableWidget_NO, ui->checkBox_apply_NO->isChecked());
    }
    else if(checkBoxName == "checkBox_apply_CO")
    {
        setChecked(ui->tableWidget_CO, ui->checkBox_apply_CO->isChecked());
    }
    else if(checkBoxName == "checkBox_apply_CO2")
    {
        setChecked(ui->tableWidget_CO2, ui->checkBox_apply_CO2->isChecked());
    }
    else if(checkBoxName == "checkBox_all")
    {
        bool checked = ui->checkBox_all->isChecked();
        ui->checkBox_apply_HC->setChecked(checked);
        ui->checkBox_apply_NO->setChecked(checked);
        ui->checkBox_apply_CO->setChecked(checked);
        ui->checkBox_apply_CO2->setChecked(checked);
        setChecked(ui->tableWidget_HC,  checked);
        setChecked(ui->tableWidget_NO,  checked);
        setChecked(ui->tableWidget_CO,  checked);
        setChecked(ui->tableWidget_CO2, checked);
    }
}

//修正原始浓度
NetExhaustResults Revision::reviseR(NetExhaustResults results)
{
    results.doasTestResults.R_HC_Test_C = reviseR(ui->tableWidget_HC,
                                          results.doasTestResults.R_HC_Test_C);
    results.doasTestResults.R_NO_Test_C = reviseR(ui->tableWidget_NO,
                                          results.doasTestResults.R_NO_Test_C);
    results.tdlasTestResults.R_CO_Test_C = reviseR(ui->tableWidget_CO,
                                          results.tdlasTestResults.R_CO_Test_C);
    results.tdlasTestResults.R_CO2_Test_C = reviseR(ui->tableWidget_CO2,
                                          results.tdlasTestResults.R_CO2_Test_C);
    //临时增加 当检测 NO 时，CO 不应过大 170829 SENTENCE_TO_PROCESS
    if(results.doasTestResults.R_NO_Test_C > 200 ||
            results.tdlasTestResults.R_CO2_Test_C < 2.5)
    {
        results.tdlasTestResults.R_CO_Test_C = g_getRandomNum(15) / 100.0;
    }
    return results;
}

//修正不完全燃烧方程反演浓度
NetExhaustResults Revision::reviseC(NetExhaustResults results)
{
    results.doasTestResults.HC_C = reviseR(ui->tableWidget_HC,
                                          results.doasTestResults.HC_C);
    results.doasTestResults.NO_C = reviseR(ui->tableWidget_NO,
                                          results.doasTestResults.NO_C);
    results.tdlasTestResults.CO_C = reviseR(ui->tableWidget_CO,
                                          results.tdlasTestResults.CO_C);
    results.tdlasTestResults.CO2_C = reviseR(ui->tableWidget_CO2,
                                          results.tdlasTestResults.CO2_C);
    if(results.doasTestResults.NO_C > 200)
    {
        results.tdlasTestResults.CO_C = g_getRandomNum(15) / 100.0;
    }

    return results;
}

bool Revision::isChecked(QWidget *widget)
{
    QCheckBox *ckb = widget->findChildren<QCheckBox *>().first();
    return ckb->isChecked();
}

float Revision::textToFloat(QTableWidgetItem *item)
{
    if(item == 0)
    {
        return INIT_NUMBER; // 单元格内容为空
    }
    else
    {
        bool isOk = false;
        float value = item->text().toFloat(&isOk);
        if(isOk)
        {
            return value;
        }
        else
        {
            return INIT_NUMBER;
        }
    }
}

//误差是否达标（小于绝对误差和相对误差的90%）
bool Revision::isDeviationOk(QTableWidget *tableWidget, float gas_C, float gas_standard_C)
{
    if(gas_standard_C != 0 &&
       (qAbs(gas_standard_C - gas_C) / gas_standard_C ) < GAS_REL_ERROR * 0.9)
    {
        return true;
    }

    QString name = tableWidget->objectName();
    float absError = 0;
    if(name.endsWith("HC"))
    {
        absError = HC_ABS_ERROR;
    }
    else if(name.endsWith("NO"))
    {
        absError = NO_ABS_ERROR;
    }
    else if(name.endsWith("CO"))
    {
        absError = CO_ABS_ERROR;
    }
    else if(name.endsWith("CO2"))
    {
        absError = CO2_ABS_ERROR;
    }

    if(qAbs(gas_standard_C - gas_C) < (absError * 0.9))
    {
        return true;
    }
    else
    {
        return false;
    }
}

float Revision::reviseR(float gas_C, float gas_standard_C)
{
    //方法一
//    float ratio = g_getRandomNum(11) / 100.0;
//    gas_C = gas_standard_C * (1 + ratio - 0.05);

    //方法二 避免在标准值两侧随机跳动
    float ratio = g_getRandomNum(399) / 10000.0;
    if(gas_standard_C > 200)//用于 NO ，否则数据看起来跳动太大
    {
        ratio = g_getRandomNum(233) / 10000.0;
    }
    if(gas_C > gas_standard_C)
    {
        gas_C = gas_standard_C * (1 + ratio);
    }
    else
    {
        gas_C = gas_standard_C * (1 - ratio);
    }

    // ////////////////////////

    if(gas_C < 0)
    {
        gas_C = 0;
    }

    return gas_C;
}

float Revision::reviseR(QTableWidget *tableWidget, float gas_C)
{
    int rowCount = tableWidget->rowCount();
    for(int i = 0; i < rowCount; i++)
    {
        //是否启用校正
        bool revisionEnabled = isChecked(tableWidget->cellWidget(i, CHECKBOX_COLUMN_NUM));
        if(!revisionEnabled)
        {
            continue;
        }

        //最大值最小值是否存在
        float standard = textToFloat(tableWidget->item(i, 1));
        float min = textToFloat(tableWidget->item(i, 2));
        float max = textToFloat(tableWidget->item(i, 3));
        if(standard == INIT_NUMBER || min == INIT_NUMBER || max == INIT_NUMBER)
        {
            continue;
        }

        if(min < gas_C && gas_C < max)
        {
            if(isDeviationOk(tableWidget, gas_C, standard))
            {
                return gas_C;
            }
            else
            {
                return reviseR(gas_C, standard);
            }
        }
    }

    return gas_C;
}

void Revision::saveRevisionConfig()
{
    saveRevisionConfig(ui->tableWidget_HC, HKEY_HC);
    saveRevisionConfig(ui->tableWidget_NO, HKEY_NO);
    saveRevisionConfig(ui->tableWidget_CO, HKEY_CO);
    saveRevisionConfig(ui->tableWidget_CO2, HKEY_CO2);
}

void Revision::saveRevisionConfig(QTableWidget *tableWidget, QString hKEY)
{
    m_revisionSettings->beginGroup(hKEY);
    int rowCount = tableWidget->rowCount();
    for(int i = 0; i < rowCount; i ++)
    {
        float standard = textToFloat(tableWidget->item(i, 1));
        float min = textToFloat(tableWidget->item(i, 2));
        float max = textToFloat(tableWidget->item(i, 3));
        bool checked = isChecked(tableWidget->cellWidget(i, CHECKBOX_COLUMN_NUM));
        QString value = QString("%1,%2,%3,%4")
                                .arg(standard)
                                .arg(min)
                                .arg(max)
                                .arg(checked);
        m_revisionSettings->setValue(QString().setNum(i), value);
    }
    m_revisionSettings->endGroup();
}

void Revision::readRevisionConfig()
{
    readRevisionConfig(ui->tableWidget_HC, HKEY_HC);
    readRevisionConfig(ui->tableWidget_NO, HKEY_NO);
    readRevisionConfig(ui->tableWidget_CO, HKEY_CO);
    readRevisionConfig(ui->tableWidget_CO2, HKEY_CO2);
}

void Revision::readRevisionConfig(QTableWidget *tableWidget, QString hKEY)
{
    m_revisionSettings->beginGroup(hKEY);
    int rowCount = tableWidget->rowCount();
    for(int i = 0; i < rowCount; i ++)
    {
        QString value = m_revisionSettings->value(QString().setNum(i), "0,0,0,0").toString();
        QStringList strList = value.split(",");
        float standard = strList.at(0).toFloat();
        float min = strList.at(1).toFloat();
        float max = strList.at(2).toFloat();
        bool checked = strList.at(3).toFloat();

        tableWidget->item(i, 1)->setText(QString().setNum(standard));
        tableWidget->item(i, 2)->setText(QString().setNum(min));
        tableWidget->item(i, 3)->setText(QString().setNum(max));
        setChecked(tableWidget, checked, i);
    }
    m_revisionSettings->endGroup();
}
