#include "Global.h"
#include <QSignalMapper>
#include <QPushButton>
#include <QMutex>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QFileDialog>
#include <QDesktopServices>
#include <QEventLoop>
#include <QTimer>
#include <QApplication>
#include <QDesktopWidget>
#include <windows.h>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QCryptographicHash>
#include <QPainter>
#include <math.h>
#include <QScreen>
#include <QWindow>
#include <QCheckBox>

#include <stdio.h>
#include <Winsock2.h>
#include <time.h>
#include "SystemStatus.h"

//#pragma comment(lib,"ws2_32")

MySignal g_mySignal;

//QSettings g_settings("HKEY_CURRENT_USER\\Software\\"+SOFTWARE_NAME
//                     ,QSettings ::NativeFormat);
QSettings g_settings(SETTINGS_FILE_PATH, QSettings::IniFormat);

QSerialPort g_weatherCom;

bool g_isSavingTestResult = true;
bool g_isDebugMode = false;
bool g_ignoringCamera = false;

int g_car_result_index = 0;

QPoint g_PlateRecogRegion_P1 = QPoint(100,360);
QPoint g_PlateRecogRegion_P2 = QPoint(1800,360);
QPoint g_PlateRecogRegion_P3 = QPoint(1800,900);
QPoint g_PlateRecogRegion_P4 = QPoint(100,900);
QPoint g_PlateRecogRegion_Top;
QPoint g_PlateRecogRegion_Right;
QList<PropertyAndText> g_propertyList;

UserStruct g_currentUser;
SiteStruct g_currentSite;

void g_qAppInit()
{
    g_propertyList.append(PropertyAndText("状态",		PROPERTY_RECORD_STATUS));
//    g_propertyList.append(PropertyAndText("状态",		PROPERTY_IS_PASS));
    g_propertyList.append(PropertyAndText("通过时间",	    PROPERTY_PASS_TIME));
    g_propertyList.append(PropertyAndText("车牌",		PROPERTY_lICENCE));
    g_propertyList.append(PropertyAndText("颜色",		PROPERTY_BG_COLOR));
    g_propertyList.append(PropertyAndText("CO浓度",		PROPERTY_C_CO));
    g_propertyList.append(PropertyAndText("CO2浓度",     PROPERTY_C_CO2));
    g_propertyList.append(PropertyAndText("NO浓度",		PROPERTY_C_NO));
    g_propertyList.append(PropertyAndText("HC浓度",		PROPERTY_C_HC));
    g_propertyList.append(PropertyAndText("黑度",		PROPERTY_OPACITY));
//    g_propertyList.append(PropertyAndText("K值",        PROPERTY_OPACITY_PARA));
    g_propertyList.append(PropertyAndText("最大不透光度",	PROPERTY_OPACITY_MAX));
    g_propertyList.append(PropertyAndText("平均不透光度",	PROPERTY_OPACITY_AVG));
    g_propertyList.append(PropertyAndText("VSP",		PROPERTY_VSP));
    g_propertyList.append(PropertyAndText("速度",		PROPERTY_SPEED));
    g_propertyList.append(PropertyAndText("加速度",		PROPERTY_ACC));
    g_propertyList.append(PropertyAndText("车牌置信度",	PROPERTY_RELIABILITY));
    g_propertyList.append(PropertyAndText("风速",		PROPERTY_WIND_SPEED));
    g_propertyList.append(PropertyAndText("风向",		PROPERTY_WIND_DIRECTION));
    g_propertyList.append(PropertyAndText("温度",		PROPERTY_OUT_DOOR_TEMP));
//    g_propertyList.append(PropertyAndText("设备",		PROPERTY_DEVICE_ID));
}

void g_connectButtonSignal(QWidget *w)
{
    QList<QPushButton *> mybuttonlist = w->findChildren<QPushButton *>();
    int count=mybuttonlist.count();
    if(count>0)
    {
        QSignalMapper *signalMapper = NULL;
        signalMapper = new QSignalMapper();

        //经验证，信号void QObject::destroyed(QObject * obj = 0)与
        //槽void QObject::deleteLater()可实现绑定,即使参数不一致
        QObject::connect(w, SIGNAL(destroyed()),
                signalMapper, SLOT(deleteLater()));

        while(count--)
        {
            signalMapper->setMapping(mybuttonlist.at(count), mybuttonlist.at(count));
            QObject::connect(mybuttonlist.at(count), SIGNAL(released()),
                        signalMapper, SLOT(map()));
        }
        QObject::connect(signalMapper, SIGNAL(mapped(QWidget * )),
                w, SLOT(buttonReleased(QWidget * )));
    }
}

