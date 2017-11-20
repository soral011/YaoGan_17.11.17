/***********************************************************
**
** class: AccessDatabase
**
** 作者：lth
** 日期：2017-07-25
**
** 数据库存取，包括保存测量结果等
**
************************************************************/

#ifndef ACCESSDATABASE_H
#define ACCESSDATABASE_H

#include <QObject>
#include <QtSql>
#include <QTableWidgetItem>
#include <QPrinter>
#include "Constant.h"
#include "Global.h"
#include "MyDebug.cpp"
//#include <math.h>
//#include <cstdlib>
//#include <stdlib.h>

class SaveTestResultThread:public QThread
{
    Q_OBJECT

public:
    QSqlTableModel  *m_sqlTableModel;

private:
    QList<PropertyAndVariant> m_currentTestResultList;
    QList<QPixmap> m_pixmapList;

    QMap <QString, QVariant> m_resultMap;
    QMap <QString, int> m_headData;

    QString m_fileName;
    QMutex  m_mutex;


    void run()
    {
        QMutexLocker locker(&m_mutex);

        if(m_headData.count() == 0)
        {
            getHeaderData();
        }

        if(m_headData.count() <= 0)
        {
            return;
        }
        int rowNum  =  m_sqlTableModel->rowCount();//获得表的行数
        m_sqlTableModel->insertRows(rowNum, 1); //添加一行

        foreach(PropertyAndVariant data, m_currentTestResultList)
        {

            int column = m_headData.value(data.m_property, -1);

            if(column < 0)
            {
                qDebug()  <<  "column = "  <<  column  <<  "data.m_property = "
                           <<  data.m_property;

                continue;
            }
            bool isOk;
            QModelIndex modelIndex = m_sqlTableModel->index(rowNum, column);

            //文本形式存储
            if(data.m_property == PROPERTY_PASS_TIME ||
               data.m_property == PROPERTY_lICENCE ||
               data.m_property == PROPERTY_BG_COLOR ||
               data.m_property == PROPERTY_RECORD_STATUS ||
               data.m_property == PROPERTY_SITE_ID ||
               data.m_property == PROPERTY_USER_ID ||
               data.m_property == PROPERTY_EQUIPMENT_ID )
            {
                QString text = data.m_variant.toString();

                isOk = m_sqlTableModel->setData(modelIndex,text);
                if(isOk == false)
                {
                    MY_DEBUG("isOk="<<isOk);
                }
            }
            else if(/*data.m_property == PROPERTY_DYNAMIC_MODE ||*/
                    data.m_property == PROPERTY_VALIDITY ||
                    data.m_property == PROPERTY_IS_PASS ||
                    data.m_property == PROPERTY_IS_DIESEL )//布尔形式存储
            {
                bool value = data.m_variant.toBool();
                isOk = m_sqlTableModel->setData(modelIndex, value);
                if(isOk == false)
                {
                    MY_DEBUG("isOk="<<isOk);
                }
            }
            else if(data.m_property == PROPERTY_PIC)//OLE对象形式存储
            {
                QPixmap pixmap = data.m_variant.value < QPixmap>();
                if(pixmap.isNull() == true)
                {
                    MY_DEBUG("m_variant.value < QPixmap> is Null");
                    continue;
                }
                QByteArray ByteArray;
                QBuffer    buffer(&ByteArray);
                buffer.open(QIODevice::WriteOnly);
                isOk  =  pixmap.save(&buffer , "jpg", 100);//抓拍机本身上传上来的是jpg
                if(ByteArray.count() <= 0)
                {
                    qDebug()<<"ByteArray.count() <= 0";
                    continue;
                }
                m_sqlTableModel->setData(modelIndex, ByteArray);
            }
            else//double形式存储
            {
                bool isOk;
                double d = data.m_variant.toDouble(&isOk);
                if(isOk == false || qIsInf(d) || qIsNaN(d))
                {
//                    MY_DEBUG("isOk == false ||　fpclassify(d) != FP_NORMAL");
                    d = ERROR_NUMBER * 1.0;
                }
                d = g_setPrecision(d, 2);
//                qDebug()  <<  "d = "  <<  d;
//                qDebug()  <<"data.m_property="<<data.m_property;
                isOk = m_sqlTableModel->setData(modelIndex, d);
                if(isOk == false)
                {
                    MY_DEBUG("m_sqlTableModel->setData(modelIndex, d) == false");
                }
            }
        }

        bool isOk = m_sqlTableModel->submitAll(); //可以直接提交
        if(isOk == false)
        {
            emit accessOperationFailed();
        }
    }

    void getHeaderData()
    {
        int count = m_sqlTableModel->columnCount();
        for(int column = 0; column < count; column++)
        {
            QString name = m_sqlTableModel->headerData(column, Qt::Horizontal)
                                                    .toString();
            m_headData.insert(name, column);
//            qDebug()  <<  "name = "  <<  name;
        }
    }

public:
    void save(QList<PropertyAndVariant> data)
    {
        MY_DEBUG("");
        QMutexLocker locker(&m_mutex);

        m_currentTestResultList = data;

        if(this->isRunning() == false)
        {
            this->start();
        }
    }

signals:
    void accessOperationFailed();
};

class AccessDatabase : public QObject
{
    Q_OBJECT
public:
    explicit AccessDatabase(QObject *parent  =  0);
    ~AccessDatabase();

    QSqlTableModel  *m_sqlTableModel;
    QSqlDatabase     m_dbconn;
    QString          m_dbName;

    SaveTestResultThread m_saveTestResultThread;

    bool createConnection(QString dbName);
    bool saveTestResult(QList<PropertyAndVariant> data);
    bool saveCurrentUser(UserStruct user);
    bool saveCurrentSite(SiteStruct site);
    void switchTable(QString table);
    QString getUserName(QString userID);
    QString getSiteName(QString siteID);

private:
    bool openDataBase();
    QStringList getTablePropertyList(QSqlTableModel  *sqlTableModel);
    int getPropertyPosition(QString property);

signals:

public slots:
    void reopenDatabase();
};

#endif // ACCESSDATABASE_H
