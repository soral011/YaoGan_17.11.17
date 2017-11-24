#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QCameraInfo>
#include <QPropertyAnimation>
#include <QTimer>
#include <QCloseEvent>
#include <QParallelAnimationGroup>
#include <QDateTime>
#include <QCursor>
#include <QDir>
#include <QTextCodec>
#include <QDesktopWidget>
#include "MyDebug.cpp"

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TestWindow)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
//    this->showMaximized();

    m_adjustCamera = AdjustCamera::getInstance();
    m_operationFlow = new OperationFlow;
    m_led = new led_control;

    m_camera = Camera::getInstance();
    m_systemStatus = SystemStatus::getInstance();

//    m_caliDialog = CaliDialog::getInstance();

    ui->stackedWidget->setCurrentWidget(ui->page_connection_status);
    m_connectionStatus.setParent(ui->widget_connection_status);
    m_connectionStatus.resize(ui->widget_connection_status->size());
    m_connectionStatus.move(0, 0);

    ui->checkBox_is_save_test_result->hide();
    ui->pushButton_create_one_test_result->hide();

    m_testResults = NULL;
    m_testerConnected = false;
    m_cameraConnected = false;

    //用于菜单按钮
    this->setMouseTracking(true);
    QList <QWidget *> w_list = this->findChildren<QWidget *>();
    foreach(QWidget *w,w_list)
    {
        w->setMouseTracking(true);
    }

    connect(&m_connectionStatus, SIGNAL(deviceConnection(bool)),
            this, SLOT(deviceConnection(bool)));

    connect(m_camera,SIGNAL(loginOk(bool)),
            this,SLOT(loginFinished(bool)));

    ui->label_vertical_spacer->hide();

    g_connectButtonSignal(this);


    m_selectedStyle = ui->pushButton_settings->styleSheet();
    m_notSelectedStyle = ui->pushButton_tools->styleSheet();
    ui->pushButton_settings->setStyleSheet(m_notSelectedStyle);

    initMenu();

    m_propertyList = g_propertyList;
    m_propertyList.insert(0,PropertyAndText("序号",		PROPERTY_SEQUENCE_NUMBER));

    ui->tableWidget_test_result_list->clear();

    ui->tableWidget_test_result_list->setRowCount(0);

    //+1用于记录保存文件的名称，当单击该行时，读取相应的文件，显示相应的图片
    //同时隐藏该行
    ui->tableWidget_test_result_list->setColumnCount(m_propertyList.count()+1);
    ui->tableWidget_test_result_list->hideColumn(m_propertyList.count());

    qDebug()<<"b m_propertyList.count() = "<<m_propertyList.count();

//    for(int column = 0; column < m_propertyList.count(); column++)
//    {

//        QTableWidgetItem *item;

//        item = ui->tableWidget_test_result_list->horizontalHeaderItem(column);
//        if(item == 0)
//        {
//            item = new QTableWidgetItem();
//            ui->tableWidget_test_result_list->setHorizontalHeaderItem(column,item);
//        }
//        item->setText(m_propertyList.at(column).m_text);
//        if(m_propertyList.at(column).m_property == PROPERTY_PASS_TIME)
//        {
//            ui->tableWidget_test_result_list->horizontalHeader()
//                    ->resizeSection(column,160);
//        }
//        else if(m_propertyList.at(column).m_property == PROPERTY_lICENCE)
//        {
//            ui->tableWidget_test_result_list->horizontalHeader()
//                    ->resizeSection(column,79);
//        }
//        else
//        {
//            //自动切换字体大小
//            QFontMetrics font_metrics = ui->tableWidget_test_result_list
//                    ->horizontalHeader()->fontMetrics();

//            int width = font_metrics.width(m_propertyList.at(column).m_text);
//            ui->tableWidget_test_result_list->horizontalHeader()
//                    ->resizeSection(column,width+28);
//        }
//    }
    g_clearPushButtonFocus(this);

}

MainWindow::~MainWindow()
{
    MY_DEBUG("~MainWindow()");

    delete m_operationFlow;
    delete m_helpMenu;
    delete m_manageMenu;
    delete m_toolsMenu;
    delete m_led;

    //不要删除空指针，有可能造成释放资源失败
    if(m_testResults !=  NULL)
    {
        delete m_testResults;
    }

    delete ui;

    MY_DEBUG("~MainWindow() end");
}

