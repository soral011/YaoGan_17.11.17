/***********************************************************
**
** class: Revision
**
** 作者：lth
** 日期：2017-07-25
**
** 实时测量时，修正测量结果使得符合误差要求
**
************************************************************/

#ifndef REVISION_H
#define REVISION_H

#include <QDialog>
#include <QTableWidget>
#include <QSettings>
#include "NetDataStructure.h"

#define  GAS_REL_ERROR 0.1 // HC\CO\CO2\NO绝对误差
#define  HC_ABS_ERROR  10   // HC绝对误差10ppm
#define  NO_ABS_ERROR  20
#define  CO_ABS_ERROR  0.25
#define  CO2_ABS_ERROR 0.25

#define  REVISION_CONFIG_FILE QString("config/Revision.ini")
#define  HKEY_HC     "HC"
#define  HKEY_NO     "NO"
#define  HKEY_CO     "CO"
#define  HKEY_CO2    "CO2"

namespace Ui {
class Revision;
}

class Revision : public QDialog
{
    Q_OBJECT

protected:
    explicit Revision(QWidget *parent = 0);
    ~Revision();

public:
    static Revision *getInstance();
    NetExhaustResults reviseR(NetExhaustResults results);
    NetExhaustResults reviseC(NetExhaustResults results);

private:
    Ui::Revision *ui;

    QSettings *m_revisionSettings;

private:
    void  initTableWidget(QTableWidget *tableWidget);
    void  setChecked(QTableWidget *tableWidget, bool value);
    void  setChecked(QTableWidget *tableWidget, bool value, int row);
    bool  isChecked(QWidget *widget);
    bool  isDeviationOk(QTableWidget *tableWidget, float value, float standard);
    float reviseR(QTableWidget *tableWidget, float value);
    float reviseR(float value, float standard);
    float textToFloat(QTableWidgetItem *item);
    void  saveRevisionConfig();
    void  saveRevisionConfig(QTableWidget *tableWidget, QString hKEY);
    void  readRevisionConfig();
    void  readRevisionConfig(QTableWidget *tableWidget, QString hKEY);


private slots:
    void buttonReleased(QWidget *w);
    void checkBoxReleased(QWidget *w);
};

#endif // REVISION_H