void g_connectCheckBoxSignal(QWidget *w)
{
    QList<QCheckBox *> checkBoxlist = w->findChildren<QCheckBox *>();
    int count = checkBoxlist.count();
    if(count>0)
    {
        QSignalMapper *signalMapper = NULL;
        signalMapper = new QSignalMapper();

        //经验证，信号void QObject::destroyed(QObject * obj = 0)与
        //槽void QObject::deleteLater()可实现绑定,即使参数不一致
        QObject::connect(w, SIGNAL(destroyed()),
                signalMapper, SLOT(deleteLater()));

        while(count--)
        {
            signalMapper->setMapping(checkBoxlist.at(count), checkBoxlist.at(count));
            QObject::connect(checkBoxlist.at(count), SIGNAL(released()),
                        signalMapper, SLOT(map()));
        }
        QObject::connect(signalMapper, SIGNAL(mapped(QWidget * )),
                w, SLOT(checkBoxReleased(QWidget * )));
    }
}

void g_log(QString msg)
{
    static QFile logFile;
    static QTextStream in;
    QDateTime nowtime = QDateTime::currentDateTime();

    //QFile(file.fileName()).exists()==false 有时突然删除了日志文件得重新打开
    if(logFile.fileName().isEmpty() ||
            QFile(logFile.fileName()).exists()==false)
    {
        if(QFile(LOG_FILE_PATH).exists()==false)
        {
            QDir().mkpath(LOG_FILE_PATH);
            qDebug()<<"QFile(LOG_FILE_PATH).exists()==false";
        }

        logFile.setFileName(LOG_FILE_PATH+"Log_"
                            + nowtime.toString("yyyy-MM-dd")+"a.txt");
        if(logFile.open(QFile::ReadWrite|QFile::Append))
        {
            qDebug()<<"open"<<logFile.fileName()<<"successful !";
            in.setDevice(&logFile);
        }
        else
        {
            qDebug()<<"open "<<logFile.fileName()<<"failed !";
            return;
        }
    }

    //当文件大于设定值之后，重命名，
    //文件以日期来命名，如2015-05-20a.txt，当2015-05-20a.txt大于设定值之后，
    //其将被命名为2015-05-20b.txt
    qint64 file_size=logFile.size();
    qint64 file_size_limit=1024*1024*1.5;//1.5MB
    if(file_size > file_size_limit)
    {
        QString backup_name = logFile.fileName().remove("a.txt")+"b.txt";

        QFile().remove(backup_name);
        logFile.rename(backup_name);
        logFile.close();
        logFile.setFileName(LOG_FILE_PATH + "Log_"
                            + nowtime.toString("yyyy-MM-dd")+"a.txt");
        if(logFile.open(QFile::ReadWrite|QFile::Append))
        {
            qDebug()<<"2 open"<<logFile.fileName()<<"successful !";
            in.setDevice(&logFile);
        }

        //保留多少天的日志文件,其它删除
        QFileInfo fileInfo;
        QDir dir(LOG_FILE_PATH);
        QStringList allSortedFiles=dir.entryList(QDir::Files,QDir::Time);
        QDateTime deadline_date;
        deadline_date = QDateTime::currentDateTime().addDays(-7*4);//保留7*4天
        quint64 secs_to_deadline = deadline_date.secsTo(QDateTime::currentDateTime());
        foreach(QString fileName,allSortedFiles)
        {
            fileInfo.setFile(dir,fileName);
//                qDebug()<<"secsTo="<<fileInfo.created().secsTo(QDateTime::currentDateTime());
//                qDebug()<<fileInfo.created().toString("yyyy-MM-dd hh:mm:ss");
//                qDebug()<<dir.path()+"/"+fileInfo.fileName();
            if(fileInfo.created().secsTo(QDateTime::currentDateTime()) > secs_to_deadline)
            {
                bool copy_ok=QFile::remove(dir.path()+"/"+fileName);
                if(copy_ok == true)
                {
                    qDebug()<<"QFile::remove = "<<dir.path()+"/"+fileName;
                }
            }
        }
    }
    QString str_time = nowtime.toString("hh:mm:ss");
    msg.remove("/YaoGan");
    in<<str_time<<" "<<msg<<'\r'<<'\n';
    in.flush();

}

