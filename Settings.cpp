#include "Settings.h"
#include "ui_Settings.h"
#include <QSignalMapper>
#include <math.h>
#include "Global.h"
#include <QListView>
#include "Camera.h"

Settings::Settings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    ui->widget_parent->setAutoFillBackground(true);//否则窗口会变透明

//    m_checkedStyle = ui->pushButton_general->styleSheet();
//    m_uncheckedStyle = ui->pushButton_preference->styleSheet();

    QList<QComboBox *> comboxList = findChildren<QComboBox *>();
    for(int i = 0;i<comboxList.count();i++)//
    {
        QListView *listView = new QListView();
        comboxList.at(i)->setView(listView);
        connect(comboxList.at(i), SIGNAL(activated(QString)),
                    this, SLOT(comboBoxActivated(QString)));
    }

    QList<QPushButton *> mybuttonlist = this->findChildren<QPushButton *>();
    int count = mybuttonlist.count();
    if(count > 0)
    {
        QSignalMapper *signalMapper = NULL;
        signalMapper = new QSignalMapper();

        //经验证，信号void QObject::destroyed(QObject * obj = 0)与
        //槽void QObject::deleteLater()可实现绑定,即使参数不一致
        QObject::connect(this, SIGNAL(destroyed()),
                signalMapper, SLOT(deleteLater()));

        while(count--)
        {
            signalMapper->setMapping(mybuttonlist.at(count), mybuttonlist.at(count));
            QObject::connect(mybuttonlist.at(count), SIGNAL(released()),
                        signalMapper, SLOT(map()));
        }
        QObject::connect(signalMapper, SIGNAL(mapped(QWidget * )),
                this, SLOT(buttonReleased(QWidget * )));
    }

//    this->buttonReleased(ui->pushButton_general);

    Camera *camera = Camera::getInstance();
    connect(camera, SIGNAL(availableCamera(QStringList)),
            this, SLOT(availableCameraReady(QStringList)));

    //注册事件过滤器，屏蔽鼠标滚动来修改 QDoubleSpinBox 值大小
    QList<QDoubleSpinBox *> dSpinBoxList = findChildren<QDoubleSpinBox *>();
    foreach(QDoubleSpinBox *dSpinBox, dSpinBoxList)
    {
        dSpinBox->installEventFilter(this);
    }
    g_clearPushButtonFocus(this);
}

Settings::~Settings()
{
    delete ui;
}

void Settings::mousePressEvent(QMouseEvent *event)
{
    m_relativePos = this->pos()-event->globalPos();
}

void Settings::mouseMoveEvent(QMouseEvent *event)
{
    this->move(event->globalPos()+ m_relativePos);
}

void Settings::showEvent(QShowEvent *event)
{
//    ui->comboBox_camera_ip->setCurrentText(
//                g_settings.value(HKEY_CAMERA_IP,"192.0.0.64").toString());
    ui->lineEdit_camera_port->setText(
                g_settings.value(HKEY_CAMERA_PORT, "8000").toString());
    ui->lineEdit_camera_user->setText(
                g_settings.value(HKEY_CAMERA_USER, "admin").toString());
    ui->lineEdit_camera_password->setText(
                g_settings.value(HKEY_CAMERA_PASSWORD, "zdmq8888").toString());

    ui->doubleSpinBox_threshold_HC->setValue(
                g_settings.value(HKEY_LIMIT_HC, 600).toDouble());
    ui->doubleSpinBox_threshold_NO->setValue(
                g_settings.value(HKEY_LIMIT_NO, 5000).toDouble());
    ui->doubleSpinBox_threshold_CO->setValue(
                g_settings.value(HKEY_LIMIT_CO, 2).toDouble());
    ui->doubleSpinBox_threshold_K->setValue(
                g_settings.value(HKEY_LIMIT_K, 2.87).toDouble());
    ui->doubleSpinBox_threshold_opacity->setValue(
                g_settings.value(HKEY_LIMIT_OPACITY, 2).toDouble());

    ui->doubleSpinBox_threshold_diesel_opacity->setValue(
                g_settings.value(HKEY_PREFERENCE_THRESHOLD_DIESEL_OPACITY, 0).toDouble());

    ui->checkBox_enable_acc_check->setChecked(
                g_settings.value(HKEY_PREFERENCE_ENABLE_ACC_CHECK, true).toBool());
    ui->doubleSpinBox_threshold_acc_lower->setValue(
                g_settings.value(HKEY_PREFERENCE_THRESHOLD_ACC_LOWER, 0).toDouble());

    ui->checkBox_enable_VSP_check->setChecked(
                g_settings.value(HKEY_PREFERENCE_ENABLE_VSP_CHECK, true).toBool());
    ui->doubleSpinBox_threshold_VSP_lower->setValue(
                g_settings.value(HKEY_PREFERENCE_THRESHOLD_VSP_LOWER, 0).toDouble());
    ui->doubleSpinBox_threshold_VSP_upper->setValue(
                g_settings.value(HKEY_PREFERENCE_THRESHOLD_VSP_UPPER, 14).toDouble());

    updateComList();

//    updateCameraList();
}