void MainWindow::buttonReleased(QWidget *w)
{
    QString buttonName = w->objectName();

    if(buttonName == "pushButton_calibrate")
    {
        if(m_testerConnected == false)
        {
            g_showTipsDialog(tr("测量主机未连接！"));
            return;
        }

        m_caliDialog.exec();
    }
    else if(buttonName == "pushButton_settings")
    {
        m_settingsDialog.close();
        g_showWithAnim(&m_settingsDialog);
    }
    else if(buttonName == "pushButton_start_test")
    {
        //删除后重新赋值，否则m_operationFlow关闭后重新打开，无法获取焦点，导致输入框无法输入
        delete m_operationFlow;
        m_operationFlow = new OperationFlow;
        connect(m_operationFlow, SIGNAL(operationFinished(bool)),
                this, SLOT(operationFlowFinished(bool)));
//        m_operationFlow->setWindowModality(Qt::ApplicationModal);//设置阻塞类型
        m_operationFlow->showMaximized();
    }
    else if(buttonName == "pushButton_minimize")
    {
        this->showMinimized();
    }
    else if(buttonName == "pushButton_close")
    {
        bool isOk = g_showAskingDialog(tr("确定要退出吗？"), this);
        if(isOk)
        {
            this->close();
        }
    }
    else if(buttonName == "pushButton_create_one_test_result")
    {
        AccessDatabase accessDatabase;

        QList<PropertyAndVariant> testResultList;
        QString nowTime = g_getNowTime();
        testResultList.append(PropertyAndVariant(PROPERTY_PASS_TIME, nowTime));

        QPixmap pixmap = Camera::getInstance()->capturePic();
        testResultList.append(PropertyAndVariant(PROPERTY_PIC, pixmap));

        int HC_C = g_getRandomNum(150);
        testResultList.append(PropertyAndVariant(PROPERTY_C_HC, HC_C));

        QString userID = AddUserDialog::getUserMD5(g_currentUser);
        testResultList.append(PropertyAndVariant(PROPERTY_USER_ID, userID));

        QString siteID = AddSite::getSiteMD5(g_currentSite);
        testResultList.append(PropertyAndVariant(PROPERTY_SITE_ID, siteID));

        accessDatabase.saveTestResult(testResultList);
        qApp->processEvents();
        g_msleep(1000);
    }
    else if(buttonName == "pushButton_main_menu")
    {
        bool isOk = g_isClickTimesOK(3);
        if(isOk)
        {
            g_isDebugMode = true;
            g_showTipsDialog(tr("已经处于调试模式！"),
                                 this); // 这里一定要加上 this 参数，否是程序可能崩溃
            delete m_helpMenu;
            delete m_manageMenu;
            delete m_toolsMenu;

            this->initMenu();
        }
    }
}

void MainWindow::initMenu()
{
    //工具
    QList<MyMenuItem> menuItems;
    menuItems << MyMenuItem(MENU_ITEM_SPECTRUM,
                          QIcon(":/images/spectrum.png"));
    menuItems << MyMenuItem(MENU_ITEM_SYSTEM_STATUS,
                          QIcon(":/images/system_status2.png"));
    menuItems << MyMenuItem(MENU_ITEM_CONTROL_PANEL, QIcon());
    if(g_isDebugMode)
    {
        menuItems << MyMenuItem(MENU_ITEM_REAL_TIME_TEST, QIcon());
    }

    m_toolsMenu = new MyMenu(menuItems);

    //管理
    menuItems.clear();
    menuItems<<MyMenuItem(MENU_ITEM_VIEW_RESULTS,
                          QIcon(":/images/time_machine_shaped.png"))
             <<MyMenuItem(MENU_ITEM_USER_MANAGE, QIcon())
             <<MyMenuItem(MENU_ITEM_TEST_SITE, QIcon());
    m_manageMenu = new MyMenu(menuItems);

    //帮助
    QStringList list;
    list<<MENU_ITEM_UPDATE_ARM_SOFTWARE
        <<MENU_ITEM_ABOUT;
    m_helpMenu = new MyMenu(list);

    //绑定槽
    QList<MyMenu *> menuList;
    menuList<<m_helpMenu
            <<m_manageMenu
            <<m_toolsMenu;
    foreach(MyMenu *menu, menuList)
    {
        connect(menu, SIGNAL(itemClicked(QString))
                , this, SLOT(menuItemClicked(QString)));
        menu->setParent(this);
        menu->hide();
    }
}