QString g_getSaveFileName(QWidget *parent
                      ,QString HKEY
                      ,QString title
                      ,QString format
                      ,QString filter)
{
    QString old_path = g_settings.value(HKEY, "").toString();
//    if(old_path=="")
//    {
//        old_path=QDesktopServices::storageLocation(QDesktopServices::DesktopLocation);
//        old_path+="/";
//    }

    QString fileName = QFileDialog::getSaveFileName(parent
                                                    ,title
                                                    ,old_path+format
                                                    ,filter);
    qDebug()<<"old_path="<<old_path;
    if(fileName.isEmpty() == true)
        return "";


    QString new_path = QFileInfo(QFile(fileName)).path() + "/";
    qDebug()<<"new_path="<<new_path;
    if(old_path != new_path)
        g_settings.setValue(HKEY, new_path);

    return fileName;
}

void g_msleep(int msec)
{
    QEventLoop ep;
    QTimer::singleShot(msec,&ep, SLOT(quit()));
    ep.exec();
}

void g_createPath(QString path)
{
    QDir dir(path);
    if(dir.exists()==false)
    {
       QDir().mkpath(path) ;
    }
}

//使用动画切换QLabel的信息
//isUpToDown--是否从上往下滚动
void g_showLabelTextWithAnim(QLabel *label,QString text,bool isUpToDown)
{
    QWidget *parentW=label->parentWidget();
    if(parentW==0)
    {
        label->setText(text);
        return;
    }
//    QScreen *screen = parentW->windowHandle()->screen();
//    QPixmap parentPix = screen->grabWindow(parentW->winId());
    QPixmap parentPix = parentW->grab();
    QPoint pos=label->mapTo(parentW,QPoint(0,0));
    QRect rect(pos.x(),pos.y(),label->width(),label->height());
    QPixmap oldPixmap=parentPix.copy(rect);
    QString oldText=label->text();
    label->setText(text);
    label->repaint();
    parentPix = parentW->grab();
    label->setText(oldText);
    label->repaint();

    QPixmap newPixmap=parentPix.copy(rect);

    QPixmap pixmap(oldPixmap.width(),oldPixmap.height()*2);
    QPainter painter(&pixmap);
    if(isUpToDown==true)
    {
        painter.drawPixmap(0,0,newPixmap);
        painter.drawPixmap(0,oldPixmap.height(),oldPixmap);
    }
    else
    {
        painter.drawPixmap(0,0,oldPixmap);
        painter.drawPixmap(0,oldPixmap.height(),newPixmap);
    }

    painter.end();

//    qDebug()<<"pixmap.height()="<<pixmap.height()
//            <<"label->height()="<<label->height()
//            <<"label->pos()="<<label->pos();

    QLabel *labelAnim=new QLabel(label);
    labelAnim->resize(pixmap.size());

    labelAnim->setPixmap(pixmap);
    labelAnim->show();

    QPropertyAnimation *anim=new QPropertyAnimation(labelAnim,"pos");
    anim->setDuration(380);
    anim->setEasingCurve(QEasingCurve::OutQuad);
    if(isUpToDown==true)
    {
        anim->setStartValue(QPoint(0,-oldPixmap.height()*4/3));
        anim->setEndValue(QPoint(0,0));
    }
    else
    {
        anim->setStartValue(QPoint(0,-oldPixmap.height()/3));
        anim->setEndValue(QPoint(0,-oldPixmap.height()));
    }
    anim->start();
    QObject::connect(anim,SIGNAL(finished()),labelAnim,SLOT(deleteLater()));
    QObject::connect(anim,SIGNAL(finished()),anim,SLOT(deleteLater()));
    label->setText(text);
}

