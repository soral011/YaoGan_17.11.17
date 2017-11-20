#include "OperationFlow.h"
#include "ui_OperationFlow.h"
#include "Global.h"
#include <QDesktopWidget>
#include <QTimer>
#include "MyDebug.cpp"
#include "AccessDatabase.h"

OperationFlow::OperationFlow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OperationFlow)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    m_currentPageIndex = 0;//操作流程显示给用户的第一个界面

    m_adjustCamera = AdjustCamera::getInstance();
    m_systemStatus = SystemStatus::getInstance();

//    m_caliDialog = CaliDialog::getInstance();

    //以下顺序是显示给用户的UI顺序
    FlowPage flowPage;
    flowPage.m_widget = &m_connectionStatus;
    flowPage.m_title = tr("网络连接状态");
    m_flowPageList.append(flowPage);

    flowPage.m_widget = m_adjustCamera;
    flowPage.m_title = tr("调节抓拍机");
    m_flowPageList.append(flowPage);

    flowPage.m_widget = ui->page_input_site;
    flowPage.m_title = tr("选择监测地点");
    m_flowPageList.append(flowPage);

    flowPage.m_widget = &m_caliDialog;
    flowPage.m_title = tr("标定");
    m_flowPageList.append(flowPage);

    flowPage.m_widget = ui->page_finish;
    flowPage.m_title = tr("完成");
    m_flowPageList.append(flowPage);


    Camera *camera = Camera::getInstance();
    connect(camera, SIGNAL(loginOk(bool)),
            this, SLOT(cameraStatus(bool)));
    m_isCameraConnected = camera->m_connected; //测量主机连接状态

    SocketClient *client = SocketClient::getInstance();
    connect(client, SIGNAL(hasClient(bool)),
            this, SLOT(testHostStatus(bool)));
    m_isTestHostConnected = client->isConnected();

    connect(&m_showTimer, SIGNAL(timeout()),
            this, SLOT(showOnTop()));

    connect(&m_caliDialog, SIGNAL(calibrationFinished(bool)),
            this, SLOT(calibrationFinished(bool)));

    g_connectButtonSignal(this);

    g_clearPushButtonFocus(this);
}

OperationFlow::~OperationFlow()
{
    MY_DEBUG("");
    delete ui;

    //若删除以下句子，会出现：RunTime Error！pure virtual method called
    //可能是由于被重复释放资源
//    m_caliDialog->setParent(NULL);
//    m_adjustCamera->setParent(NULL);
//    m_connectionStatus.setParent(NULL);
    MY_DEBUG("");
}

void OperationFlow::buttonReleased(QWidget *w)
{
    QString buttonName = w->objectName();

    if(buttonName == "pushButton_next")
    {
        this->showPage(m_currentPageIndex + 1);
    }
    else if(buttonName == "pushButton_previous")
    {
        if(m_currentPageIndex == 3 && m_caliDialog.isCalibrating())
        {
            g_showTipsDialog(tr("正在标定，请稍等！"));
            return;
        }
        this->showPage(m_currentPageIndex - 1);
    }
    else if(buttonName == "pushButton_add_site")
    {
        SiteManagement siteManagement;
        siteManagement.exec();
        showSite(siteManagement.m_siteList);
    }
    else if(buttonName == "pushButton_close")
    {
//        currentWidget = NULL;
//        this->close();
        m_showTimer.stop();
        g_closeWithAnim(this);
        emit operationFinished(false);

        FlowPage  flowPage = m_flowPageList.at(m_currentPageIndex);
        if(flowPage.m_widget == &m_caliDialog )
        {
            QTimer::singleShot(10, &m_caliDialog, SLOT(stopCalibration()));
        }
    }
}

void OperationFlow::showSite(QList<SiteStruct> siteList)
{
    ui->comboBox_site->clear();
    QStringList siteStrList;
    foreach(SiteStruct tmpSite, siteList)
    {
        siteStrList.append(tmpSite.m_siteName);
    }
    ui->comboBox_site->addItems(siteStrList);
}

