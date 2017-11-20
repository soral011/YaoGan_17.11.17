#include "Results.h"
#include "ui_Results.h"
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QFileDialog>
#include <QPainter>
#include <QPrintPreviewDialog>

#include <QTimer>
#include <QDateTime>

Results::Results(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TestResults)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);

    m_sqlTableModel = NULL;
    m_lastRowPosition = 0;
//    ui->verticalWidget_filter->hide();

    m_checkBoxList = this->findChildren<QCheckBox *>();
    int i = m_checkBoxList.count();
    QSignalMapper *checkBoxSignalMapper = new QSignalMapper;
    while(i--)
    {
        checkBoxSignalMapper->setMapping(m_checkBoxList.at(i), m_checkBoxList.at(i));
        connect(m_checkBoxList.at(i), SIGNAL(clicked()),
                    checkBoxSignalMapper, SLOT(map()));
    }
    connect(checkBoxSignalMapper, SIGNAL(mapped(QWidget * )),
            this, SLOT(checkBoxClicked(QWidget * )));
    connect(this, SIGNAL(destroyed()),checkBoxSignalMapper, SLOT(deleteLater()));

    connect(ui->tableWidget_menu->horizontalHeader(),SIGNAL(sectionClicked(int ))
            ,this, SLOT(tableWidgetSectionClicked(int)));

    connect(&m_showOneRowTimer,SIGNAL(timeout()),
            this,SLOT(showRows()));

    m_viewCarPicWidget = new ViewCarPic(this);
    m_viewCarPicWidget->hide();
    ui->pushButton_show_car_pic->hide();

    ui->comboBox_test_result_date->clear();

    g_connectButtonSignal(this);

    connect(m_viewCarPicWidget,SIGNAL(switchItem(bool))
            ,this,SLOT(switchItem(bool)));

    g_clearPushButtonFocus(this);
    qDebug()<<"called TestResults constructor";
}

Results::~Results()
{
    delete ui;
}

