/***********************************************************
**
** class: Results
**
** 作者：lth
** 日期：2017-07-25
**
** 监测结果查看，还包括筛选、打印等
**
************************************************************/

#ifndef TESTRESULTS_H
#define TESTRESULTS_H

#include "Constant.h"
#include <QWidget>
#include "ViewCarPic.h"
#include <QTableWidgetItem>
#include <QMouseEvent>
#include <QPrinter>
#include <QTimer>
#include <QCheckBox>
#include <QDoubleSpinBox>
//#include "ExportExcel.h"
#include "AccessDatabase.h"
#include "Global.h"
#include "tools/Tips3DDialog.h"
#include "Print.h"

#define PIC_SAVE_PATH           "PicSaveAs/"

namespace Ui {
class TestResults;
}

class Results : public QWidget
{
    Q_OBJECT

public:
    explicit Results(QWidget *parent = 0);
    ~Results();

private:
    Ui::TestResults *ui;

    ViewCarPic *m_viewCarPicWidget;

    QTimer      m_showOneRowTimer;

    int m_lastSelectedPosition;
    int m_lastRowPosition;//一次显示一行，上一次显示的位置
    QString m_testResultsPath;
//    QList<PropertyAndText> g_propertyList;
//    QTimer m_showResultTimer;
    QList<QString> m_resultNameList;

    QList<QCheckBox *> m_checkBoxList;

    AccessDatabase  m_accessDatabase;
    QSqlTableModel *m_sqlTableModel;

    Tips3DDialog m_tips3DDialog;

    Print m_printForm;
    QString m_userID;
    QString m_siteID;
    QString m_currentDbFileName;

//functions:
    int  getColumnPosition(QString property);
    bool hasTestResult(QString data);
    void filterDatabase();
    QString createFilter(QCheckBox *checkBox,
                         QComboBox *comboBox,
                         QDoubleSpinBox *dSpinBox,
                         QString property);
    QStringList findTestResultDate();

private slots:
    void buttonReleased(QWidget *w);
    void on_tableWidget_menu_itemClicked(QTableWidgetItem *item);
    void switchItem(bool isDown);
    void mouseMoveEvent(QMouseEvent *event);
    void printTestPage(QPrinter *printer);
    void tableWidgetSectionClicked(int section);
    void openTestResult(QString fileName);
    void showTestResult();
    void showEvent(QShowEvent * event);
    void showCarPicWidgetNormal();
//    void showTestResultRow();
    void checkBoxClicked(QWidget *w );
    void on_comboBox_test_result_date_activated(const QString &arg1);
    void showRows();
//    void keyPressEvent(QKeyEvent * event);
    void on_tableWidget_menu_itemSelectionChanged();
};

#endif // TESTRESULTS_H
