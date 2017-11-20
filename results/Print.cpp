#include "Print.h"
#include "ui_PrintForm.h"
#include <QPrintPreviewDialog>
#include <QTableWidgetItem>
#include <QTimer>

Print::Print(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrintForm)
{
    ui->setupUi(this);

    //表格的最佳高度,大致由如下语句得来
    //ui->tableWidget_summary_menu->verticalHeader()->length()
    m_maxHeight = 1075;

    //tableWidget_summary_menu最多能够显示多少行
    m_properRowNum = 39;

//    adjustHeadviewWidth();
    g_clearPushButtonFocus(this);
}

Print::~Print()
{
    delete ui;
}

void Print::initHorizontalHeadview()
{
    MY_LOG("start");
    ui->tableWidget_summary_menu->clear();
    ui->tableWidget_summary_menu->setColumnCount(g_propertyList.count());
    ui->tableWidget_summary_menu->setRowCount(1/*m_propercolumnNum*/);

    //隐藏表格自带的表头
    ui->tableWidget_summary_menu->horizontalHeader()->setVisible(false);
    ui->tableWidget_summary_menu->verticalHeader()->setVisible(false);

//    QTableWidgetItem *item;
//    item = ui->tableWidget_summary_menu->item(37,0);
//    if(item == 0)
//    {
//        item = new QTableWidgetItem();
//        ui->tableWidget_summary_menu->setItem(37,0,item);
//    }
//    item->setText("End");


    //显示表头
    for(int column = 0;column<g_propertyList.count();column++)
    {
//        qDebug()<<"position = "<<position<<"rowCount = "<<rowCount;

        //将表格的第0行作为表头，而不是tableWidget->horizontalHeader()
        int row = 0;
        QTableWidgetItem *item;
        item = ui->tableWidget_summary_menu->item(row,column);
        if(item == 0)
        {
            item = new QTableWidgetItem();
            ui->tableWidget_summary_menu->setItem(row,column,item);
        }
        item->setText(g_propertyList.at(column).m_text);
        item->setBackgroundColor(QColor(200,200,200,100));
        item->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

        //调节表头字段的宽度
        if(g_propertyList.at(column).m_property == PROPERTY_PASS_TIME)
        {
            ui->tableWidget_summary_menu->horizontalHeader()
                    ->resizeSection(column,198);
        }
        else if(g_propertyList.at(column).m_property == PROPERTY_lICENCE)
        {
            ui->tableWidget_summary_menu->horizontalHeader()
                    ->resizeSection(column,88);
        }
        else
        {
            QFontMetrics font_metrics = ui->tableWidget_summary_menu
                    ->horizontalHeader()->fontMetrics();

            int width = font_metrics.width(g_propertyList.at(column).m_text);
            if(g_propertyList.at(column).m_property == PROPERTY_OPACITY_MAX ||
               g_propertyList.at(column).m_property == PROPERTY_OPACITY_AVG)
            {
                ui->tableWidget_summary_menu->horizontalHeader()
                        ->resizeSection(column,width+35);
            }
            else
            {
                ui->tableWidget_summary_menu->horizontalHeader()
                        ->resizeSection(column,width+29);
            }
        }
    }
    int maxWidth = ui->tableWidget_summary_menu->horizontalHeader()->length();
//    int maxHeight = ui->tableWidget_summary_menu->verticalHeader()->length();
//    maxHeight += ui->tableWidget_summary_menu->pos().y();
//    qDebug()<<"maxWidth = "<<maxWidth;
//    qDebug()<<"maxHeight = "<<maxHeight;
//    ui->label_summary_menu_title->setMaximumWidth(maxWidth);
//    ui->tableWidget_summary_menu->setMaximumWidth(maxWidth);

    this->resize(maxWidth+1,m_maxHeight);

    MY_LOG("end");
}

void Print::printSummaryMenu(QTableWidget *tableWidget)
{
    MY_LOG("start");
    ui->stackedWidget->setCurrentIndex(0);
    m_pixmapList.clear();

    int rowCount = tableWidget->rowCount();
    int columnCount = tableWidget->columnCount();

    if(rowCount == 0 || columnCount == 0)
    {
        return;
    }
//    ui->tableWidget_summary_menu->setColumnCount(columnCount);

    initHorizontalHeadview();

    for(int row = 0;row<rowCount;row++)
    {
        //由于隐藏了表格自身的表头，并且使用表格的第0行作为表头
        //因此每增加一行内容，表格的行数应增加1
        int rowCountTemp = row%m_properRowNum+2;
        ui->tableWidget_summary_menu->setRowCount(rowCountTemp);

        for(int column = 0;column<columnCount;column++)
        {
            //获取参数tableWidget表格的数据
            QTableWidgetItem *item = tableWidget->item(row,column);
            if(item == 0)
            {
                continue;
            }
            QString text = item->text();

            //将获取到的数据赋值到打印的表格
            item = ui->tableWidget_summary_menu->item(rowCountTemp-1,column);
            if(item == 0)
            {
                item = new QTableWidgetItem();
                ui->tableWidget_summary_menu->setItem(rowCountTemp-1,column,item);
            }
            item->setText(text);
        }

        //一页最多能够显示m_propercolumnNum行，此时截图保存
        //然后从第一行开始填充数据
        if((row+1)%m_properRowNum == 0)
        {
            ui->label_summary_menu_page_umber->setText(
                        tr("第 %1 页").arg(m_pixmapList.count()+1));
            m_pixmapList<<this->grab();

            initHorizontalHeadview();
        }
    }
    ui->label_summary_menu_page_umber->setText(
                tr("第 %1 页").arg(m_pixmapList.count()+1));
    m_pixmapList<<this->grab();

    print(QPrinter::Landscape);
    MY_LOG("end");
}