/*************************************************
** Function: g_getAvailableComList
** Description: 获取可使用的串口列表
*************************************************/
QStringList g_getAvailableComList()
{
    QString path = "HKEY_LOCAL_MACHINE\\HARDWARE\\DEVICEMAP\\SERIALCOMM\\";
    QSettings settings( path, QSettings::NativeFormat);
    QStringList key = settings.allKeys();
    QStringList com_list;
    int num = key.size();
    qDebug()<<"";
    qDebug()<<"num="<<num;
    HKEY hKey;
    wchar_t keyname[256]; //键名数组
    char keyvalue[256];  //键值数组
    DWORD keysize,type,valuesize;
    for(int index=0; index<num; index++)
    {
        if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"),0,KEY_READ,&hKey)!=0)
        {
            continue;//无法打开注册表时返回error
        }

        QString valuemessage;//键值
//        index = index;//要读取键值的索引号
        keysize=sizeof(keyname);
        valuesize=sizeof(keyvalue);
        if(RegEnumValue(hKey,index,keyname,&keysize,0,&type,(BYTE*)keyvalue,&valuesize)==0)//列举键名和值
        {

            for(int j=0;j<valuesize;j++)
            {
                if(keyvalue[j]!=0x00)
                {
                    valuemessage.append(keyvalue[j]);
                }
            }//for(int j=0;j<valuesize;j++) 读取键值
            com_list.append(valuemessage.toLower());
        }
    }
    return com_list;
    RegCloseKey(hKey);//关闭注册表
}

bool g_exportExcel(QString fileName,QTableWidget *tableWidget)
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
        return false;
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

    return true;
}

/*************************************************
** Function: printer
** Description: 打印QPixmap对象到QPrinter打印机
*************************************************/
void g_print(QPrinter *printer,QPixmap pixmap)
{
    QPainter painter(printer);

    QRect rect = painter.viewport();
    QSize size = pixmap.size();
    size.scale(rect.size(), Qt::KeepAspectRatio); //此处保证图片显示完整
    painter.setViewport(rect.x(), rect.y(),size.width(), size.height());
    painter.setWindow(pixmap.rect());

    painter.drawPixmap(0,0,pixmap); /* 数据显示至预览界面 */

//    painter.end();
}

void g_printMultipage(QPrinter *printer,QList<QPixmap> pixmapList)
{
    QPainter painter(printer);

    for(int i=0;i<pixmapList.count();i++)
    {
        if(i>0)
        {
           printer->newPage();
        }
        QPixmap pixmap=pixmapList.at(i);
        QRect rect = painter.viewport();
        QSize size = pixmap.size();

        size.scale(rect.size(), Qt::KeepAspectRatio); //此处保证图片显示完整
        painter.setViewport(rect.x(), rect.y(),size.width(), size.height());
        painter.setWindow(pixmap.rect());

        painter.drawPixmap(0,0,pixmap); /* 数据显示至预览界面 */
    }
}