//切换流程界面
//pageIndex : 下一个要显示的界面
void OperationFlow::showPage(int pageIndex)
{
    int index ;
    if(pageIndex < PAGE_INDEX_STATUE)
    {
        index = PAGE_INDEX_STATUE;
    }
    else if(pageIndex > m_flowPageList.count() - 1) //操作引导流程完成
    {
        m_showTimer.stop();
        g_closeWithAnim(this);
        emit operationFinished(true);
        return;
    }
    else
    {
        index = pageIndex;
    }

    if(index == PAGE_INDEX_STATUE) //连接状态
    {
        if(m_isCameraConnected && m_isTestHostConnected)
        {
            index += 1;
        }
    }

    if(index == PAGE_INDEX_CAMERA) //调节抓拍机
    {
        if(m_isCameraConnected == false)
        {
            g_showTipsDialog(tr("抓拍机未连接！"), this);
            return;
        }
        else if(m_isTestHostConnected == false)
        {
            g_showTipsDialog(tr("测量主机未连接！"), this);
            return;
        }
    }

    if(index == (PAGE_INDEX_SITE + 1)) //监测地点的下一步
    {
        if(ui->comboBox_site->count())
        {
            int index = ui->comboBox_site->currentIndex();
            SiteManagement siteManagement;
            g_currentSite = siteManagement.m_siteList.at(index);
            AccessDatabase accessDatabase;
            accessDatabase.saveCurrentSite(g_currentSite);
        }
        else
        {
            g_showTipsDialog(tr("请添加监测地点！"), this);
            return;
        }

    }


    m_currentPageIndex = index;

    m_caliDialog.close();
    m_adjustCamera->close();
    m_connectionStatus.close();
    ui->pushButton_next->setEnabled(true);
    ui->pushButton_next->setText(tr("下一步"));
    ui->pushButton_previous->show();

    FlowPage flowPage = m_flowPageList.at(m_currentPageIndex);
    QWidget *currentWidget = flowPage.m_widget;
    QString title = flowPage.m_title;
    QCursor cursor;
    SiteManagement siteManagement;
    switch(m_currentPageIndex)
    {
    case PAGE_INDEX_STATUE:
        ui->pushButton_previous->hide();
        break;

    case PAGE_INDEX_SITE:
        this->showSite(siteManagement.m_siteList);
        break;

    case PAGE_INDEX_CALI:
        if(m_systemStatus->m_testerStatus.m_caliRunSecs <= 0)
        {
           ui->pushButton_next->setEnabled(false);//标定完成才能下一步
        }
        break;

    case PAGE_INDEX_CAMERA:
        cursor.setPos(qApp->desktop()->width()/2,
                      qApp->desktop()->height()/2 + 41);//将鼠标移至抓拍机调节界面中间，显示调节牌识区
        ui->pushButton_previous->hide();
        break;

    case PAGE_INDEX_FINISH:
        ui->pushButton_next->setText(tr("完成"));
        ui->pushButton_previous->hide();
        break;
    }

    if(currentWidget == ui->page_finish || currentWidget == ui->page_input_site)
    {
        ui->stackedWidget->setCurrentWidget(currentWidget);
    }
    else
    {
        ui->stackedWidget->setCurrentWidget(ui->page_main);
        currentWidget->setParent(ui->page_main);
        currentWidget->resize(ui->page_main->size());
//        qApp->processEvents();
        currentWidget->move(0,0);
        currentWidget->show();
    }

    ui->label_title->setText(title);
}


void OperationFlow::paintEvent(QPaintEvent *event)
{
//    g_drawShadowBorder(this);
}

void OperationFlow::mousePressEvent(QMouseEvent *event)
{
    m_relativePos = this->pos()-event->globalPos();
}

void OperationFlow::mouseMoveEvent(QMouseEvent *event)
{
//    this->move(event->globalPos()+ m_relativePos);
}

void OperationFlow::showEvent(QShowEvent * event)
{
//    QTimer::singleShot(30, this, SLOT(showPage()));
    showPage(PAGE_INDEX_STATUE);
    m_showTimer.start(200);
    QTimer::singleShot(200, this, SLOT(updatetestHostInfo()));
}

void OperationFlow::cameraStatus(bool isConnected)
{
    m_isCameraConnected = isConnected;
    if(isConnected == false &&
        m_currentPageIndex != PAGE_INDEX_STATUE  &&
        m_currentPageIndex != PAGE_INDEX_CALI )
    {
        showPage(PAGE_INDEX_STATUE);
    }
}

void OperationFlow::testHostStatus(bool isConnected)
{
    m_isTestHostConnected = isConnected;
    if(isConnected == false && m_currentPageIndex != PAGE_INDEX_STATUE)
    {
        showPage(PAGE_INDEX_STATUE);
    }

    QTimer::singleShot(1000, this, SLOT(updatetestHostInfo()));
}

void OperationFlow::updatetestHostInfo()
{
    g_updateTesterInfo(ui->label_tester_info);
}

void OperationFlow::calibrationFinished(bool succeeded)
{
    if(succeeded)
    {
        ui->pushButton_next->setEnabled(true);
        showPage(PAGE_INDEX_FINISH);//显示最后一页
    }
}


void OperationFlow::closeEvent(QCloseEvent * event)
{
    m_showTimer.stop();
}

//保持操作引导流程显示在最上层
//注意：不要使用setWindowModality()来实现，否则调节抓拍机界面获取不了焦点
void OperationFlow::showOnTop()
{
//    QWidget *w = qApp->focusWidget();
//    if(w != NULL)
//    {
//        qDebug()<<"w->isAncestorOf(this)="<<w->isAncestorOf(this);
//    }

    //qApp->activeWindow() != 0当点击了其他程序时会等于零，说明用户在使用其它程序
    //qApp->modalWindow() == 0显示提示窗口时QMessageBox不为零
    //m_adjustCamera->isAdjustingCamera()正在调节抓拍机时this不是activeWindow
    if(qApp->activeWindow() != 0 && qApp->activeWindow() != this &&
        m_adjustCamera->isAdjustingCamera() == false && qApp->modalWindow() == 0)
    {
        qDebug()<<qApp->activeWindow();
        this->activateWindow();
    }
}
