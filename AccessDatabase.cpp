#include "AccessDatabase.h"
#include <QMessageBox>


AccessDatabase::AccessDatabase(QObject *parent) : QObject(parent)
{
    MY_LOG("");
    m_sqlTableModel = NULL;

    //addDatabase函数（给每个对象）增加第二个参数，否则会出现如下警告
    //QSqlDatabasePrivate::database: unable to open database
    static int count = 1;
    m_dbconn = QSqlDatabase::addDatabase("QODBC",QString().setNum(count));
    count++;
//    MY_DEBUG("QSqlDatabase::drivers()=" <<QSqlDatabase::drivers());
    connect(&m_saveTestResultThread,SIGNAL(accessOperationFailed()),
            this,SLOT(reopenDatabase()));
}

AccessDatabase::~AccessDatabase()
{
    MY_DEBUG("");
    m_dbconn.close();//关闭数据库，否则产生.ldb文件
}

bool AccessDatabase::createConnection(QString dbName)
{
    MY_LOG("start");
    m_dbName = dbName;
//    dbName="Data-"+dbName+".mdb";
    //打开数据库文件
    if(m_dbconn.isOpen())
    {
        m_dbconn.close();
    }

    QString type = "DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=";
    QString dsn = QString(type + dbName);
    m_dbconn.setDatabaseName(dsn);
//    qDebug()<<"dsn="<<dsn;

    bool isOpen = m_dbconn.open();
//    qDebug()<<"m_dbconn.lastError()="<<m_dbconn.lastError();
    if(isOpen == false)
    {
        QMessageBox::warning(NULL,tr("提示"),tr("打开数据库出错!"),QMessageBox::Ok);
        return false;
    }

    //数据库文件的内容显示到控件
    if(m_sqlTableModel == NULL)
    {
        //确保当m_dbconn打开后再生成m_sqlTableModel变量
        m_sqlTableModel = new QSqlTableModel(0, m_dbconn);
//        m_sqlTableModel->setTable(DB_TABLE_TESTDATA);
        switchTable(DB_TABLE_TESTDATA);
        m_sqlTableModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    }


    MY_LOG("end");
    return true;
}

void AccessDatabase::switchTable(QString table)
{
    QString currentTable = m_sqlTableModel->tableName();
    if(currentTable != table)
    {
        m_sqlTableModel->setTable(table);
    }
    m_sqlTableModel->select();
    while(m_sqlTableModel->canFetchMore())
    {
        m_sqlTableModel->fetchMore();
    }
}

bool AccessDatabase::saveTestResult(QList<PropertyAndVariant> data)
{

    if( openDataBase() )
    {
        switchTable(DB_TABLE_TESTDATA);
        m_saveTestResultThread.m_sqlTableModel = this->m_sqlTableModel;
        m_saveTestResultThread.save(data);
        return true;
    }
    else
    {
        return false;
    }

//    int rowNum = m_sqlTableModel->rowCount();//获得表的行数
//    int id = 10;
//    m_sqlTableModel->insertRows(rowNum,1); //添加一行
//    m_sqlTableModel->setData(m_sqlTableModel->index(rowNum,0),id);

//    bool isOk=m_sqlTableModel->submitAll(); //可以直接提交
//    qDebug()<<"isOk="<<isOk;

}

bool AccessDatabase::saveCurrentUser(UserStruct user)
{
    if( !openDataBase() )
    {
        return false;
    }

    switchTable(DB_TABLE_USER);
    QStringList propertyList = getTablePropertyList(this->m_sqlTableModel);

    //使用 md5 作为主键，防止插入重复（行）数据，当同一用户多次登录时，写入数据库时只会提交成功一次
    QString userID = AddUserDialog::getUserMD5(user);
    int rowNum  =  m_sqlTableModel->rowCount();//获得表的行数
    for(int row = 0; row < rowNum; row++)
    {
        int userIDColumn = getPropertyPosition(PROPERTY_USER_ID);
        QModelIndex modelIndex = m_sqlTableModel->index(row, userIDColumn);
        QString tmpUserID = modelIndex.data().toString();

        if(userID == tmpUserID)
        {
            return false;
        }
    }

    m_sqlTableModel->insertRows(rowNum, 1); //添加一行

    /*
     * 独立变量 ： 行号
     * 关联变量 ： 属性-文本，属性-列号
     * 目的    ： 将文本放到数据库相应的单元格
     */
    QMap <QString, QVariant> propertyTextMap;   //属性-文本
    // propertyList 相当于 “属性-列号”

    propertyTextMap.insert(PROPERTY_USER_ID,   userID);
    propertyTextMap.insert(PROPERTY_USER_NAME, user.m_userName);
    propertyTextMap.insert(PROPERTY_USER_CHINESE_NAME, user.m_userChineseName);
    propertyTextMap.insert(PROPERTY_USER_DESCRIPTION,  user.m_userDiscription);

    for(int column = 0; column < propertyList.count() ;column++)
    {
        QModelIndex modelIndex = m_sqlTableModel->index(rowNum, column);
        QVariant var = propertyTextMap.value(propertyList.at(column));
        m_sqlTableModel->setData(modelIndex, var);
    }
    bool isOk = m_sqlTableModel->submitAll(); //可以直接提交
    MY_DEBUG("isOk="<<isOk);

}