void Results::buttonReleased(QWidget *buttonW)
{
    QString buttonName = buttonW->objectName();
//    QPushButton *nowbutton = qobject_cast<QPushButton *>(w);
//    qDebug()<<"nowbutton_name = "<<nowbutton_name;

    QRegExp regExp("print|show_car_pic|export_excel|view_car|save_pic|\
                   pushButton_filter|clear_filter");//注意这里的斜杠'\'放在'|'后面
    if(buttonName.contains(regExp) == true)
    {
       if(m_sqlTableModel == NULL)
       {
           QMessageBox::warning(this, tr("提示"),
                           tr("未打开检测结果数据库文件，请选择打开！"));
           return;
       }
    }

    if(buttonName == "pushButton_print_menu")
    {
        if(ui->tableWidget_menu->rowCount() == 0)
        {
            MY_DEBUG("");
            return;
        }
        if(ui->tableWidget_menu->rowCount() == m_sqlTableModel->rowCount())
        {
            m_printForm.printSummaryMenu(ui->tableWidget_menu);
        }
        else
        {
            g_showTipsDialog(tr("正在加载，请稍等！"), this);
        }
    }
    else if(buttonName == "pushButton_print_single_item")
    {
        QList<PropertyAndVariant> testResultList;
        if(ui->tableWidget_menu->rowCount() == 0 ||
           m_lastSelectedPosition >= ui->tableWidget_menu->rowCount())
        {
            MY_DEBUG("");
            return;
        }
        int columnCount = m_sqlTableModel->columnCount();
        for(int column = 0;column<columnCount;column++)
        {
            QVariant variant = m_sqlTableModel->index(m_lastSelectedPosition,
                                                    column).data();
            QString propertyName = m_sqlTableModel->headerData(column,Qt::Horizontal)
                                                    .toString();
            testResultList.append(PropertyAndVariant(propertyName,variant));
        }
        m_printForm.printSingleItem(testResultList);

    }
    else if(buttonName == "pushButton_open")
    {
        //打开窗口让用户选中数据库文件
        QString fileName ;
        fileName =  QFileDialog::getOpenFileName(this
                                       ,tr("选择数据库文件")
                                       ,g_settings.value(HKEY_RESULTS_PATH,"").toString()
                                       ,tr("Access (*.mdb)"));

        if (fileName.isEmpty() == true)
        {
            qDebug()<<"fileName.isEmpty() == true";
            return ;
        }
        openTestResult(fileName);
    }
    else if(buttonName == "pushButton_show_car_pic")
    {
        m_viewCarPicWidget->show();
        buttonW->hide();
        ui->pushButton_save_pic->show();
    }
    else if(buttonName == "pushButton_export_excel")
    {
        QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd_hhmmss");
        QString fileName  =  QFileDialog::getSaveFileName(this
                                                        ,"导出Excel文件"
                                                        ,time+".xls"
                                                        ,tr("Excel文件 (*.xls)"));
        if(fileName.isEmpty())
        {
            return;
        }
        g_exportExcel(fileName,ui->tableWidget_menu);
    }

    else if(buttonName == "pushButton_view_car_maximize")
    {
        int oldPosition = m_lastSelectedPosition;
        static QString original_style = buttonW->styleSheet();
        if(m_viewCarPicWidget->size() != ui->tableWidget_menu->size())
        {
            m_viewCarPicWidget->resize(ui->tableWidget_menu->size());
            m_viewCarPicWidget->move(ui->tableWidget_menu->pos());
            buttonW->setStyleSheet(original_style.replace("max","normal"));
        }
        else
        {
            this->showCarPicWidgetNormal(); //调用这个函数时会改变m_lastSelectedPosition的值
            buttonW->setStyleSheet(original_style.replace("normal","max"));
        }
        this->on_tableWidget_menu_itemClicked(
                    ui->tableWidget_menu->item(oldPosition, 0));
    }
    else if(buttonName == "pushButton_view_car_close")
    {
        static bool isShowAlready = false;
        m_viewCarPicWidget->hide();
        ui->pushButton_save_pic->hide();
        ui->pushButton_show_car_pic->show();

        //显示一次就好了
        if(isShowAlready == false)
        {
            isShowAlready = true;
            m_tips3DDialog.showMessage(tr("点击这里可以显示车辆图片！"),
                                       ui->pushButton_show_car_pic);
        }

        qDebug()<<"columnCount = "<<m_sqlTableModel->columnCount();
        qDebug()<<"rowCount = "<<m_sqlTableModel->rowCount();
    }
    else if(buttonName == "pushButton_save_pic")
    {
        if(m_lastSelectedPosition >= ui->tableWidget_menu->rowCount())
        {
            return;
        }

        //提取图片
        int position = getColumnPosition(PROPERTY_PIC);
        QVariant variant = m_sqlTableModel->index(m_lastSelectedPosition,
                                                position).data();

        QByteArray data = variant.toByteArray();
        qDebug()<<"data.count() = "<<data.count();
        QPixmap pixmap;
        pixmap.loadFromData(data);
        qDebug()<<"pixmap.isNull() = "<<pixmap.isNull();
        if(pixmap.isNull() == false)
        {
            //提取车牌
            position = getColumnPosition(PROPERTY_lICENCE);
            variant = m_sqlTableModel->index(m_lastSelectedPosition,
                                                            position).data();
            QString license = variant.toString();

            QString time = QDateTime::currentDateTime().toString("_yyyy-MM-dd");
            license != time;

            QString img_suffix = "png";
            QString fileName  =  QFileDialog::getSaveFileName(this
                                                            ,tr("另存图片")
                                                            ,license + "." + img_suffix
                                                            ,QString("(*.%1)").arg(img_suffix));
            if(fileName.isEmpty())
            {
                return;
            }
            bool isOk = pixmap.save(fileName, img_suffix.toUtf8(), 100);
            qDebug()<<"isOk = "<<isOk;
        }
    }
    else if(buttonName == "pushButton_previous_day" ||
            buttonName == "pushButton_next_day")
    {
//        if(m_showResultTimer.isActive())
//        {
//            MY_DEBUG("");
//            return;
//        }

        QList<QString> resultDateList = this->findTestResultDate();

        if(resultDateList.count() == 0)
        {
            MY_DEBUG("");
            return;
        }
//        ui->comboBox_test_result_date->clear();
//        ui->comboBox_test_result_date->addItems(resultDateList);

        //            QString currentResultData = ui->label_current_result_date->text();
        QString currentResultDate = ui->comboBox_test_result_date->currentText();
        //            currentResultData = TEST_RESULT_FILE_NAME(currentResultData);
        int index;
        if(resultDateList.contains(currentResultDate) == true)
        {
            MY_DEBUG("");
            index = resultDateList.indexOf(currentResultDate);
        }
        else
        {
            MY_DEBUG("");
            index = resultDateList.count()-1;
        }
//        MY_DEBUG("resultDateList = "<<resultDateList);
//        qDebug()<<"resultDateList.at(index) = "<<resultDateList.at(index);

//        int index = resultDateList.indexOf(currentResultData);
        qDebug()<<"index = "<<index;
        while(1)
        {
            //判断是选择前一天还是后一天
            if(buttonName.contains("previous"))
            {
                index -= 1;
                if(index<0)
                {
                    break;
                }
            }
            else
            {
                index += 1;
                if(index == resultDateList.count())
                {
                    break;
                }
            }

            QString fileName = resultDateList.at(index);
            fileName = TEST_RESULT_FILE_NAME(fileName);
            if(this->hasTestResult(fileName))
            {
                m_testResultsPath = RESULT_SAVE_PATH+fileName+"/";

                qDebug()<<"m_testResultsPath = "<<m_testResultsPath;

                //打开数据库
                openTestResult(QDir(RESULT_SAVE_PATH+fileName).absolutePath());


                ui->comboBox_test_result_date->clear();
                ui->comboBox_test_result_date->addItems(resultDateList);
                int i = ui->comboBox_test_result_date->findText(
                            resultDateList.at(index));
                if(i >= 0)
                {
                    ui->comboBox_test_result_date->setCurrentIndex(i);
                }

                break;
            }
        }
    }
    else if(buttonName == "pushButton_show_filter_widget" ||
            buttonName == "pushButton_hide_filter_widget")
    {
        static int maxWidth = ui->verticalWidget_filter->maximumWidth();
        bool value = ui->verticalWidget_filter->isVisible();
        int width = ui->verticalWidget_filter->width();
        if(value == true && width != maxWidth)
        {
            return;
        }

        QPropertyAnimation *anim = new QPropertyAnimation(
                    ui->verticalWidget_filter,"maximumWidth");
        anim->setDuration(280);
        anim->setEasingCurve(QEasingCurve::OutQuad);

        //隐藏
        if(value == true)
        {
            anim->setStartValue(maxWidth);
            anim->setEndValue(0);
            connect(anim,SIGNAL(finished()),ui->verticalWidget_filter,SLOT(hide()));
        }
        else//显示
        {
            ui->verticalWidget_filter->show();
            anim->setStartValue(0);
            anim->setEndValue(maxWidth);
        }
        anim->start();

        connect(anim,SIGNAL(finished()),anim,SLOT(deleteLater()));

//        ui->verticalWidget_filter->setVisible(!value);
    }
    else if(buttonName == "pushButton_filter")
    {
        this->filterDatabase();
    }
    else if(buttonName == "pushButton_clear_filter")
    {
        this->m_sqlTableModel->setTable(DB_TABLE_TESTDATA);
        this->m_sqlTableModel->setFilter("");
        this->m_sqlTableModel->select();
        showTestResult();
    }
    else if(buttonName == "pushButton_minimize")
    {
        this->showMinimized();
    }
    else if(buttonName == "pushButton_close")
    {
        g_closeWithAnim(this);
    }
}

