#ifndef EXPORTEXCEL_H
#define EXPORTEXCEL_H

#include <QObject>
#include <QAxObject>
#include <QTableWidget>

class ExportExcel
{
public:
    ExportExcel();

    void exportExcel(QString fileName,QTableWidget *tableWidget);

private:
    QAxObject *pApplication = NULL;
    QAxObject *pWorkBooks = NULL;
    QAxObject *pWorkBook = NULL;
    QAxObject *pSheets = NULL;
    QAxObject *pSheet = NULL;

    void newExcel(const QString &fileName);
    void appendSheet(const QString &sheetName);
    void setCellValue(int row, int column, const QString &value);
    void saveExcel(const QString &fileName);
    void freeExcel();
};

#endif // EXPORTEXCEL_H