void Print::printSummaryMenu(QSqlTableModel *sqlTableModel)
{
    MY_LOG("start");
    ui->stackedWidget->setCurrentIndex(0);
    m_pixmapList.clear();

    int rowCount = sqlTableModel->rowCount();
    int columnCount = sqlTableModel->columnCount();

    if(rowCount == 0 || columnCount == 0)
    {
        return;
    }
//    ui->tableWidget_summary_menu->setColumnCount(columnCount);

    initHorizontalHeadview();

    for(int row = 0;row<rowCount;row++)
    {
        //由于隐藏了表格自身的表头，并且使用表格的第0行作为表头
        //因此每增加一行内容，表格的行数应增加1
        int rowCountTemp = row%m_properRowNum+2;
        ui->tableWidget_summary_menu->setRowCount(rowCountTemp);

        for(int column = 0; column < columnCount; column++)
        {
            //获取参数tableWidget表格的数据
//            QTableWidgetItem *item = sqlTableModel->item(row,column);
            QVariant variant = sqlTableModel->index(row, column).data();
            if(variant.isNull())
            {
                continue;
            }
            QString text = variant.toString();

            //将获取到的数据赋值到打印的表格
            QTableWidgetItem *item = ui->tableWidget_summary_menu->item(rowCountTemp-1,column);
            if(item == 0)
            {
                item = new QTableWidgetItem();
                ui->tableWidget_summary_menu->setItem(rowCountTemp-1,column,item);
            }
            item->setText(text);
        }

        //一页最多能够显示m_propercolumnNum行，此时截图保存
        //然后从第一行开始填充数据
        if((row+1)%m_properRowNum == 0)
        {
            ui->label_summary_menu_page_umber->setText(
                        tr("第 %1 页").arg(m_pixmapList.count()+1));
            m_pixmapList<<this->grab();

            initHorizontalHeadview();
        }
    }
    ui->label_summary_menu_page_umber->setText(
                tr("第 %1 页").arg(m_pixmapList.count()+1));
    m_pixmapList<<this->grab();

    print(QPrinter::Landscape);
    MY_LOG("end");
}

void Print::saveAsPDF(QList<QPixmap> pixmapList,
                        QString file_name,
                        QPrinter::Orientation orientation)
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPrinter::A4);  //设置纸张大小为A4
    printer.setOutputFormat(QPrinter::PdfFormat);  //设置输出格式为pdf
    printer.setOrientation(orientation);

//    QString fileName = QFileDialog::getSaveFileName(this, tr("导入气体浓度值"),"",
//                                                    tr("PDF files (*.pdf)"));

    printer.setOutputFileName(file_name);   //设置输出路径

    QPainter painter(&printer);

    for(int i=0; i<pixmapList.count(); i++)
    {
        if(i > 0)
        {
           printer.newPage();
        }
        QPixmap pixmap=pixmapList.at(i);
        QRect rect = painter.viewport();
        QSize size = pixmap.size();

        size.scale(rect.size(), Qt::KeepAspectRatio); //此处保证图片显示完整
        painter.setViewport(rect.x(), rect.y(),size.width(), size.height());
        painter.setWindow(pixmap.rect());

        painter.drawPixmap(0,0,pixmap); /* 数据显示至预览界面 */
    }

    painter.end();
}