void Results::openTestResult(QString fileName)
{
    m_showOneRowTimer.stop();

    MY_DEBUG(g_getNowTime());
    //保存用户的选择路径
    QFileInfo fileInfo(fileName);
    g_settings.setValue(HKEY_RESULTS_PATH, fileInfo.path());

    m_testResultsPath = fileInfo.path()+"/";

    bool isOk = m_accessDatabase.createConnection(fileName);
    if(isOk == true)
    {
        m_currentDbFileName = fileName;
        m_sqlTableModel = m_accessDatabase.m_sqlTableModel;
//        ui->tableView_menu->setModel(m_sqlTableModel);

    }
    else
    {
        return;
    }

    //根据数据库指定的数据表
    ui->tableWidget_menu->setColumnCount(g_propertyList.count());
    ui->tableWidget_menu->horizontalHeader()->setVisible(true);
    ui->tableWidget_menu->clear();

    this->m_sqlTableModel->setTable(DB_TABLE_TESTDATA);
    this->m_sqlTableModel->setFilter("");
    this->m_sqlTableModel->select();
    showTestResult();
//    qDebug()<<"columnCount = "<<m_sqlTableModel->columnCount();
//    qDebug()<<"rowCount = "<<m_sqlTableModel->rowCount();
}

void Results::showTestResult()
{
    MY_DEBUG(g_getNowTime());
    //考虑效率问题，m_sqlTableModel的默认只取256条数据
    //全部加载使用如下语句
    while(m_sqlTableModel->canFetchMore())
    {
//        g_msleep(20);
        m_sqlTableModel->fetchMore();
    }
    MY_DEBUG(g_getNowTime());

    //更新数据库指定的数据表
    int rowCount = this->m_sqlTableModel->rowCount();
    ui->tableWidget_menu->setRowCount(0);//清空原来的内容
//    ui->tableWidget_menu->setRowCount(rowCount);
    ui->label_total->setText(QString().setNum(rowCount));
    if(rowCount == 0)
    {
        MY_DEBUG("rowCount == 0");
        return;
    }

     MY_DEBUG(g_getNowTime());

    //显示表头
    for(int column = 0;column<g_propertyList.count();column++)
    {
//        qDebug()<<"position = "<<position<<"rowCount = "<<rowCount;
        QTableWidgetItem *item;

        item = ui->tableWidget_menu->horizontalHeaderItem(column);
        if(item == 0)
        {
            item = new QTableWidgetItem();
            ui->tableWidget_menu->setHorizontalHeaderItem(column,item);
        }
        item->setText(g_propertyList.at(column).m_text);
        if(g_propertyList.at(column).m_property == PROPERTY_PASS_TIME)
        {
            ui->tableWidget_menu->horizontalHeader()
                    ->resizeSection(column,160);
        }
        else if(g_propertyList.at(column).m_property == PROPERTY_lICENCE)
        {
            ui->tableWidget_menu->horizontalHeader()
                    ->resizeSection(column,79);
        }
        else
        {
            //自动切换HC|CO|CO2|NO|O2|PEF字体大小
            QFontMetrics font_metrics = ui->tableWidget_menu
                    ->horizontalHeader()->fontMetrics();

            int width = font_metrics.width(g_propertyList.at(column).m_text);
            ui->tableWidget_menu->horizontalHeader()
                    ->resizeSection(column,width+29);
        }
    }
//    ui->tableWidget_menu->hideColumn(m_propertyList.count()-1);

    m_lastRowPosition = 0;
    m_showOneRowTimer.start(6);
//    m_lastSelectedPosition = 1;
     MY_DEBUG(g_getNowTime());
     MY_DEBUG("");
}

