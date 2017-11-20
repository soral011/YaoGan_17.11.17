#include "AdjustCamera.h"
#include "ui_AdjustCamera.h"
#include "MyDebug.cpp"

AdjustCamera::AdjustCamera() :
    QDialog(),
    ui(new Ui::AdjustCamera)
{
    ui->setupUi(this);

    m_plateRecogRegion = NULL;
    m_camera = Camera::getInstance();

    connect(this,SIGNAL(controlYunTai(DWORD,DWORD)),
            m_camera,SLOT(controlYunTai(DWORD,DWORD)));

    //用于云台控制
    this->setMouseTracking(true);
    QList <QWidget *> w_list = this->findChildren<QWidget *>();
    foreach(QWidget *w,w_list)
    {
        w->setMouseTracking(true);
    }
    this->setYunTaiControlVisible(false);

    //将ui->widget_yuntai设置为NULL，否则鼠标移动到视频区时，其不能一直显示
    //以下语句顺序不能变，否则影响效果
    ui->widget_yuntai->setParent(NULL);
    ui->widget_yuntai->setWindowFlags(Qt::Tool |Qt::FramelessWindowHint |
                                      Qt::WindowStaysOnTopHint);
    ui->widget_yuntai->setAttribute(Qt::WA_TranslucentBackground, true);//使背景透明
    connect(this, SIGNAL(destroyed()), ui->widget_yuntai, SLOT(deleteLater()));

    QList<QPushButton *> mybuttonlist = this->findChildren<QPushButton *>();
    //ui->widget_yuntai在前面被设置为NULL了
    if(mybuttonlist.contains(ui->pushButton_yuntai_down) == false)
    {
        mybuttonlist += ui->widget_yuntai->findChildren<QPushButton *>();
    }

    int i = mybuttonlist.count();
    while(i--)
    {
        m_signalMapper.setMapping(mybuttonlist.at(i), mybuttonlist.at(i));
        connect(mybuttonlist.at(i), SIGNAL(released()),
                &m_signalMapper, SLOT(map()));

        //云台控制：鼠标按下的瞬间开始转动云台，鼠标放开时停止转动
        if(mybuttonlist.at(i)->objectName().
                contains("pushButton_yuntai_"))
        {
            connect(mybuttonlist.at(i),SIGNAL(pressed()),
                    this,SLOT(turnYunTai()));
        }
    }
    connect(&m_signalMapper, SIGNAL(mapped(QWidget * )),
            this, SLOT(buttonReleased(QWidget * )));

    g_clearPushButtonFocus(this);
}

AdjustCamera::~AdjustCamera()
{
    MY_DEBUG("");
    delete ui;
    delete m_plateRecogRegion;
    MY_DEBUG("");
}

AdjustCamera  *AdjustCamera::getInstance()
{
    static AdjustCamera instance; //局部静态变量，若定义为指针，则需要手动释放内容
    return &instance;
}

void AdjustCamera::buttonReleased(QWidget *w)
{
    QString buttonName = w->objectName();

    if(buttonName.contains(QRegExp("^pushButton_yuntai_(up|down|right|left|focus_add|focus_sub)$")))
    {
        stopYunTai();

        //再次发送停止命令，确保云台停止
        int stopYunTaiTime = 50;//ms
        QTimer::singleShot(stopYunTaiTime, this, SLOT(stopYunTai()));
    }
}

void AdjustCamera::stopYunTai()
{
    emit controlYunTai(UP_RIGHT,1);
}

void AdjustCamera::turnYunTai()
{
    QObject *obj = qobject_cast<QObject*>(this->sender());
    if(obj == 0)
    {
        return;
    }

    QString buttonName = obj->objectName();

    DWORD dwPTZCommand;
    if(buttonName.contains("up"))
    {
        dwPTZCommand = TILT_UP;
    }
    else if(buttonName.contains("down"))
    {
        dwPTZCommand = TILT_DOWN;
    }
    else if(buttonName.contains("left"))
    {
        dwPTZCommand = PAN_LEFT;
    }
    else if(buttonName.contains("right"))
    {
        dwPTZCommand = PAN_RIGHT;
    }
    else if(buttonName.contains("focus_add"))
    {
        dwPTZCommand = ZOOM_IN;
    }
    else if(buttonName.contains("focus_sub"))
    {
        dwPTZCommand = ZOOM_OUT;
    }

    //是否使用自己配置的云台
//    if(g_settings.value(HKEY_IS_USING_OWN_YUN_TAI,true).toBool() == true)
//    {
////        controlYunTai(cmd);
//    }
//    else
//    {
        emit controlYunTai(dwPTZCommand,0);
//    }
}

