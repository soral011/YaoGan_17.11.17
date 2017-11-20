#include "ExportExcel.h"
#include <QFile>
#include <QDir>
#include <QSqlDatabase>
#include <QDebug>
#include <QMessageBox>
#include <QSqlQuery>
#include <QVariant>
#include <QTime>
#include <QTextCodec>
#include <QHeaderView>

ExportExcel::ExportExcel()
{

}

void ExportExcel::exportExcel(QString fileName,QTableWidget *tableWidget)
{
    // 创建一个数据库实例， 设置连接字符串
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC","excelexport");
    QString dsn = QString("DRIVER={Microsoft Excel Driver (*.xls)};DSN=''; "
                          "FIRSTROWHASNAMES=1; READONLY=FALSE;"
                          "CREATE_DB=\"%1\";DBQ=%2")
                          .arg(fileName)
                          .arg(fileName);
    db.setDatabaseName(dsn);
    // 打开数据库
    if (!db.open())
    {
        qDebug()<< "open false";
        QMessageBox::about(NULL, "r", "open false");
    }



    // 创建表格和字段名称
    //sql = "create table sheet (name TEXT, age NUMBER)";
    QString sql = "create table sheet (";
    int columnCount=tableWidget->columnCount();
    for(int column=0;column<columnCount;column++)
    {
        sql+=tableWidget->horizontalHeaderItem(column)->text();
        if(column!=(columnCount-1))
        {
            sql+=" TEXT, ";
        }
        else
        {
           sql+=" TEXT)";
        }
    }

    QSqlQuery query(db);
    if (!query.exec(sql))
    {
        qDebug()<< "create table false！";
        QMessageBox::about(NULL, "r", "create table false！");
    }

    // 写入数据
    QString sections;
    for(int column=0;column<columnCount;column++)
    {
        sections+=tableWidget->horizontalHeaderItem(column)->text();
        if(column!=(columnCount-1))
        {
            sections+=", ";
        }
    }
    int rowCount=tableWidget->rowCount();
//    qDebug()<<"rowCount="<<rowCount;
    //db.exec( "insert into sheet(中文, age) values('ctb', '28')");
    for(int row=0;row<rowCount;row++)
    {
        QString values;
        for(int column=0;column<columnCount;column++)
        {
            QTableWidgetItem *item=tableWidget->item(row,column);
            if(item!=0)
            {
                values+="'"+item->text()+"'";
            }
            else
            {
                values+="' '";
            }
            if(column!=(columnCount-1))
            {
                values+=", ";
            }
        }
        QString cmd=QString("insert into sheet (%1) values(%2)")
                .arg(sections)
                .arg(values);
        db.exec(cmd);
    }

    // 关闭数据库
    db.close();
}

void ExportExcel::newExcel(const QString &fileName)
{
    pApplication = new QAxObject();
    pApplication->setControl("Excel.Application");//连接Excel控件
    pApplication->dynamicCall("SetVisible(bool)", false);//false不显示窗体
    pApplication->setProperty("DisplayAlerts", false);//不显示任何警告信息。
    pWorkBooks = pApplication->querySubObject("Workbooks");
    QFile file(fileName);
    if (file.exists())
    {
        pWorkBook = pWorkBooks->querySubObject("Open(const QString &)", fileName);
    }
    else
    {
        pWorkBooks->dynamicCall("Add");
        pWorkBook = pApplication->querySubObject("ActiveWorkBook");
    }
    pSheets = pWorkBook->querySubObject("Sheets");
    pSheet = pSheets->querySubObject("Item(int)", 1);
}

//2.增加1个Worksheet
void ExportExcel::appendSheet(const QString &sheetName)
{
//    QAxObject *pLastSheet = pSheets->querySubObject("Item(int)", cnt);
//    pSheets->querySubObject("Add(QVariant)", pLastSheet->asVariant());
//    pSheet = pSheets->querySubObject("Item(int)", cnt);
//    pLastSheet->dynamicCall("Move(QVariant)", pSheet->asVariant());
//    pSheet->setProperty("Name", sheetName);
}

//3.向Excel单元格中写入数据
void ExportExcel::setCellValue(int row, int column, const QString &value)
{

    if(row==0)
    {
        qDebug()<<"value="<<value;
    }

    QAxObject *pRange = pSheet->querySubObject("Cells(int,int)", row, column);
    if(pRange)
    {
        pRange->dynamicCall("Value", value);
    }
}

//4.保存Excel
void ExportExcel::saveExcel(const QString &fileName)
{
    pWorkBook->dynamicCall("SaveAs(const QString &)",
                           QDir::toNativeSeparators(fileName));
}

//5.释放Excel
void ExportExcel::freeExcel()
{
    if (pApplication != NULL)
    {
        pApplication->dynamicCall("Quit()");
        delete pApplication;
        pApplication = NULL;
    }
}