void MainWindow::menuItemClicked(QString item)
{
    qDebug()<<"item = "<<QTextCodec::codecForLocale()
              ->toUnicode(item.toUtf8());
    hideButtonMenu();
    if(item == MENU_ITEM_BASIC ||
        item == MENU_ITEM_PREFERENCE ||
        item == MENU_ITEM_OTHER)
    {
        m_settingsDialog.close();
        g_showWithAnim(&m_settingsDialog);
//        hideButtonMenu();
    }
    else if(item == MENU_ITEM_QUIT)
    {
        this->close();
    }
    else if(item == MENU_ITEM_VIEW_RESULTS)
    {
        if(m_testResults == NULL)
        {
            m_testResults = new Results();
        }
        m_testResults->close();
//        QTimer::singleShot(100,m_testResults,SLOT(show()));
        m_testResults->showMaximized();
        g_showWithAnim(m_testResults);

    }
    else if(item == MENU_ITEM_CONTROL_PANEL)
    {
        m_controlPanel.close();
        m_controlPanel.show();
        m_controlPanel.activateWindow();

        g_showWithAnim(&m_controlPanel);
    }
    else if(item == MENU_ITEM_SPECTRUM)
    {
        m_spectrum.close();
        m_spectrum.showMaximized();
        g_showWithAnim(&m_spectrum);
    }
    else if(item == MENU_ITEM_REAL_TIME_TEST)
    {
        m_realTimeTest.close();
        m_realTimeTest.showMaximized();
    }
    else if(item == MENU_ITEM_MANUAL_CAPTURE_FULL ||
            item == MENU_ITEM_MANUAL_CAPTURE_VEDIO)
    {
        g_createPath(CAPTURE_PATH);

        QLabel *label = new QLabel();
        label->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint
                                       |Qt::Tool/*|Qt::SubWindow*/);

        g_msleep(30);//防止菜单被截取
        QPixmap fullScreenPix,pixmapToSave;

        QPixmap pixmap(this->size());
        pixmap.fill(Qt::white);

        if(item == MENU_ITEM_MANUAL_CAPTURE_FULL)
        {
            fullScreenPix = QPixmap::grabWindow(QApplication::desktop()->winId());
            pixmapToSave = fullScreenPix.copy(this->rect());
            label->resize(this->size());

            label->setPixmap(pixmap);
            label->move(0,0);
        }
        else
        {
            QPoint pos = ui->widget_show_vedio_parent->mapToGlobal(QPoint(0,0));

            QCursor cursor;
            cursor.setPos(QPoint(pos.x()+100,pos.y()+100));
            g_msleep(30);

            fullScreenPix = QPixmap::grabWindow(QApplication::desktop()->winId());
            QRect rect = ui->widget_show_vedio_parent->rect();

            pixmapToSave = fullScreenPix.copy(pos.x(),pos.y(),rect.width(),rect.height());

            label->resize(ui->widget_show_vedio_parent->size());

            label->setPixmap(pixmap);
            label->move(ui->widget_show_vedio_parent->mapToGlobal(QPoint(0,0)));
        }
        QString passTime = QDateTime::currentDateTime().toString("yyyy-MM-dd_hhmmss");
        pixmapToSave.save(CAPTURE_PATH+passTime+".jpg","jpg",100);

        label->show();
        QPropertyAnimation *anim = new QPropertyAnimation(label,"windowOpacity");
        anim->setEasingCurve(QEasingCurve::InQuart);
        anim->setDuration(800);
        anim->setStartValue(1.0);
        anim->setEndValue(0.5);
        anim->start();
        connect(anim,SIGNAL(finished()),label,SLOT(deleteLater()));
    }
    else if(item == MENU_ITEM_ABOUT)
    {
        emit g_mySignal.showTips(QString(MENU_ITEM_ABOUT)+"!");
        static About *aboutWidget = NULL;
        if(aboutWidget == NULL)
        {
            aboutWidget = new About();
            connect(this,SIGNAL(destroyed()),aboutWidget,SLOT(deleteLater()));
        }
        aboutWidget->close();
        aboutWidget->show();
    }
    else if(item == MENU_ITEM_SYSTEM_STATUS)
    {
        this->m_systemStatus->close();
        g_showWithAnim(m_systemStatus);
    }
    else if(item == MENU_ITEM_UPDATE_ARM_SOFTWARE)
    {
        if(m_systemStatus->m_isTestHostConnected == false)
        {
            g_showTipsDialog(tr("测量主机未连接！"), this);
            return;
        }
        UpdateSoftwareARM updateSoftwareARM;
        updateSoftwareARM.exec();
    }
    else if(item == MENU_ITEM_USER_MANAGE)
    {
        UserManagement userManagement;
        userManagement.exec();
    }
    else if(item == MENU_ITEM_TEST_SITE)
    {
        SiteManagement siteManagement;
        siteManagement.exec();
    }
}

void MainWindow::hideButtonMenu()
{
    ui->pushButton_settings->setStyleSheet(this->m_notSelectedStyle);
    ui->pushButton_tools->setStyleSheet(this->m_notSelectedStyle);
    ui->pushButton_manage->setStyleSheet(this->m_notSelectedStyle);
    ui->pushButton_help->setStyleSheet(this->m_notSelectedStyle);
    ui->pushButton_calibrate->setStyleSheet(this->m_notSelectedStyle);
    m_helpMenu->hide();
    m_toolsMenu->hide();
//    m_settingsMenu->hide();
    m_manageMenu->hide();
}

void MainWindow::closeWithAnimation()
{
    this->setMinimumHeight(0); //因为窗口之前被设置为固定大小

    int duration = 400;
    QEasingCurve esasing_curve = QEasingCurve::OutQuad;
    QPropertyAnimation *rect_animation =
            new QPropertyAnimation(this,"size");//使用geometry效果不佳
    rect_animation->setDuration(duration);
    rect_animation->setEndValue(QSize(/*this->pos().x(),this->pos().y()+this->height()/2,*/
                                      this->width(),10));
    rect_animation->setEasingCurve(esasing_curve);

    QPropertyAnimation *pos_animation = new QPropertyAnimation(this,"pos");
    pos_animation->setDuration(duration);
    pos_animation->setEndValue(QPoint(this->pos().x()
                                      ,this->pos().y()+this->height()/2));
    pos_animation->setEasingCurve(esasing_curve);

    QPropertyAnimation *opacity_animation = new QPropertyAnimation(this,"windowOpacity");
    opacity_animation->setDuration(duration);
//    opacity_animation->setStartValue(0.8);
    opacity_animation->setEndValue(0.35);
    opacity_animation->setEasingCurve(esasing_curve);


    QParallelAnimationGroup *group = new QParallelAnimationGroup();
    group->addAnimation(pos_animation);
    group->addAnimation(rect_animation);
    group->addAnimation(opacity_animation);

    connect(group, SIGNAL(finished()),group, SLOT(deleteLater()));
    connect(group, SIGNAL(finished()),rect_animation, SLOT(deleteLater()));
    connect(group, SIGNAL(finished()),pos_animation, SLOT(deleteLater()));
    connect(group, SIGNAL(finished()),opacity_animation, SLOT(deleteLater()));
    connect(group, SIGNAL(finished()),this, SLOT(hide()));

    connect(group,SIGNAL(finished()),qApp,SLOT(quit())); //退出app，并释放资源

    group->start();
}