QString AccessDatabase::getUserName(QString userID)
{
    switchTable(DB_TABLE_USER);
    int rowNum  =  m_sqlTableModel->rowCount();//获得表的行数
    for(int row = 0; row < rowNum; row++)
    {
        int userIDColumn = getPropertyPosition(PROPERTY_USER_ID);
        QModelIndex modelIndex = m_sqlTableModel->index(row, userIDColumn);
        QString tmpUserID = modelIndex.data().toString();

        if(userID == tmpUserID)
        {
            int userNameColumn = getPropertyPosition(PROPERTY_USER_CHINESE_NAME);
            QModelIndex modelIndex = m_sqlTableModel->index(row, userNameColumn);
            QString userName = modelIndex.data().toString();
            return userName;
        }
    }
    return "";
}

QString AccessDatabase::getSiteName(QString siteID)
{
    switchTable(DB_TABLE_SITE);
    int rowNum  =  m_sqlTableModel->rowCount();//获得表的行数
    for(int row = 0; row < rowNum; row++)
    {
        int siteIDColumn = getPropertyPosition(PROPERTY_SITE_ID);
        QModelIndex modelIndex = m_sqlTableModel->index(row, siteIDColumn);
        QString tmpSiteID = modelIndex.data().toString();

        if(siteID == tmpSiteID)
        {
            int siteNameColumn = getPropertyPosition(PROPERTY_SITE_NAME);
            QModelIndex modelIndex = m_sqlTableModel->index(row, siteNameColumn);
            QString siteName = modelIndex.data().toString();
            return siteName;
        }
    }
    return "";
}

//根据给定属性获取其在数据库中的列位置
int AccessDatabase::getPropertyPosition(QString property)
{
    int count = m_sqlTableModel->columnCount();
    for(int i = 0;  i< count; i++)
    {
        if(m_sqlTableModel->headerData(i, Qt::Horizontal).toString() == property)
        {
            return i;
        }
    }
    return -1;
}

bool AccessDatabase::saveCurrentSite(SiteStruct site)
{
    if( !openDataBase() )
    {
        return false;
    }

    switchTable(DB_TABLE_SITE);
    QStringList propertyList = getTablePropertyList(this->m_sqlTableModel);

    //使用 md5 作为主键，防止插入重复（行）数据，当同一用户多次登录时，写入数据库时只会提交成功一次
    QString siteID = AddSite::getSiteMD5(site);
    int rowNum  =  m_sqlTableModel->rowCount();//获得表的行数
    for(int row = 0; row < rowNum; row++)
    {
        int siteIDColumn = getPropertyPosition(PROPERTY_SITE_ID);
        QModelIndex modelIndex = m_sqlTableModel->index(row, siteIDColumn);
        QString tmpSiteID = modelIndex.data().toString();
        if(siteID == tmpSiteID)
        {
            return false;
        }
    }

    m_sqlTableModel->insertRows(rowNum, 1); //添加一行


    /*
     * 独立变量 ： 行号
     * 关联变量 ： 属性-文本，属性-列号
     * 目的    ： 将文本放到数据库相应的单元格
     */
    QMap <QString, QVariant> propertyTextMap;   //属性-文本
    // propertyList 相当于 “属性-列号”

    propertyTextMap.insert(PROPERTY_SITE_ID,         siteID);
    propertyTextMap.insert(PROPERTY_SITE_NAME,       site.m_siteName);
    propertyTextMap.insert(PROPERTY_SITE_GRADIENT,   site.m_siteGradiant);
    propertyTextMap.insert(PROPERTY_SITE_LATITUDE,   site.m_siteLatitude);
    propertyTextMap.insert(PROPERTY_SITE_LONGITUDE,  site.m_siteLongitude);

    for(int column = 0; column < propertyList.count() ;column++)
    {
        QModelIndex modelIndex = m_sqlTableModel->index(rowNum, column);
        QVariant var = propertyTextMap.value(propertyList.at(column));
        m_sqlTableModel->setData(modelIndex, var);
    }
    bool isOk = m_sqlTableModel->submitAll(); //可以直接提交
    MY_DEBUG("isOk="<<isOk);
}

bool AccessDatabase::openDataBase()
{
    if(m_dbconn.isOpen() == false)
    {
        QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd");
        QString fileName = TEST_RESULT_FILE_NAME(date);
        QFile file(RESULT_SAVE_PATH + fileName);

        //如果数据库文件不存在则复制模板数据库过来
        if(file.exists() == false)
        {
            QDir dir(TEMPLATE_RESULT_PATH);
            QList<QString> resultNameList = dir.entryList(QDir::Files,QDir::NoSort);
            foreach(QString str, resultNameList)
            {
                if(str.endsWith(".mdb") == true)
                {
                    QFile::copy(TEMPLATE_RESULT_PATH + str,
                                          RESULT_SAVE_PATH + fileName);
                }
            }
        }

//        qDebug()<<"file.exists()="<<file.exists();

        //打开数据库
        bool isCreated = false;
        if(file.exists() == true)
        {
            QDir dir(RESULT_SAVE_PATH + fileName);
            isCreated = createConnection(dir.absolutePath());
        }


        if(isCreated == false)
        {
            QMessageBox::warning(NULL,
                                 tr("提示"),
                                 tr("数据库未打开!"),
                                 QMessageBox::Ok);
            return false;
        }
    }

    return true;
}

//由于数据库操作失败重新打开数据库
void AccessDatabase::reopenDatabase()
{
    bool isOpen = createConnection(m_dbName);
    MY_DEBUG("isOpen="<<isOpen);
}


QStringList AccessDatabase::getTablePropertyList(QSqlTableModel  *sqlTableModel)
{
    QStringList headerNameList;
    int count = sqlTableModel->columnCount();
    for(int column = 0; column < count; column++)
    {
        QString name = sqlTableModel->headerData(column, Qt::Horizontal)
                                                .toString();
        headerNameList.append(name);
//            qDebug()  <<  "name = "  <<  name;
    }
    return headerNameList;
}