void Settings::buttonReleased(QWidget *w)
{
    QString buttonName = w->objectName();
    if(buttonName == "pushButton_close" ||
            buttonName == "pushButton_save_setttings")
    {
        g_settings.setValue(HKEY_CAMERA_IP,ui->comboBox_camera_ip->currentText());
        g_settings.setValue(HKEY_CAMERA_PORT,ui->lineEdit_camera_port->text());
        g_settings.setValue(HKEY_CAMERA_USER,ui->lineEdit_camera_user->text());
        g_settings.setValue(HKEY_CAMERA_PASSWORD,ui->lineEdit_camera_password->text());

        g_settings.setValue(HKEY_LIMIT_HC,ui->doubleSpinBox_threshold_HC->value());
        g_settings.setValue(HKEY_LIMIT_NO,ui->doubleSpinBox_threshold_NO->value());
        g_settings.setValue(HKEY_LIMIT_CO,ui->doubleSpinBox_threshold_CO->value());
        g_settings.setValue(HKEY_LIMIT_K,ui->doubleSpinBox_threshold_K->value());
        g_settings.setValue(HKEY_LIMIT_OPACITY
                            ,ui->doubleSpinBox_threshold_opacity->value());

        g_settings.setValue(HKEY_PREFERENCE_ENABLE_ACC_CHECK
                            ,ui->checkBox_enable_acc_check->isChecked());
        g_settings.setValue(HKEY_PREFERENCE_THRESHOLD_ACC_LOWER
                            ,ui->doubleSpinBox_threshold_acc_lower->value());

        g_settings.setValue(HKEY_PREFERENCE_ENABLE_VSP_CHECK
                            ,ui->checkBox_enable_VSP_check->isChecked());
        g_settings.setValue(HKEY_PREFERENCE_THRESHOLD_VSP_LOWER
                            ,ui->doubleSpinBox_threshold_VSP_lower->value());
        g_settings.setValue(HKEY_PREFERENCE_THRESHOLD_VSP_UPPER
                            ,ui->doubleSpinBox_threshold_VSP_upper->value());

        g_settings.setValue(HKEY_PREFERENCE_THRESHOLD_DIESEL_OPACITY
                            ,ui->doubleSpinBox_threshold_diesel_opacity->value());

        g_settings.sync();
//        this->close();
        g_closeWithAnim(this);
    }
    else if(buttonName == "pushButton_camera_refresh")
    {
        ui->comboBox_camera_ip->clear();
        ui->pushButton_camera_refresh->setEnabled(false);
        qApp->setOverrideCursor(Qt::WaitCursor);
        qApp->processEvents();
        updateCameraList();
        ui->pushButton_camera_refresh->setEnabled(true);
        qApp->restoreOverrideCursor();
    }

}

void Settings::paintEvent(QPaintEvent *event)
{
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);

    int grayWidth = 8;

    path.addRect(grayWidth, grayWidth, this->width() - grayWidth*2,
                 this->height()-grayWidth*2);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

//    QPixmap pixmap = this->grab();
    painter.fillPath(path, QBrush(Qt::gray));

    QColor color(0, 0, 0, 15);
    for(int i = 0; i<grayWidth; i++)
    {
        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        path.addRect(grayWidth-i, grayWidth-i, this->width()-(grayWidth-i)*2,
                     this->height()-(grayWidth-i)*2);
        int max_alpha = 25;
        color.setAlpha((sqrt(grayWidth)*max_alpha+1) - sqrt(i)*max_alpha);
        painter.setPen(color);
        painter.drawPath(path);
    }
}

/*************************************************
** Function: updateComList
** Description: 打开串口设置窗口时更新串口状态
** Input:
** Output:
** Return:
*************************************************/
void Settings::updateComList()
{
    QStringList comNameList = g_getAvailableComList();
    comNameList.append(tr("关闭"));
    ui->comboBox_environment_parameter->clear();
//    if(comNameList.count())
    {
        comNameList.sort();
        ui->comboBox_environment_parameter->addItems(comNameList);

        QString port_name = g_settings.value(HKEY_WEATHER_COM,"com3").toString();

//        qDebug()<<"3 port_name = "<<port_name;
        int index = ui->comboBox_environment_parameter->findText(port_name);
        if(index >= 0)
        {
            ui->comboBox_environment_parameter->setCurrentIndex(index);
        }
        else
        {
            ui->comboBox_environment_parameter->setCurrentIndex(
                        comNameList.count()-1);
        }
    }
}

/*************************************************
** Function: comboBoxActivated
** Description: 如果用户选中串口号为“关闭”，则波特率变灰色(非使能，不能选择了)。
** Input:
** Output:
** Return:
*************************************************/
void Settings::comboBoxActivated(QString value)
{
//    LOG_HERE("");
    QComboBox *comboBox = qobject_cast<QComboBox*>(sender());
    if(comboBox == 0)
        return;
    QString combo_name = comboBox->objectName();
    qDebug()<<"LightCheck combo_name = "<<combo_name<<" text = "<<value;

    if(combo_name == "comboBox_environment_parameter")
    {
        QString comName = value;

        if(g_weatherCom.portName() != comName)
        {
            g_settings.setValue(HKEY_WEATHER_COM,comName);
            g_weatherCom.close();//将会在环境参数类里打开
        }
    }

}

void Settings::updateCameraList()
{
    Camera *camera = Camera::getInstance();
    QStringList ipList = camera->getAllOnlineCamera();
    this->availableCameraReady(ipList);
}

void Settings::availableCameraReady(QStringList cameraIpList)
{
    QString currentCameraIp = g_settings.value(HKEY_CAMERA_IP, "192.0.0.64").toString();
    ui->comboBox_camera_ip->clear();
    ui->comboBox_camera_ip->addItems(cameraIpList);
    if(!currentCameraIp.isEmpty() && cameraIpList.contains(currentCameraIp))
    {
        ui->comboBox_camera_ip->setCurrentText(currentCameraIp);
    }
}


void Settings::on_comboBox_camera_ip_currentIndexChanged(const QString &cameraIp)
{
    MY_DEBUG("");
    Camera *camera = Camera::getInstance();
    camera->reconnect(cameraIp);
}

bool Settings::eventFilter(QObject * watched, QEvent * event)
{
    if(event->type() == QEvent::Wheel)//过滤鼠标滚动事件
    {
        return true;
    }
    return false;
}