//显示一行数据（一行代表一辆车的数据）
void Results::showRows()
{
    m_showOneRowTimer.stop();
    int size = 1; //每次显示多少行
    while(size--)
    {
        int row = m_lastRowPosition;
        for(int column = 0; column < g_propertyList.count(); column++)
        {
            ui->tableWidget_menu->setRowCount(row + 1);
            QTableWidgetItem *item;
            item = ui->tableWidget_menu->item(row, column);
            if(item == 0)
            {
                item = new QTableWidgetItem();
                ui->tableWidget_menu->setItem(row, column, item);
            }

            QString text = "";
            int position = getColumnPosition(g_propertyList.at(column).m_property);
            if(position < 0)
            {
                continue;
            }

            QVariant variant = m_sqlTableModel->index(row, position).data();
            if(variant.type() != QVariant::Double)
            {
                text = variant.toString();
            }

            if(g_propertyList.at(column).m_property == PROPERTY_PASS_TIME)
            {
                text.replace("T"," ");
            }
//            else if(g_propertyList.at(column).m_property == PROPERTY_IS_PASS)
//            {
//                bool value = variant.toBool();
//                if(value == false)
//                    text = tr("超标");
//                else
//                    text = tr("合格");
//            }
            else if(g_propertyList.at(column).m_property == PROPERTY_SEQUENCE_NUMBER)
            {
                //最后一列（PROPERTY_SEQUENCE_NUMBER）用于保存文件名，点击显示车牌图片、删除车牌时用
                text = m_resultNameList.first();
            }
            //            qDebug()<<"variant = "<<variant;

            if(text != "")
            {
                item->setText(text);
            }
            else//数字，为排序做准备
            {
                bool ok;
                float f = variant.toFloat(&ok);
                //                qDebug()<<"ok = "<<ok;
                if(ok == true)
                    item->setData(Qt::DisplayRole, f);
                else
                    item->setData(Qt::DisplayRole, variant);
            }
        }

        if( row + 1 >= m_sqlTableModel->rowCount())//是否显示到末尾了
        {
            m_showOneRowTimer.stop();//停止刷新显示
            MY_DEBUG(g_getNowTime());
            return;
        }
        else if(row == 0)
        {
            ui->tableWidget_menu->scrollToTop();

            if(m_viewCarPicWidget->isHidden() && ui->pushButton_show_car_pic->isHidden())
            {
                QTimer::singleShot(200, this, SLOT(showCarPicWidgetNormal()));
            }
            else
            {
                this->on_tableWidget_menu_itemClicked(ui->tableWidget_menu->item(0,0));
            }
        }

        m_lastRowPosition++;
    }

//    MY_DEBUG(g_getNowTime());

    m_showOneRowTimer.start();

//    MY_DEBUG(g_getNowTime());
}