void Print::print(QPrinter::Orientation orientation)
{
    MY_LOG("start");
    int result = QDialog::Accepted;
    if(QSysInfo::WV_XP == QSysInfo::windowsVersion())
    {
        QString tmp_result = "tmp_result.pdf";
        saveAsPDF(this->m_pixmapList, tmp_result, orientation);
        const wchar_t *action = L"print";
        const wchar_t *file = L"tmp_result.pdf";
        ShellExecute(0, action, file, NULL, NULL, SW_HIDE);//打印
        MyMessageDialog msg_dialog;
        msg_dialog.setVisible_CancelButton(false);
        QTimer::singleShot(1800, &msg_dialog, SLOT(accept()));
        msg_dialog.setMessage("使用默认打印机，打印中...");
        msg_dialog.exec();

    }
    else
    {
        MY_LOG("");

        QPrinter printer(QPrinter::HighResolution);

        QPrintPreviewDialog preview(&printer, 0); /* 打印预览 */
        Qt::WindowFlags flags = preview.windowFlags();
        flags |= Qt::WindowMaximizeButtonHint;
        preview.setWindowFlags(preview.windowFlags());

        MY_LOG("");

        //更改打印预览窗口按钮自带提示文字(英->中)
        QList<QObject*> objectList = preview.findChildren<QObject *>();
        foreach(QObject *object,objectList)
        {
            if(object->isWidgetType())
            {
                QWidget *w = qobject_cast<QWidget *>(object);
                QString text = w->toolTip();
                if(text == "Print")
                {
                    w->setToolTip(tr("打印"));
                }
                else if(text == "Page setup")
                {
                    w->setToolTip(tr("页面设置"));
                }
//                qDebug()<<"w->toolTip() = "<<w->toolTip();
            }
        }

        MY_LOG("");
        printer.setOrientation(orientation);
        int marginH = 20,marginV = 11;
        printer.setPageMargins(marginH,marginV,marginH,11,QPrinter::Millimeter);

        MY_LOG("");
        /**
         * QPrintPreviewDialog类提供了一个打印预览对话框，里面功能比较全，
         * paintRequested(QPrinter *printer)是系统提供的，
         * 当preview.exec()执行时该信号被触发，
         * plotPic(QPrinter *printer)是用户自定义的槽函数，图像的绘制就在这个函数里。
         */
        connect(&preview, SIGNAL(paintRequested(QPrinter *)),
                this, SLOT(print(QPrinter *))); //其实会立即执行，先绘制图片给用户预览并等待确定是否打印

        MY_LOG("");
        preview.show();
        preview.showMaximized();
        result = preview.exec(); /* 等待预览界面退出 */

        MY_LOG("");
        qDebug()<<"result = "<<result;
    }
    MY_LOG("end");
}

void Print::print(QPrinter *printer)
{
    MY_LOG("start");
    g_printMultipage(printer, m_pixmapList);
    MY_LOG("end");
}

void Print::printSingleItem(QList<PropertyAndVariant> testResultList)
{
    MY_LOG("start");
    QSize properSize(726,1130);//合适的尺寸
    this->resize(properSize);
    ui->stackedWidget->setCurrentIndex(1);

    //显示图片
    QByteArray data = getVariant(testResultList,PROPERTY_PIC).toByteArray();
    qDebug()<<"data.count() = "<<data.count();
    QPixmap pixmap;
    pixmap.loadFromData(data);
    qDebug()<<"pixmap.isNull() = "<<pixmap.isNull();
    if(pixmap.isNull())
    {
        pixmap = QPixmap(ui->label_car_pic->size());
        pixmap.fill(Qt::white);
    }
    else
    {
        pixmap = pixmap.scaled(ui->label_car_pic->size());
    }
    ui->label_car_pic->setStyleSheet("");
    ui->label_car_pic->setPixmap(pixmap);

    //显示车牌号
    QString text = getVariant(testResultList,PROPERTY_lICENCE).toString();
    ui->label_license->setText(text);

    //显示车牌颜色
    text = getVariant(testResultList,PROPERTY_BG_COLOR).toString();
    ui->label_license_color->setText(text);

    //显示检测地点
    text = getVariant(testResultList,PROPERTY_SITE_ID).toString();
    ui->label_test_site->setText(text);

    //显示环境参数
    text = tr("温度:")+getVariant(testResultList,PROPERTY_OUT_DOOR_TEMP).toString();
    text += tr("  相对湿度:")+getVariant(testResultList,PROPERTY_HUMIDITY).toString();
    text += tr("  风速:")+getVariant(testResultList,PROPERTY_WIND_SPEED).toString();
    text += tr("  风向:")+getVariant(testResultList,PROPERTY_WIND_DIRECTION).toString();
    ui->label_environment_parameters->setText(text);

    //显示检测时间
    text = getVariant(testResultList,PROPERTY_PASS_TIME).toString();
    ui->label_test_date->setText(text);

    //显示CO浓度
    text = getVariant(testResultList,PROPERTY_C_CO).toString();
    ui->label_CO_result->setText(text);

    //显示HC浓度
    text = getVariant(testResultList,PROPERTY_C_HC).toString();
    ui->label_HC_result->setText(text);

    //显示NO浓度
    text = getVariant(testResultList,PROPERTY_C_NO).toString();
    ui->label_NO_result->setText(text);

    //显示黑度
    text = getVariant(testResultList,PROPERTY_OPACITY).toString();
    ui->label_opacity_result->setText(text);

    m_pixmapList.clear();
    m_pixmapList<<this->grab();

    print(QPrinter::Portrait);
    MY_LOG("end");
}


QVariant Print::getVariant(QList<PropertyAndVariant> testResultList,
                               QString property)
{
    foreach(PropertyAndVariant data,testResultList)
    {
        if(property == data.m_property)
        {
            return data.m_variant;
        }
    }
    return "--";
}