void MainWindow::mousePressEvent(QMouseEvent * event)
{
//    qDebug()<<"mousePressEvent !";
    hideButtonMenu();
}

int MainWindow::getColumnPosition(QString property)
{
    int count = m_propertyList.count();
    for(int i = 0;i<count;i++)
    {
        if(m_propertyList.at(i).m_property == property)
        {
            return i;
        }
    }
    MY_DEBUG("property ="<<property);
    return 0;
}

void MainWindow::switchCarPic(QPixmap carPixmap)
{
//    //使用属性动画来切换图片
//    QSize car_size = ui->label_show_capture_img->size();
//    QPixmap pixmap(car_size.width()*2, car_size.height());
//    QPainter painter(&pixmap);
//    bool is_on_right = true;
//    if(is_on_right)
//    {
//        painter.drawPixmap(0, 0, ui->label_show_capture_img->grab());
//        painter.drawPixmap(car_size.width(), 0, carPixmap.scaled(car_size));
//    }
//    else
//    {
//        painter.drawPixmap(0, 0, carPixmap.scaled(car_size));
//        painter.drawPixmap(car_size.width(), 0, ui->label_show_capture_img->grab());
//    }

//    painter.end();

//    QLabel *label = new QLabel(ui->label_show_capture_img);
//    label->resize(car_size.width()*2, car_size.height());
////    label->move(0,0);
//    label->setPixmap(pixmap);
//    label->show();
////    qDebug()<<"pixmap.size() = "<<pixmap.size();
////    qDebug()<<"label.size() = "<<label->size();
//    QPropertyAnimation *anim = new QPropertyAnimation(label,"pos");
//    anim->setDuration(380);
//    anim->setEasingCurve(QEasingCurve::OutQuad);
//    if(is_on_right)
//    {
//        anim->setStartValue(QPoint(-car_size.width()/2,0));
//        anim->setEndValue(QPoint(-car_size.width(),0));
//    }
//    else
//    {
//        anim->setStartValue(QPoint(-car_size.width()/2,0));
//        anim->setEndValue(QPoint(0,0));
//    }
//    anim->start();
//    connect(anim, SIGNAL(finished()), label, SLOT(deleteLater()));


    ui->label_show_capture_img->setPixmap(carPixmap);
//    m_pixmap.save("test.jpg","JPG");//保存图片知本身存在黑边
    ui->label_show_capture_img->update();
}

//在状态栏右键点击关闭、在窗口菜单栏点击“退出”、点击右上角“叉号”都会调用此函数
void MainWindow::closeEvent(QCloseEvent * event)
{
    if(QSysInfo::WV_XP == QSysInfo::windowsVersion())
    {
        qApp->quit();
    }
    else
    {
        event->ignore();

        QTimer::singleShot(20,this,SLOT(closeWithAnimation()));
    }
}

//用于隐藏或者显示控制云台按钮
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
//    qDebug()<<event->pos();

    //鼠标移至界面左上角菜单栏时，显示下拉菜单；移开时隐藏
    bool isMouseOnMenuButton = false;
    QMap<QPushButton *,MyMenu *> buttonAndMenuMap;
    buttonAndMenuMap.insert(ui->pushButton_manage, m_manageMenu);
    buttonAndMenuMap.insert(ui->pushButton_tools, m_toolsMenu);
    buttonAndMenuMap.insert(ui->pushButton_help, m_helpMenu);
    QList<QPushButton *> pushButtonList = buttonAndMenuMap.keys();
    QList<MyMenu *> menuWidgetList = buttonAndMenuMap.values();
    foreach(QPushButton *button, pushButtonList)
    {
        QPoint pos = button->mapToGlobal(QPoint(0,0));
        int x = event->pos().x()-pos.x();
        int y = event->pos().y()-pos.y();
        QRect rect = button->rect();
        if(rect.contains(QPoint(x,y)))
        {
            isMouseOnMenuButton = true;
            MyMenu *menu = buttonAndMenuMap.value(button,NULL);
            if(menu!= NULL && menu->isVisible() == false)
            {
                this->hideButtonMenu();
                QPoint pos = button->pos();
                pos.setY(pos.y()+button->height());
                menu->move(pos);
                menu->show();
                menu->raise(); // 显示到最顶层
                button->setStyleSheet(m_selectedStyle);
            }
            break;
        }
    }
    foreach(MyMenu *menu, menuWidgetList)
    {
        if(menu->isVisible() && isMouseOnMenuButton == false)
        {
            QPoint pos = menu->mapToGlobal(QPoint(0,0));
            int x = event->pos().x()-pos.x();
            int y = event->pos().y()-pos.y();
            QRect rect = menu->rect();
            if(rect.contains(QPoint(x,y)) == false)
            {
                this->hideButtonMenu();
            }
            break;
        }
    }
}