void Results::on_tableWidget_menu_itemClicked(QTableWidgetItem *item)
{
    if(item == 0)
    {
        MY_DEBUG("item == 0");
        return;
    }
    int row = item->row();
    m_lastSelectedPosition = row;

    //获取图片
    int position = this->getColumnPosition(PROPERTY_PIC);
    QVariant variant = m_sqlTableModel->index(row,position).data();
    QByteArray ba = variant.toByteArray();
    QPixmap pixmap;
    pixmap.loadFromData(ba);
    if(pixmap.isNull())
    {
        return;
    }
    m_viewCarPicWidget->m_pixmap = pixmap;

    //获取车牌
    //..车牌颜色
    position = this->getColumnPosition(PROPERTY_BG_COLOR);
    variant = m_sqlTableModel->index(row,position).data();
    QString bgColor = variant.toString();

    //..车牌号
    position = this->getColumnPosition(PROPERTY_lICENCE);
    variant = m_sqlTableModel->index(row,position).data();
    QString licence = variant.toString();

    //..第几条
    int count = ui->tableWidget_menu->rowCount();
    QString rowNum = QString().setNum(row+1)+"/"+QString().setNum(count);

    m_viewCarPicWidget->m_license = bgColor+licence+"       "+rowNum;

    QTimer::singleShot(1,m_viewCarPicWidget,SLOT(showCarInfo()));

    //操作员 和 监测地点
    AccessDatabase accessDatabase; //不使用成员变量 m_accessDatabase，移动处于筛选情况下，处理会比较复杂
    bool isOk = accessDatabase.createConnection(m_currentDbFileName);
    if(isOk)
    {
        //..操作员
        position = this->getColumnPosition(PROPERTY_USER_ID);
        QString userID = m_sqlTableModel->index(row, position).data().toString();
        if(m_userID != userID)
        {
            m_userID = userID;
            QString userChineseName = accessDatabase.getUserName(userID);
            ui->label_operator->setText(userChineseName);
        }

        //..操作地点
        position = this->getColumnPosition(PROPERTY_SITE_ID);
        QString siteID = m_sqlTableModel->index(row, position).data().toString();
        if(m_siteID != siteID)
        {
            m_siteID = siteID;
            QString siteName = accessDatabase.getSiteName(siteID);
            ui->label_site->setText(siteName);
        }
    }
}



//切换显示上/下一条目
void Results::switchItem(bool isDown)
{
    int old = m_lastSelectedPosition;
    if(isDown == true)
        m_lastSelectedPosition++;
    else
        m_lastSelectedPosition--;

    if(0 <= m_lastSelectedPosition
       && m_lastSelectedPosition <= ui->tableWidget_menu->rowCount()-1)
    {
        ui->tableWidget_menu->selectRow(m_lastSelectedPosition);
        this->on_tableWidget_menu_itemClicked(
                    ui->tableWidget_menu->item(m_lastSelectedPosition,0));
    }
    else
    {
        m_lastSelectedPosition = old;
    }
}

//在右下角显示小窗口（显示机动车照片）
void Results::showCarPicWidgetNormal()
{
    static QSize size = m_viewCarPicWidget->size();
    m_viewCarPicWidget->resize(size);
    m_viewCarPicWidget->show();
    QPoint pos = ui->tableWidget_menu->pos();
    int width = ui->tableWidget_menu->width();
    int height = ui->tableWidget_menu->height();
    m_viewCarPicWidget->move(pos.x()+width-m_viewCarPicWidget->width()-18
                             ,pos.y()+height-m_viewCarPicWidget->height()-18);

    on_tableWidget_menu_itemClicked(ui->tableWidget_menu->item(0,0));
}