QParallelAnimationGroup * g_closeWithAnim(QWidget *widget)
{
    //尝试使用QLabel，但是效果不佳 2016-07-19
//    QLabel *label = new QLabel();
//    label->setWindowFlags(Qt::FramelessWindowHint);
//    QSize size = widget->size();
//    label->resize(size);
//    label->setPixmap(widget->grab());
//    label->move(widget->pos());

    int duration = 280;
    int percent = 11; //上下左右各减少原来长度的 ( 1 / percent )
    QEasingCurve esasing_curve = QEasingCurve::OutCubic;
    QPropertyAnimation *rect_animation =
            new QPropertyAnimation(widget,"size");//使用geometry效果不佳
    rect_animation->setDuration(duration);
    rect_animation->setEndValue(QSize(widget->width() * (percent - 1) / percent,
                                      widget->height() * (percent - 1) / percent));
    rect_animation->setEasingCurve(esasing_curve);

    QPropertyAnimation *pos_animation = new QPropertyAnimation(widget,"pos");
    pos_animation->setDuration(duration);
    pos_animation->setEndValue(QPoint(widget->pos().x() + widget->width() / (percent * 2)
                                      ,widget->pos().y() + widget->height() / (percent * 2) ));

    pos_animation->setEasingCurve(esasing_curve);

    QPropertyAnimation *opacity_animation = new QPropertyAnimation(widget,"windowOpacity");
    opacity_animation->setDuration(duration);
//    opacity_animation->setStartValue(0.9);
    opacity_animation->setEndValue(0.0);
    opacity_animation->setEasingCurve(esasing_curve);


    QParallelAnimationGroup *group = new QParallelAnimationGroup();
    group->addAnimation(rect_animation);
    group->addAnimation(pos_animation);

    group->addAnimation(opacity_animation);
//    widget->setWindowOpacity(0.8);

    QObject::connect(group, SIGNAL(finished()),group, SLOT(deleteLater()));
    QObject::connect(group, SIGNAL(finished()),rect_animation, SLOT(deleteLater()));
    QObject::connect(group, SIGNAL(finished()),pos_animation, SLOT(deleteLater()));
    QObject::connect(group, SIGNAL(finished()),opacity_animation, SLOT(deleteLater()));
    QObject::connect(group, SIGNAL(finished()),widget, SLOT(close()));

    QPropertyAnimation *recover_pos = new QPropertyAnimation(widget,"pos");
    recover_pos->setEndValue(widget->pos());
    QPropertyAnimation *recover_opacity = new QPropertyAnimation(widget,"windowOpacity");
    recover_opacity->setEndValue(1.0);
    QPropertyAnimation *recover_rect = new QPropertyAnimation(widget,"size");
    recover_rect->setEndValue(widget->size());
    QObject::connect(group, SIGNAL(finished()),recover_pos, SLOT(start()));
    QObject::connect(group, SIGNAL(finished()),recover_opacity, SLOT(start()));
    QObject::connect(group, SIGNAL(finished()),recover_rect, SLOT(start()));

    QObject::connect(recover_pos, SIGNAL(finished()),recover_pos, SLOT(deleteLater()));
    QObject::connect(recover_opacity, SIGNAL(finished()),recover_opacity, SLOT(deleteLater()));
    QObject::connect(recover_rect, SIGNAL(finished()),recover_rect, SLOT(deleteLater()));

//    QObject::connect(group, SIGNAL(finished()),label, SLOT(deleteLater()));
//    QObject::connect(group, SIGNAL(finished()),widget, SLOT(hide()));
//    label->show();
//    widget->setWindowOpacity(0.0);

    group->start();
    return group;
}

//画阴影边框
void g_drawShadowBorder(QWidget *w)
{
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);

    int grayWidth = 8;

    path.addRect(grayWidth, grayWidth, w->width()-grayWidth*2,
                 w->height()-grayWidth*2);

    QPainter painter(w);
    painter.setRenderHint(QPainter::Antialiasing, true);
//    painter.fillPath(path, QBrush(QColor(254,254,254)));

    QColor color(0, 0, 0, 15);
    for(int i = 0; i<grayWidth; i++)
    {
        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        path.addRect(grayWidth-i, grayWidth-i, w->width()-(grayWidth-i)*2,
                     w->height()-(grayWidth-i)*2);
        int max_alpha = 25;
        color.setAlpha((sqrt(grayWidth)*max_alpha+1) - sqrt(i)*max_alpha);
        painter.setPen(color);
        painter.drawPath(path);
    }
}

/* 使用动画来切换界面
 * oldPixmap ：原来显示的界面图片
 * newPixmap ：即将呈现的界面图片
 * parent    ：在哪个窗口控件上显示动画
 * isGoLeft  ：往左边滑动还是右边
 */