void MainWindow::on_tableWidget_test_result_list_itemClicked(QTableWidgetItem *item)
{
//    int row = item->row();

//    item = ui->tableWidget_test_result_list->item(row,
//                           ui->tableWidget_test_result_list->columnCount()-1);
////    qDebug()<<"item = "<<item->text();
//    if(item == 0)
//        return;

//    QFile file(item->text());
//    if(file.open(QFile::ReadOnly) == false)
//    {
//        qDebug()<<"open file for reading failed!";
//        return;
//    }

//    QMap<QString,QVariant> resultMap;

//    QDataStream out(&file);
//    out.setVersion(QDataStream::Qt_4_7);
//    out>>resultMap;

//    if(resultMap.count() == 0)
//        return;

//    QPixmap pixmap;
//    out>>pixmap;

//    ui->label_car_licence->setText(resultMap.value(PROPERTY_lICENCE).toString());
////    ui->label_car_licence_color->setText(resultMap.value(PROPERTY_BG_COLOR).toString());
////    ui->label_entire_believability->setText(resultMap.value(PROPERTY_RELIABILITY).toString());

//    if(pixmap.isNull() == false)
//    {
//        pixmap = pixmap.scaled(ui->label_show_capture_img->size());
//        QPainter painter(&pixmap);
//        QPixmap tmpPixmap;
//        out>>tmpPixmap;
//        tmpPixmap = tmpPixmap.scaled(70, 35/*tmpPixmap.width()/2,tmpPixmap.height()/2*/);
//        painter.drawPixmap(0,0,tmpPixmap);
//        QPen pen(Qt::gray);
//        pen.setWidth(1);
//        painter.setPen(pen);
//        painter.drawRect(tmpPixmap.rect());
//        painter.end();
//        switchCarPic(pixmap);
////        ui->label_show_capture_img->setPixmap(pixmap);
//    }
}

void MainWindow::loginFinished(bool value)
{
//    qDebug()<<"value = "<<value;
    m_cameraConnected = value;
}

void MainWindow::updateWeather(WeatherReport data)
{
    ui->label_wind_speed->setText(     tr("风速：") + data.windSpeed + "m/s");
    ui->label_wind_direction->setText( tr("风向：") + data.windDirection + tr("度"));
    ui->label_temprature->setText(     tr("温度：") + data.temperature + "℃");
    ui->label_humidity->setText(       tr("湿度：") + data.humidity + "%");
    ui->label_atmosphere->setText(     tr("大气压：") + data.atmosphere + "hPa");
}

void MainWindow::updateTestResultText()
{
    QString carLicense = m_captureResults.license;
    QString carLicenseColor = m_captureResults.licenseColor;
    int byEntireBelievability = m_captureResults.believability;

    ui->label_car_licence->setText(carLicense);
//    ui->label_car_licence_color->setText(carLicenseColor);
//    ui->label_entire_believability->setText(QString().setNum(byEntireBelievability));

    g_showLabelTextWithAnim(ui->label_car_licence,
                            ui->label_car_licence->text(),false);
//    g_showLabelTextWithAnim(ui->label_car_licence_color,
//                            ui->label_car_licence_color->text());
//    g_showLabelTextWithAnim(ui->label_entire_believability,
//                            ui->label_entire_believability->text(),false);

    //速度
    NetSpeedResults speedResults = m_carTestResults.exhaustResults.speedResults;
    g_showLabelTextWithAnim(ui->label_car_speed,
                            QString().setNum(speedResults.V, 'f', 2) + "（km/h）");
//    g_showLabelTextWithAnim(ui->label_car_acceleration,
//                            QString().setNum(speedResults.a, 'f', 2) + "（m/s^2）" , false);

    //HC、NO气体浓度
    NetDOASTestResults doasTestResults = m_carTestResults.exhaustResults.doasTestResults;
    g_showLabelTextWithAnim(ui->label_result_NO,
                            QString().setNum(doasTestResults.NO_C,'f',0));
    g_showLabelTextWithAnim(ui->label_result_HC,
                            QString().setNum(doasTestResults.HC_C,'f',0));

    //CO、CO2气体浓度
    NetTDLASTestResults tdlasTestResults = m_carTestResults.exhaustResults.tdlasTestResults;
    g_showLabelTextWithAnim(ui->label_result_CO2,
                            QString().setNum(tdlasTestResults.CO2_C, 'f', 2));
    g_showLabelTextWithAnim(ui->label_result_CO,
                            QString().setNum(tdlasTestResults.CO_C, 'f', 2));

    //黑度
    NetOpacityResults opacityResults = m_carTestResults.exhaustResults.opacityResults;
    g_showLabelTextWithAnim(ui->label_result_opacity,
                            QString().setNum(opacityResults.opacity, 'f', 2));

    //通过时间
    QString passTime = QDateTime::currentDateTime().toString("hh:mm:ss");
    g_showLabelTextWithAnim(ui->label_result_passtime, passTime);

    //检测状态
    QString statusText;
    if(m_carTestResults.statusText == tr("合格"))
    {
        statusText = "<font color = #00aaff>合格</font>";
    }
    else if(m_carTestResults.statusText == tr("超标"))
    {
        statusText = "<font color = #ff0000>超标 </font>";
    }
    else
    {
        statusText = "<font color = #ffff00>无效</font>";
    }
    g_showLabelTextWithAnim(ui->label_car_state, statusText);

    MY_DEBUG("show results finished!"<<::g_getNowTime());
}