//用于隐藏或者显示控制云台按钮
void AdjustCamera::mouseMoveEvent(QMouseEvent *event)
{
    if(this->isVisible() == false)
    {
        return;
    }

    //判断是否该显示云台控制按钮、车牌识别区等
    //当鼠标移至图像浏览子窗口时显示云台控制按钮等，否则隐藏
    QPoint pos = ui->widget_show_vedio->mapToGlobal(QPoint(0,0));
    int x = event->globalX() - pos.x();
    int y = event->globalY() - pos.y();
    QRect vedioWidgetRect = ui->widget_show_vedio->rect();

    if(vedioWidgetRect.contains(QPoint(x,y)))
    {
        if(ui->pushButton_yuntai_right->isVisible() == false)
        {
            setYunTaiControlVisible(true);
            //当鼠标突然移动到具有鼠标捕获的控件上，则需要判断是否该隐藏云台控制按钮
            QTimer::singleShot(50,this,SLOT(isTimeToHide()));
        }
    }
    else if(ui->pushButton_yuntai_right->isVisible())
    {
        setYunTaiControlVisible(false);
    }
}

void AdjustCamera::setYunTaiControlVisible(bool value)
{
    //摄像机连接上才会显示云台、识别区调节 2016-07-12
//    if(m_cameraConnected == false && value == true)
//    {
//        return;
//    }

    QWidget *vedioWidget = ui->widget_show_vedio;
    QWidget *yuntaiWidget = ui->widget_yuntai;

    yuntaiWidget->setVisible(value);
    if(value)
    {
       QSize size = vedioWidget->size();
       yuntaiWidget->resize(size.width(), yuntaiWidget->height());

       QPoint pos = vedioWidget->mapToGlobal(QPoint(0,0));
       int h = vedioWidget->height() - yuntaiWidget->height();
       yuntaiWidget->move(pos.x(), pos.y() + h);
    }

//    ui->widget_yuntai->raise();

    if(m_plateRecogRegion == NULL)
    {
        m_plateRecogRegion = new PlateRecogRegion();
    }

    if(m_plateRecogRegion->size()!= vedioWidget->size())
    {
        m_plateRecogRegion->resize(vedioWidget->size());
        QTimer::singleShot(10, m_plateRecogRegion, SLOT(updateRecogRegion()));
    }
    m_plateRecogRegion->move(vedioWidget->mapToGlobal(QPoint(0,0)));
    m_plateRecogRegion->setVisible(value);
}

//是否该隐藏云台控制按钮或者信息提示窗口
void AdjustCamera::isTimeToHide()
{
    //云台控制按钮等
    QPoint pos = ui->widget_show_vedio->mapToGlobal(QPoint(0,0));
    int x = QCursor::pos().x()-pos.x();
    int y = QCursor::pos().y()-pos.y();
    QRect rect = ui->widget_show_vedio->rect();
    if(rect.contains(QPoint(x,y)) == false)
    {
        setYunTaiControlVisible(false);
    }
    else
    {
        QTimer::singleShot(50,this,SLOT(isTimeToHide()));
    }
}

void AdjustCamera::showEvent(QShowEvent * event)
{
    MY_DEBUG("");
    HWND hWnd = (HWND)ui->widget_show_vedio->winId();
    m_camera->reStartPreview(hWnd);
//    ui->widget_show_vedio->clearFocus();
//    if(m_camera->isRunning() == false)
//    {
//        m_camera->start();
//    }
}

void AdjustCamera::closeEvent(QCloseEvent * event)
{
    MY_DEBUG("");
    setYunTaiControlVisible(false);
    this->setParent(NULL);
}

void AdjustCamera::hideEvent(QHideEvent * event)
{
    MY_DEBUG("");
    setYunTaiControlVisible(false);
    this->setParent(NULL);
}

bool AdjustCamera::isAdjustingCamera()
{
    return ui->widget_yuntai->isVisible();
}