QPropertyAnimation *g_switchPage(QPixmap oldPixmap
                                , QPixmap newPixmap
                                , QWidget *parent
                                , bool isGoLeft
                                , int duration)
{
    //准备图片
    QSize size = oldPixmap.size();
    QPixmap combinationPixmap(size.width()*2, size.height()); //合成的图片
    QPainter painter(&combinationPixmap);

    if(isGoLeft)//往左边滑动
    {
        painter.drawPixmap(0,0,oldPixmap);
        painter.drawPixmap(size.width(),0,newPixmap.scaled(size));
    }
    else
    {
        painter.drawPixmap(0,0,newPixmap.scaled(size));
        painter.drawPixmap(size.width(),0,oldPixmap);
    }
    painter.end();

    //..创建QLabel控件，用作动画窗口
    QLabel *label = new QLabel(parent);
    label->resize(size.width()*2,size.height());
    label->setPixmap(combinationPixmap);
    label->show();

    QPropertyAnimation *anim = new QPropertyAnimation(label,"pos");
    anim->setDuration(duration);
    anim->setEasingCurve(QEasingCurve::OutQuad);
    if(isGoLeft)
    {
        anim->setStartValue(QPoint(-size.width()/2,0));
        anim->setEndValue(QPoint(-size.width(),0));
    }
    else
    {
        anim->setStartValue(QPoint(-size.width()*3/4,0));
        anim->setEndValue(QPoint(0,0));
    }
    anim->start();
    QObject::connect(anim,SIGNAL(finished()),label,SLOT(deleteLater()));
    QObject::connect(anim,SIGNAL(finished()),anim,SLOT(deleteLater()));
    return anim;
}

QParallelAnimationGroup *g_showWithAnim(QWidget *widget)
{
    if(widget == NULL)
    {
        return NULL;
    }

    widget->show();
    widget->setWindowOpacity(0.0);
    qApp->processEvents();

    int duration = 200;
    int percent = 22; //上下左右各减少原来长度的 ( 1 / percent )
    QEasingCurve esasing_curve = QEasingCurve::OutQuad;
    QPropertyAnimation *size_animation =
            new QPropertyAnimation(widget,"size");//使用geometry效果不佳
    size_animation->setDuration(duration);
    size_animation->setStartValue(QSize(widget->width() * (percent - 1) / percent,
                                      widget->height() * (percent - 1) / percent));
    size_animation->setEndValue(widget->size());
    size_animation->setEasingCurve(esasing_curve);

    QPropertyAnimation *pos_animation = new QPropertyAnimation(widget,"pos");
    pos_animation->setDuration(duration);
    pos_animation->setStartValue(QPoint(widget->pos().x() + widget->width() / (percent * 2)
                                      ,widget->pos().y() + widget->height() / (percent * 2) ));
    pos_animation->setEndValue(widget->pos());
    pos_animation->setEasingCurve(esasing_curve);

    QPropertyAnimation *opacity_animation = new QPropertyAnimation(widget
                                                                   ,"windowOpacity");
    opacity_animation->setDuration(duration);
    opacity_animation->setStartValue(0.0);
    opacity_animation->setEndValue(1);
    opacity_animation->setEasingCurve(esasing_curve);


    QParallelAnimationGroup *group = new QParallelAnimationGroup();
    group->addAnimation(size_animation);
    group->addAnimation(pos_animation);
    group->addAnimation(opacity_animation);

    QObject::connect(group, SIGNAL(finished()),group, SLOT(deleteLater()));
    QObject::connect(group, SIGNAL(finished()),size_animation, SLOT(deleteLater()));
    QObject::connect(group, SIGNAL(finished()),pos_animation, SLOT(deleteLater()));
    QObject::connect(group, SIGNAL(finished()),opacity_animation, SLOT(deleteLater()));
    QObject::connect(group, SIGNAL(finished()),widget, SLOT(update()));

    group->start();

    return group;
}

//生成Md5（将标准的Md5加工了一下）
QString g_codeMd5(QString data)
{
    if(data.count() == 0)
    {
        return data;
    }

    data += "Md5"; //在用户输入的密码基础上，又添加了字符串"Md5", 提高破解难度

    QByteArray hash = QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Md5);
    QString strMD5 = hash.toHex();
//    qDebug()<<"strMD5 ="<<strMD5;
    QString odd; //奇数位置的字符
    QString even;
    int count = strMD5.count();
    for(int i = 0; i < count; i++)
    {
        if(i % 2 == 1)
        {
            odd.append(strMD5.at(i));
        }
        else
        {
            even.append(strMD5.at(i));
        }
    }
    strMD5 = odd + even ;
    return strMD5;
}