void MainWindow::showTipsWidget()
{
//    QPoint pos = ui->widget_tips->mapToGlobal(QPoint(0,0));

//    int x = QCursor::pos().x()-pos.x();
//    int y = QCursor::pos().y()-pos.y();
//    QRect rectTips = ui->widget_tips->rect();
//    if(rectTips.contains(QPoint(x,y))&& x<70)
//    {
//        QSize size = ui->gridWidget_gas_intensity->size();
//        if(m_runningStatus.size()!= size)
//        {
//            m_runningStatus.resize(size);
//        }
//        QPoint newPos = QPoint(0,pos.y()-m_runningStatus.height());
////        m_showTipsWidget.move(newPos);

//        if(m_runningStatus.isVisible() == false)
//        {
//            m_runningStatus.show();
//            QPropertyAnimation *anim = new QPropertyAnimation(&m_runningStatus,"pos");
//            anim->setDuration(280);
//            anim->setEasingCurve(QEasingCurve::OutQuad);
//            anim->setStartValue(QPoint(newPos.x(),newPos.y()-50));
//            anim->setEndValue(newPos);
//            anim->start();
//            QObject::connect(anim,SIGNAL(finished()),anim,SLOT(deleteLater()));
//            QObject::connect(anim,SIGNAL(finished()),this,SLOT(isTimeToHide()));
//        }

//    }
}

void MainWindow::receiveCaptureInfo(CaptureResults captureResults)
{
//    qDebug()<<"called receiveCaptureInfo (MY_NET_ITS_PLATE_RESULT)";
//    m_captureResults = captureResults;

//      //显示抓拍到的车牌图片（在上排中间窗口）
//    QPixmap carImg = m_captureResults.vehiclePic;
//    carImg = carImg.scaled(ui->label_show_capture_img->size());
//    switchCarPic(carImg);

//    //增加一条测量记录（在右下角窗口）
//    addOneRowTestResult();

//    //更新车辆号、合格与否等信息（在左上角窗口）
//    updateTestResultText();

//    //保存测量结果
////    saveTestResult(my_NET_ITS_PLATE_RESULT.carPic);
}

void MainWindow::receiveTestResult(CarTestResults carTestResults)
{
    m_carTestResults = carTestResults;

    m_captureResults = carTestResults.captureResults;

    //显示抓拍图片
    QPixmap tmpImg = m_captureResults.vehiclePic;
    QPixmap carImg = m_captureResults.vehiclePic;
    if(m_captureResults.license != "无法识别" && m_captureResults.license != "无车牌")
        carImg = m_camera->processPixmap(tmpImg,m_captureResults);
    carImg = carImg.scaled(ui->label_show_capture_img->size());
    switchCarPic(carImg);

    //新增一条检测记录
    addOneRowTestResult();

    //更新车牌号、合格与否等信息
    updateTestResultText();

//    qApp->processEvents();
}