void Results::mouseMoveEvent(QMouseEvent *event)
{
//    qDebug()<<event->pos();
}

void Results::printTestPage(QPrinter *printer)
{
    int margin = 10;
    printer->setPageMargins(margin,margin,3,margin,QPrinter::Millimeter);
    printer->setOrientation(QPrinter::Landscape);

    QPainter painter(printer);

    int verticalHeaderHeight = ui->tableWidget_menu->verticalHeader()->defaultSectionSize();
    int verticalHeaderWidth = ui->tableWidget_menu->horizontalHeader()->width();

    int width = ui->tableWidget_menu->verticalHeader()->width();
    width != verticalHeaderWidth;

    QRect rect  =  painter.viewport();
    QSize size1;
    size1.scale(rect.size(), Qt::IgnoreAspectRatio); //此处保证图片显示完整
    painter.setViewport(rect.x(), rect.y(),size1.width(), size1.height());

    painter.setWindow(QRect(0,0,width,verticalHeaderHeight*29));


    int rowCount = ui->tableWidget_menu->rowCount();

    QRectF rectf(0,0,width,70);
    painter.drawText(rectf,tr("机动车尾气遥测数据报表"),QTextOption(Qt::AlignHCenter));

//    if(rowCount<25)
    {
        int height = verticalHeaderHeight * rowCount;
        height += ui->tableWidget_menu->horizontalHeader()->height();
        height += 1;

        QPixmap pixmap;
        pixmap = QPixmap::grabWidget(ui->tableWidget_menu,0,0,width,height);

        painter.drawPixmap(0,80,pixmap);
    }

//    int height = verticalHeaderHeight*12;
//    height != ui->tableWidget_menu->horizontalHeader()->height();
//    height != 1;



//    QPixmap pixmap;
//    pixmap = QPixmap::grabWidget(ui->tableWidget_menu,0,0,width,height);

//    painter.drawPixmap(0,0,pixmap);
}

void Results::tableWidgetSectionClicked(int section)
{
    static bool is_up = true;

    if(ui->tableWidget_menu->rowCount() == 0)
    {
        MY_DEBUG("rowCount() == 0");
        return;
    }

//    ui->tableWidget_menu->sortByColumn(section);
    int position = this->getColumnPosition(
                g_propertyList.at(section).m_property);
    if(position < 0)
    {
        return;
    }
    m_sqlTableModel->sort(position,(Qt::SortOrder)is_up);

    ui->tableWidget_menu->horizontalHeader()->setIconSize(QSize(8,8));

    QPixmap pixmap(":/images/sort_arrow.png");
//    pixmap.scaled(pixmap.width()/2,pixmap.height()/2);
    if(is_up == false)
    {
        QMatrix leftmatrix;
        leftmatrix.rotate(180);
        pixmap = pixmap.transformed(leftmatrix,Qt::SmoothTransformation);
    }
    is_up = !is_up;
    QIcon icon(pixmap);
    QTableWidgetItem *selectedItem = ui->tableWidget_menu->
                                            horizontalHeaderItem(section);
    selectedItem->setIcon(icon);

    int count = ui->tableWidget_menu->horizontalHeader()->count();
    for(int i = 0; i < count; i++)
    {
        QTableWidgetItem *item = ui->tableWidget_menu->horizontalHeaderItem(i);
        if(item != selectedItem)
        {
            item->setIcon(QIcon());
        }
    }
    showTestResult();
}

//根据给定属性获取其在数据库中的列位置
int Results::getColumnPosition(QString property)
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

