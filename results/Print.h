/***********************************************************
**
** class: Print
**
** 作者：lth
** 日期：2017-07-25
**
** 打印监测结果
**
************************************************************/

#ifndef PRINTFORM_H
#define PRINTFORM_H

#include <QWidget>
#include "Global.h"
#include "Constant.h"
#include <QSqlTableModel>

namespace Ui {
class PrintForm;
}

class Print : public QWidget
{
    Q_OBJECT

public:
    explicit Print(QWidget *parent = 0);
    ~Print();

    void printSummaryMenu(QTableWidget *tableWidget);
    void printSummaryMenu(QSqlTableModel *sqlTableModel);
    void printSingleItem(QList<PropertyAndVariant> testResultList);

private:
    Ui::PrintForm *ui;
    int m_maxHeight,//总目录表格的最佳高度
        m_properRowNum;  //tableWidget_summary_menu最多能够显示多少行
    QList<QPixmap> m_pixmapList;

    void initHorizontalHeadview();
    void printTestResult(QTableWidget *tableWidget);
    QVariant getVariant(QList<PropertyAndVariant> testResultList,
                        QString property);
    void saveAsPDF(QList<QPixmap> pixmapList,
                            QString file_name,
                            QPrinter::Orientation orientation);

private slots:
    void print(QPrinter *printer);
    void print(QPrinter::Orientation orientation);
};

#endif // PRINTFORM_H