//还原Md5
QString g_decodeMd5(QString MD5)
{
    if(MD5.count() == 0)
    {
        return MD5;
    }

    QString odd; //奇数位置的字符
    QString even;
    int count = MD5.count();
    if((count % 2) == 0) //MD5为偶数个字符
    {
        even = MD5.left(count / 2);
        odd  = MD5.right(count / 2);
    }
    else//MD5为奇数个字符
    {
        even = MD5.left(count / 2);
        odd  = MD5.right(count / 2 + 1);
    }

    QString strMD5;
    for(int i = 0; i < count / 2; i++)
    {
        strMD5.append(odd.at(i));
        strMD5.append(even.at(i));
    }
    if((count % 2) == 1) //MD5为偶数个字符
    {
        strMD5.append(odd.right(1));
    }


    return strMD5;
}

double g_setPrecision(double d,int precision)
{
    char buffer[50]={'\0'};
    switch(precision)
    {
    case 1:
        sprintf( buffer, "%.1f", d );
        break;
    case 2:
        sprintf( buffer, "%.2f", d );
        break;
    case 3:
        sprintf( buffer, "%.3f", d );
        break;
    case 4:
        sprintf( buffer, "%.4f", d );
        break;
    case 5:
        sprintf( buffer, "%.5f", d );
        break;
    case 6:
        sprintf( buffer, "%.6f", d );
        break;
    default:
        sprintf( buffer, "%.2f", d );
    }
    return QString(buffer).toDouble();
}

/*************************************************
** Function: g_showTipsDialog
** Description: 显示一个可设置显示信息，同时带有确认按钮的对话框
*************************************************/
MyMessageDialog *g_showTipsDialog(QString msg,QWidget *parent, QMessageBox::Icon icon)
{
////    g_logHere(LOG_INFO(g_log_var_list,""));
//    static MyMessageDialog *messageDialog = NULL;
//    if(messageDialog == NULL)
//    {
//        messageDialog = new MyMessageDialog(/*w*/);
//        messageDialog->setVisible_CancelButton(false);
//        //    messageDialog->click_to_destroy=true;
//    }
//    messageDialog->setMessage(msg,QObject::tr("提示"));
//    messageDialog->setButtonsName(" ", " ", QObject::tr("确定"));//messageDialog是一个静态变量，语言切换的时候应实时更新
//    messageDialog->disconnect();//这是一个静态变量

//    if(parent != NULL)
//    {
//        QPoint parent_pos = parent->pos();
//        messageDialog->move(parent_pos.x() + (parent->width() - messageDialog->width()) / 2
//                  ,parent_pos.y() + (parent->height() - messageDialog->height()) / 2);
////        qDebug()<<"this->pos()="<<messageDialog->pos();
//    }

//    messageDialog->exec();
////    qApp->processEvents();
//    return messageDialog;

    QMessageBox msgBox;
    msgBox.setWindowTitle(QMessageBox::tr("提示"));
    msgBox.setIcon(icon);
    msgBox.setText(msg);
    msgBox.exec();

//    QMessageBox::information(parent,
//                             QObject::tr("提示"),
//                             msg); //一般性的提示信息
}

/*************************************************
** Function: g_showTipsDialog
** Description: 显示一个可设置显示信息，同时带有确认、取消按钮的对话框
*************************************************/
bool g_showAskingDialog(QString msg, QWidget *parent)
{
//    LOG_HERE("");
    static MyMessageDialog *messageDialog = new MyMessageDialog;

    messageDialog->setMessage(msg);
    messageDialog->setButtonsName(" ",QObject::tr("取消"),QObject::tr("确定"));
//    messageDialog->show();
    if(parent != NULL)
    {
        QPoint parent_pos = parent->mapToGlobal(QPoint(0 ,0));
        messageDialog->move(parent_pos.x() + (parent->width() - messageDialog->width()) / 2
                  ,parent_pos.y() + (parent->height() - messageDialog->height()) / 2);
//        qDebug()<<"this->pos()="<<messageDialog->pos();
    }
    qApp->processEvents();
    messageDialog->exec();
//    QEventLoop ep;
//    QObject::connect(messageDialog,SIGNAL(buttonClicked()),&ep,SLOT(quit()));
//    ep.exec();
    if(messageDialog->exitCode == "sure")
        return true;
    else
        return false;
}