void MainWindow::addOneRowTestResult()
{
    //重新初始化表头
    static bool result_list_init_flag = true;
    if(result_list_init_flag){
        result_list_init_flag = false;

        QList <QString>     HeaderString;
        HeaderString.append("序号");
        HeaderString.append("状态");
        HeaderString.append("通过时间");
        HeaderString.append("车牌");
        HeaderString.append("颜色");
        HeaderString.append("CO浓度");
        HeaderString.append("CO2浓度");
        HeaderString.append("NO浓度");
        HeaderString.append("HC浓度");
        HeaderString.append("黑度");
        HeaderString.append("最大不透光度");
        HeaderString.append("平均不透光度");
        HeaderString.append("VSP");
        HeaderString.append("速度");
        HeaderString.append("加速度");
        HeaderString.append("车牌置信度");
        HeaderString.append("风速");
        HeaderString.append("风向");
        HeaderString.append("温度");

        ui->tableWidget_test_result_list->insertRow(0);

        QTableWidgetItem *item;
        for(int i = 0; i<HeaderString.count(); i++)
        {
            item = ui->tableWidget_test_result_list->item(0, i);
            if(item == 0)
            {
                item = new QTableWidgetItem();
                ui->tableWidget_test_result_list->setItem(0, i, item);
            }
            item->setText(HeaderString.at(i));
        }

    }

    //点阵屏显示
    show_result result;
    result.car_license = m_captureResults.license;
    result.isok = m_carTestResults.statusText;
    m_led->setParm("192.168.1.249",2929);
    m_led->connect_servers();
    m_led->set_playlst(result);
    m_led->sendmsg();


    int row = 1;

    QString carLicense = m_captureResults.license;
    QString carLicenseColor = m_captureResults.licenseColor;
    int byEntireBelievability = m_captureResults.believability;

    //在表格开头添加一行
    ui->tableWidget_test_result_list->insertRow(1);
    QTableWidgetItem *item;
    for(int i = 0; i<ui->tableWidget_test_result_list->columnCount(); i++)
    {
        item = ui->tableWidget_test_result_list->item(row, i);
        if(item == 0)
        {
            item = new QTableWidgetItem();
            ui->tableWidget_test_result_list->setItem(row, i, item);
        }
    }

    //序号
    int column = this->getColumnPosition(PROPERTY_SEQUENCE_NUMBER);
    item = ui->tableWidget_test_result_list->item(row, column);
    item->setText( QString().setNum(ui->tableWidget_test_result_list->rowCount()-1) );

    //状态
    column = this->getColumnPosition(PROPERTY_RECORD_STATUS);
    item = ui->tableWidget_test_result_list->item(row,column);
    item->setText(m_carTestResults.statusText);

    //通过时间
    column = this->getColumnPosition(PROPERTY_PASS_TIME);
    item = ui->tableWidget_test_result_list->item(row,column);
    QString tmp = m_carTestResults.passTime.section(" ",-1);
    item->setText(tmp);

    //车牌号
    column = this->getColumnPosition(PROPERTY_lICENCE);
    item = ui->tableWidget_test_result_list->item(row,column);
    item->setText(carLicense);

    //车牌颜色
    column = this->getColumnPosition(PROPERTY_BG_COLOR);
    item = ui->tableWidget_test_result_list->item(row,column);
    item->setText(carLicenseColor);

    //车牌置信度
    column = this->getColumnPosition(PROPERTY_RELIABILITY);
    item = ui->tableWidget_test_result_list->item(row,column);
    item->setText(QString().setNum(byEntireBelievability));

    //..速度加速度
    NetSpeedResults speedResults = m_carTestResults.exhaustResults.speedResults;
    column = this->getColumnPosition(PROPERTY_SPEED);
    item = ui->tableWidget_test_result_list->item(row,column);
    item->setText(QString().setNum(speedResults.V, 'f', 2));

    column = this->getColumnPosition(PROPERTY_ACC);
    item = ui->tableWidget_test_result_list->item(row,column);
    item->setText(QString().setNum(speedResults.a, 'f', 2));

    //..HC、NO气体浓度
    NetDOASTestResults doasTestResults = m_carTestResults.exhaustResults.doasTestResults;
    column = this->getColumnPosition(PROPERTY_C_HC);
    item = ui->tableWidget_test_result_list->item(row,column);
    item->setText(QString().setNum(doasTestResults.HC_C, 'f', 0));

    column = this->getColumnPosition(PROPERTY_C_NO);
    item = ui->tableWidget_test_result_list->item(row,column);
    item->setText(QString().setNum(doasTestResults.NO_C, 'f', 0));

    //..CO、CO2气体浓度
    NetTDLASTestResults tdlasTestResults = m_carTestResults.exhaustResults.tdlasTestResults;
    column = this->getColumnPosition(PROPERTY_C_CO);
    item = ui->tableWidget_test_result_list->item(row,column);
    item->setText(QString().setNum(tdlasTestResults.CO_C, 'f', 2));

    column = this->getColumnPosition(PROPERTY_C_CO2);
    item = ui->tableWidget_test_result_list->item(row,column);
    item->setText(QString().setNum(tdlasTestResults.CO2_C, 'f', 2));
    ui->tableWidget_test_result_list->selectRow(row);

    //最大不透光度、平均不透光度、K值、黑度
    NetOpacityResults   opacityResults = m_carTestResults.exhaustResults.opacityResults;
    column = this->getColumnPosition(PROPERTY_OPACITY_MAX);
    item = ui->tableWidget_test_result_list->item(row, column);
    item->setText(QString().setNum(opacityResults.opacityMax, 'f', 2));

    column = this->getColumnPosition(PROPERTY_OPACITY_AVG);
    item = ui->tableWidget_test_result_list->item(row, column);
    item->setText(QString().setNum(opacityResults.opacityAvg, 'f', 2));

//    column = this->getColumnPosition(PROPERTY_OPACITY_PARA); // K 值
//    item = ui->tableWidget_test_result_list->item(row, column);
//    item->setText(QString().setNum(opacityResults.K, 'f', 2));

    column = this->getColumnPosition(PROPERTY_OPACITY);
    item = ui->tableWidget_test_result_list->item(row, column);
    item->setText(QString().setNum(opacityResults.opacity, 'f', 2));

    //VSP
    column = this->getColumnPosition(PROPERTY_VSP);
    item = ui->tableWidget_test_result_list->item(row, column);
    item->setText(QString().setNum(m_carTestResults.vsp, 'f', 2));

    //风速、风向、温度
    column = this->getColumnPosition(PROPERTY_WIND_SPEED);
    item = ui->tableWidget_test_result_list->item(row, column);
    item->setText(m_carTestResults.weatherReport.windSpeed);

    column = this->getColumnPosition(PROPERTY_WIND_DIRECTION);
    item = ui->tableWidget_test_result_list->item(row, column);
    item->setText(m_carTestResults.weatherReport.windDirection);

    column = this->getColumnPosition(PROPERTY_OUT_DOOR_TEMP);
    item = ui->tableWidget_test_result_list->item(row, column);
    item->setText(m_carTestResults.weatherReport.temperature);

    if(row >= ui->tableWidget_test_result_list->rowCount())
    {
        QTimer::singleShot(30, ui->tableWidget_test_result_list, SLOT(scrollToTop()));
    }

    //动态显示增加一行信息（逐渐变高）
    int minHeight = 0;
    int defaultHeight = ui->tableWidget_test_result_list
            ->verticalHeader()->defaultSectionSize();
    ui->tableWidget_test_result_list->verticalHeader()->resizeSection(0, minHeight);
    QPropertyAnimation *anim = new QPropertyAnimation(
                ui->tableWidget_test_result_list->verticalHeader(), "minimumSectionSize");
    anim->setDuration(380);
    anim->setEasingCurve(QEasingCurve::OutQuad);

    anim->setStartValue(minHeight);
    anim->setEndValue(defaultHeight);

    anim->start();
    QObject::connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));

    //..由于改变属性"minimumSectionSize"不起作用，因此使用另外的方式
    QObject::connect(anim, SIGNAL(valueChanged(QVariant)),
                     this, SLOT(updateVerticalHeaderHeight(QVariant)));//
}