void Results::showEvent(QShowEvent * event)
{
//    this->showMaximized();
//    QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd");
//    ui->label_current_result_date->setText(date);
//    this->repaint();

    if(ui->comboBox_test_result_date->count())//如果已经打开了结果数据库文件则返回
    {
        //更新数据库文件列表到comboBox
        QString date = ui->comboBox_test_result_date->currentText();
        ui->comboBox_test_result_date->clear();
        ui->comboBox_test_result_date->addItems(this->findTestResultDate());
        ui->comboBox_test_result_date->setCurrentText(date);
    }
    else
    {
        ui->comboBox_test_result_date->clear();

        QList<QString> resultDateList = this->findTestResultDate();
        if(resultDateList.count())
        {
            QString date = resultDateList.last();

            QString fileName = TEST_RESULT_FILE_NAME(date);
            if(hasTestResult(fileName) == true)
            {
                m_testResultsPath = RESULT_SAVE_PATH + date + "/";
                openTestResult(QDir(RESULT_SAVE_PATH + fileName).absolutePath());

                ui->comboBox_test_result_date->addItems(this->findTestResultDate());
                ui->comboBox_test_result_date->setCurrentText(date);
            }
        }
    }


//    QString fileName = TEST_RESULT_FILE_NAME(date);
//    if(hasTestResult(fileName) == true)
//    {
//        m_testResultsPath = RESULT_SAVE_PATH + date + "/";
//        openTestResult(QDir(RESULT_SAVE_PATH + fileName).absolutePath());

//        ui->comboBox_test_result_date->clear();
//        ui->comboBox_test_result_date->addItems(this->findTestResultDate());
//        ui->comboBox_test_result_date->setCurrentText(date);
//    }
//    else
//    {
//         this->buttonReleased(ui->pushButton_previous_day);
//    }
}

bool Results::hasTestResult(QString fileName)
{
//    QString path = RESULT_SAVE_PATH+date;
//    QDir dir(path);
//    QList<QString> resultNameList = dir.entryList(QDir::Files,QDir::NoSort);
//    QList<QString> strListTmp = resultNameList;
//    foreach(QString str,strListTmp)
//    {
//        if(str.endsWith(".dat") == false)
//        {
//            resultNameList.removeOne(str);
//        }
//    }

//    if(resultNameList.count() == 0)
//        return false;
//    else
//        return true;

    return QFile(RESULT_SAVE_PATH + fileName).exists();
}

void Results::checkBoxClicked(QWidget *w )
{
    QString checkbox_name = w->objectName();
    QCheckBox *checkbox_clicked = qobject_cast<QCheckBox*>(w);
    bool is_checked = checkbox_clicked->isChecked();
    if(checkbox_name == "checkBox_select_all")
    {
        foreach(QCheckBox *checkbox,m_checkBoxList)
        {
            checkbox->setChecked(is_checked);
        }
    }
    else
    {
        ui->checkBox_select_all->setChecked(false);
    }
}

QString Results::createFilter(QCheckBox *checkBox,
                              QComboBox *comboBox,
                              QDoubleSpinBox *dSpinBox,
                              QString property)
{
    QString condition,filterTemp;
    if(checkBox->isChecked())
    {
        QString relationship = " and ";
        condition = comboBox->currentText();
        filterTemp = property + " " + condition + " ";
        filterTemp += QString().setNum(dSpinBox->value());
        filterTemp += relationship;
    }
    return filterTemp;
}