/*************************************************
** Function: g_isClickTimesOK
** Description: //连续点击(某个控件)指定次数times则返回true
*************************************************/
bool g_isClickTimesOK(int times)
{
//    LOG_HERE("");
    static int click_times = 0;
    static qint64 last_time = 0;
    qint64 now_time = QDateTime::currentDateTime().toMSecsSinceEpoch();//unit:milliseconds

    if( (now_time - last_time) < 500)//两次点击间隔时间小于500毫秒
    {
        click_times++;
        if(click_times >= times)//设为3，
        {
            click_times = 0;
//            usleep(1000*80);
            g_msleep(80);
            return true;
        }
    }
    else//否则重新计数
    {
        click_times = 1;
    }
    last_time = now_time;
    return false;
}

QString g_getNowTime(QString format)
{
    return QDateTime::currentDateTime().toString(format);
}

extern int g_getRandomNum(int max)
{
    static bool isDone = false;
    if(isDone == false)
    {
        srand((int)time(NULL)); //每次执行种子不同，生成不同的随机数
        isDone = true;
    }
    int num = qrand() % max;
    return num;
}

//扫描端口，存在端口返回 true
bool g_scanPort(char *Ip, int port)
{
    int StartPort = port;
    int EndPort = port;
    clock_t StartTime,EndTime;   //cloct_t是int的一个别名，表示时间
    float CostTime;
    TIMEVAL TimeOut;  //timeval是个结构体，变量成员有tv_sec（秒），tv_usec（毫秒）
    FD_SET mask;      // 表示文件的集合，这是为下面的非阻塞I/O做准备，因为要将一个connect调用返回的fd设置在mask
    //里，然后调用select从中调试可以可读或者可写，或者预期的，可连接的fd


    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server;  //服务器的相关信息


    int CurrPort;
    int ret;
    unsigned long mode = 1;  //ioctlsocket函数的最后一个参数，设置目前模式是非阻塞模式


    WSAStartup(MAKEWORD(2, 2), &wsa);


    TimeOut.tv_sec = 0;
    TimeOut.tv_usec = 50;  //超时为50ms
//    MY_DEBUG("ip="<<Ip);


    FD_ZERO(&mask);   //mask初始化


    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(Ip);

    bool existed = false;
    StartTime = clock();

    for(CurrPort = StartPort; CurrPort <= EndPort; CurrPort++)
    {
        s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        FD_SET(s, &mask);
        ioctlsocket(s, FIONBIO, &mode);  //设置为非阻塞模式，FIONBIO means fctl I/O to Nonblocking I/O
        server.sin_port = htons(CurrPort);
        connect(s, (struct sockaddr *)&server, sizeof(server));

        g_msleep(10);
        ret = select(0, NULL, &mask, NULL, &TimeOut);  //查询可写入状态,如果想知道哪些文件可读或者可写，下面要用到FD_ISSET(int socket)来进行判断是否是此socket可读或者可写
        if(0 == ret || -1 == ret)
        {
            closesocket(s);
        }
        else
        {
            //            printf("%s:%d\n", Ip, CurrPort);
            closesocket(s);
            existed = true;
            break;
        }
    }


    EndTime = clock();
    CostTime = (float)(EndTime - StartTime) / CLOCKS_PER_SEC;
//    printf("Cost time:%f second\n", CostTime);

    WSACleanup();

    return existed;
}

//去除 PushButton 的focus ，因为在 Win XP 系统下 button 会产生一个虚线框
void g_clearPushButtonFocus(QWidget *widget)
{
    QList<QPushButton *> mybuttonlist = widget->findChildren<QPushButton *>();
    foreach(QPushButton *button, mybuttonlist)
    {
        button->setFocusPolicy(Qt::NoFocus);
    }
}

void g_updateTesterInfo(QLabel *label)
{
    if(label == 0)
    {
        MY_DEBUG("");
        return;
    }
    SystemStatus *systemStatus = SystemStatus::getInstance();
    if(systemStatus->m_isTestHostConnected)
    {
        QString info = systemStatus->m_testerStatus.m_hostName;
        info ="# " + info + " ( " + systemStatus->m_testerStatus.m_hostIP + " )";
        label->setText(info);
    }
    else
    {
        label->clear();
    }
}