void MainWindow::updateVerticalHeaderHeight(QVariant var)
{
//    MY_DEBUG("var.toInt()="<<var.toInt());
    ui->tableWidget_test_result_list->verticalHeader()->resizeSection(0, var.toInt());
}

void MainWindow::hasClient(bool value)
{

    m_testerConnected = value;
//    m_testerConnected = true;
//    setDevicesStatusVisible();
}


void MainWindow::dataPacketReady(SocketPacket packet)
{
    if(packet.dataType == NET_DATA_TYPE_TESTER_STATUS)
    {
        NetTesterStatus testerStatus;
        testerStatus.read(packet.data);
//        MY_DEBUG("tester_workMode  = "<<testerStatus.m_data);
        m_testerWorkMode = testerStatus.m_workMode;
        if( E_WorkMode_Testing == m_testerWorkMode )
        {
//            if(this->isActiveWindow() &&
//                 ui->stackedWidget->currentWidget() != ui->page_test &&
//                 m_systemStatus->m_isCameraConnected &&
//                   m_systemStatus->m_isTestHostConnected)
//            {
//                ui->stackedWidget->setCurrentWidget(ui->page_test);
//                qApp->processEvents();
//                QTimer::singleShot(100,this,SLOT(showAdjustCamera()));
//            }
        }
    }
}

void MainWindow::on_checkBox_is_save_test_result_clicked(bool checked)
{
    g_isSavingTestResult = checked;
}

void MainWindow::showEvent(QShowEvent * event)
{
    MY_DEBUG("");
//    m_connectionStatus.resize(ui->page_connection_status->size());
//    m_connectionStatus.move(0, 0);
//    this->showAdjustCamera();
    if(ui->stackedWidget->currentWidget() == ui->page_test)
    {
       QTimer::singleShot(100,this,SLOT(showAdjustCamera()));
    }

    g_updateTesterInfo(ui->label_tester_info);
}

//操作引导流程结束
void MainWindow::operationFlowFinished(bool value)
{
    value=true;
    if(value)
    {
//        QTimer::singleShot(100,this,SLOT(showAdjustCamera()));
//        ui->stackedWidget->setCurrentWidget(ui->page_test);
        QTimer::singleShot(10, this, SLOT(updateMainWindow()));
    }
    else
    {
        ui->stackedWidget->setCurrentWidget(ui->page_start);
    }
}

void MainWindow::showAdjustCamera()
{
    m_adjustCamera->close();
    m_adjustCamera->setParent(ui->widget_show_vedio);
    m_adjustCamera->move(0, 0);
    m_adjustCamera->show();
    m_adjustCamera->raise();

    ui->stackedWidget->setCurrentWidget(ui->page_test);

    QSize size = ui->widget_show_vedio->size();
    if(m_adjustCamera->size() != size)
    {
        m_adjustCamera->resize(size);
    }
}

void MainWindow::deviceConnection(bool isConnected)
{
    MY_DEBUG("isConnected="<<isConnected);

    //延迟一点更新主界面，因为需要等待获取最新测量主机状态来判断显示什么界面
    QTimer::singleShot(1300, this, SLOT(updateMainWindow()));
//    QTimer::singleShot(2000, this, SLOT(updateMainWindow()));
}

void MainWindow::updateMainWindow()
{
    QWidget *widget = NULL;

    for(int column = 0; column < m_propertyList.count(); column++)
    {

        QTableWidgetItem *item;

        item = ui->tableWidget_test_result_list->horizontalHeaderItem(column);
        if(item == 0)
        {
            item = new QTableWidgetItem();
            ui->tableWidget_test_result_list->setHorizontalHeaderItem(column,item);
        }
        item->setText(m_propertyList.at(column).m_text);
        if(m_propertyList.at(column).m_property == PROPERTY_PASS_TIME)
        {
            ui->tableWidget_test_result_list->horizontalHeader()
                    ->resizeSection(column,160);
        }
        else if(m_propertyList.at(column).m_property == PROPERTY_lICENCE)
        {
            ui->tableWidget_test_result_list->horizontalHeader()
                    ->resizeSection(column,79);
        }
        else
        {
            //自动切换字体大小
            QFontMetrics font_metrics = ui->tableWidget_test_result_list
                    ->horizontalHeader()->fontMetrics();

            int width = font_metrics.width(m_propertyList.at(column).m_text);
            ui->tableWidget_test_result_list->horizontalHeader()
                    ->resizeSection(column,width+28);
        }
    }

    bool isConnected = m_systemStatus->m_isCameraConnected &&
                        m_systemStatus->m_isTestHostConnected;
    if(isConnected)
//    if(true)
    {
        if(m_systemStatus->m_testerStatus.m_caliRunSecs > 0
                 && !g_currentSite.m_siteName.isEmpty())
        {
            widget = ui->page_test;
            QTimer::singleShot(100,this,SLOT(showAdjustCamera()));
        }
        else
        {
            widget = ui->page_start;
        }
    }
    else
    {
        widget = ui->page_connection_status;
    }

    if(ui->stackedWidget->currentWidget() != widget && widget != NULL)
    {
        ui->stackedWidget->setCurrentWidget(widget);
    }

    g_updateTesterInfo(ui->label_tester_info);

}