void Results::filterDatabase()
{
    QString filters;
    filters += createFilter(ui->checkBox_CO,
                          ui->comboBox_CO,
                          ui->doubleSpinBox_CO,
                          PROPERTY_C_CO);

    filters += createFilter(ui->checkBox_CO2,
                          ui->comboBox_CO2,
                          ui->doubleSpinBox_CO2,
                          PROPERTY_C_CO2);

    filters += createFilter(ui->checkBox_NO,
                          ui->comboBox_NO,
                          ui->doubleSpinBox_NO,
                          PROPERTY_C_NO);

    filters += createFilter(ui->checkBox_HC,
                          ui->comboBox_HC,
                          ui->doubleSpinBox_HC,
                          PROPERTY_C_HC);

    filters += createFilter(ui->checkBox_speed,
                          ui->comboBox_speed,
                          ui->doubleSpinBox_speed,
                          PROPERTY_SPEED);

    filters += createFilter(ui->checkBox_accelerate,
                          ui->comboBox_accelerate,
                          ui->doubleSpinBox_accelerate,
                          PROPERTY_ACC);

    filters += createFilter(ui->checkBox_opacity,
                          ui->comboBox_opacity,
                          ui->doubleSpinBox_opacity,
                          PROPERTY_OPACITY);

    filters += createFilter(ui->checkBox_K_value,
                          ui->comboBox_K_value,
                          ui->doubleSpinBox_K_value,
                          PROPERTY_OPACITY_PARA);

    filters += createFilter(ui->checkBox_reliability,
                          ui->comboBox_reliability,
                          ui->doubleSpinBox_reliability,
                          PROPERTY_RELIABILITY);

    filters += createFilter(ui->checkBox_VSP,
                          ui->comboBox_VSP_max,
                          ui->doubleSpinBox_VSP_max,
                          PROPERTY_VSP);

    filters += createFilter(ui->checkBox_VSP,
                          ui->comboBox_VSP_min,
                          ui->doubleSpinBox_VSP_min,
                          PROPERTY_VSP);

    QString condition,filterTemp;
    QString relationship = " and ";
    if(ui->checkBox_license->isChecked())
    {
        condition = ui->comboBox_license->currentText();
        QString text = ui->lineEdit_license->text();
        if(condition == tr("含"))
        {
            filterTemp = QString(PROPERTY_lICENCE)+" like '%"+text+"%'";
        }
        else
        {
            filterTemp = QString(PROPERTY_lICENCE)+" not like '%"+text+"%'";
        }
        filters += (filterTemp+relationship);
    }
    if(ui->checkBox_is_pass->isChecked())
    {
        condition = ui->comboBox_is_pass->currentText();
        if(condition == tr("合格"))
        {
            filterTemp = QString(PROPERTY_IS_PASS)+"  =  1";
        }
        else
        {
            filterTemp = QString(PROPERTY_IS_PASS)+"  =  0";
        }
        filters += (filterTemp+relationship);
    }

    if(filters.count()>0)
    {
        filters += "1";//因为filters是以" and "结尾的，在此增加1
    }
    else
    {
        m_tips3DDialog.showMessage(tr("未勾选条件！"),ui->label_filter_conditions);
        return;
    }

    qDebug()<<"b rowCount = "<<m_sqlTableModel->rowCount();
    this->m_sqlTableModel->setTable(DB_TABLE_TESTDATA);
    this->m_sqlTableModel->setFilter(filters);
    this->m_sqlTableModel->select();
    qDebug()<<"a rowCount = "<<m_sqlTableModel->rowCount();
    showTestResult();
}

void Results::on_comboBox_test_result_date_activated(const QString &arg1)
{
    QString fileName = TEST_RESULT_FILE_NAME(arg1);
    if(hasTestResult(fileName) == true)
    {
        openTestResult(QDir(RESULT_SAVE_PATH+fileName).absolutePath());
    }
}

QStringList Results::findTestResultDate()
{
    QDir dir(RESULT_SAVE_PATH);
    QList<QString> resultFileList = dir.entryList(QDir::Files,QDir::Name);
    QList<QString> strListTmp;

    //筛选测量结果文件，文件名称格式为TEST_RESULT_FILE_NAME(yyyy-MM-dd)，
    //TEST_RESULT_FILE_NAME是一个宏定义
    foreach(QString fileName,resultFileList)
    {
        //删除yyyy-MM-dd字符
        QString tmp = QString(fileName).remove(
                    QRegExp("[0-9]{4}-[0-9]{2}-[0-9]{2}"));

        if(tmp == TEST_RESULT_FILE_NAME(""))
        {
            //fileName只保留yyyy-MM-dd，删除.mdb等字符
            QStringList strList = TEST_RESULT_FILE_NAME(",").split(",");
            foreach(QString text,strList)
            {
                fileName.remove(text);
            }
            strListTmp.append(fileName);
        }
    }

    return strListTmp;
}

//void Results::keyPressEvent(QKeyEvent * event)
//{
//    MY_DEBUG("event->key()="<<event->key());
//    if(event->key() == Qt::Key_Left /*|| event->key() == Qt::Key_Up*/)
//    {
//        int row = m_lastSelectedPosition - 1;
//        ui->tableWidget_menu->selectRow(row);
//        this->on_tableWidget_menu_itemSelectionChanged();

//    }
//    else if(event->key() == Qt::Key_Right /*|| event->key() == Qt::Key_Down*/)
//    {
//        ui->tableWidget_menu->selectRow(m_lastSelectedPosition + 1);
//        this->on_tableWidget_menu_itemSelectionChanged();
//    }
//}

void Results::on_tableWidget_menu_itemSelectionChanged()
{
    this->on_tableWidget_menu_itemClicked(ui->tableWidget_menu->currentItem());
//    ui->tableWidget_menu->currentItem();
}





