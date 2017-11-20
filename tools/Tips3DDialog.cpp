#include "Tips3DDialog.h"
#include "ui_Tips3DDialog.h"
#include <QDesktopWidget>
#include <QPalette>
#include <QBrush>
#include <QPainter>
#include <QPainterPath>
#include <QPolygon>
#include <QDebug>
#include <QTimer>
//#include "CommonFuntions.h"
#define IS_DEBUG_MODE 0

Tips3DDialog::Tips3DDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Tips3DDialog)
{
    LOG_HERE("");

    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);//设置不显示窗口菜单栏
    QDesktopWidget* desktop = QApplication::desktop(); //  = qApp->desktop();也可以
    move((desktop->width() - this->width())/2, (desktop->height() - this->height())/2);//移到桌面中间
    this->setAutoFillBackground(true);//用于显示背景图片setPalette(palette);
    ui->horizontalWidget->setAutoFillBackground(true);

    m_widget_parent = NULL;
    m_widget_goal = NULL;
    m_errorSign = -999;

    QList<QPushButton *> mybuttonlist = this->findChildren<QPushButton *>();
    foreach(QPushButton *button,mybuttonlist)
    {
        if(button->objectName().contains(QRegExp("^pushButton_test_(top|down)_(left|right)$")))
        {
            button->setStyleSheet("border:none;");
            button->setVisible((IS_DEBUG_MODE>0)?true:false);
        }
        connect(button, SIGNAL(released()),this, SLOT(buttonReleased()));
    }

    if(IS_DEBUG_MODE<1)
    {
        ui->pushButton->resize(this->size());
        ui->pushButton->move(0,0);
    }
    m_widget_goal = 0;
//    ui->label_tips->setText(tr("水平偏差设置范围\n|X|< = 1050mm/10m"));
//    QTimer::singleShot(0,this,SLOT(draw3DShadow()));
    m_image = QImage(this->size(),QImage::Format_ARGB32);

//    g_clearPushButtonFocus(this);
}

Tips3DDialog::~Tips3DDialog()
{
    delete ui,m_widget_goal;
}

/*************************************************
** Function: draw3DShadow
** Description: 绘制指向目标控件的3D效果
*************************************************/
void Tips3DDialog::draw3DShadow()
{
    LOG_HERE("");
    m_image = QImage(this->size(),QImage::Format_ARGB32);
    QPainter painter(&m_image);
    QPainterPath path;
    QPolygon polygon;//多边形
    QBrush mybrush(QColor(255, 244, 245, 131));
//    QRect rect = this->rect();
//    qDebug()<<"rect = "<<rect;
//    path.addRect(rect);
//    painter.fillPath(path,QBrush(QColor(Qt::transparent)));
//    qDebug()<<"pos_des_down = "<<pos_des_down;
//    qDebug()<<"pos_des_up = "<<pos_des_up;
    QPoint des_pos_tmp;
    if(m_pos_des_down.x() == m_errorSign)
    {
        des_pos_tmp = m_pos_des_up;
        des_pos_tmp.setY(des_pos_tmp.y()-10);
    }
    else
    {
        des_pos_tmp = m_pos_des_down;
        des_pos_tmp.setY(des_pos_tmp.y()+10);
    }
    des_pos_tmp.setX(des_pos_tmp.x()+5);
//    pos_des_down = ui->horizontalWidget->mapTo(this,pos_des_down);
//    pos_des_up = ui->horizontalWidget->mapTo(this,pos_des_up);
    des_pos_tmp = ui->horizontalWidget->mapTo(this,des_pos_tmp);
    m_pos_a = ui->horizontalWidget->mapTo(this,m_pos_a);
    m_pos_b = ui->horizontalWidget->mapTo(this,m_pos_b);
    m_pos_c = ui->horizontalWidget->mapTo(this,m_pos_c);
    m_pos_d = ui->horizontalWidget->mapTo(this,m_pos_d);

//    qDebug()<<"pos_des_down = "<<pos_des_down;
//    qDebug()<<"pos_des_up = "<<pos_des_up;
//    qDebug()<<"pos_a = "<<pos_a;
//    qDebug()<<"pos_b = "<<pos_b;
//    qDebug()<<"pos_c = "<<pos_c;
//    qDebug()<<"pos_d = "<<pos_d;

    QPen pen(QColor(134, 142, 255));
    pen.setWidth(2);
    painter.setPen(pen);
    painter.drawRect(this->m_des_w_rect);

    polygon.clear();
    //底边或上边与目标点三角形
    if(m_pos_des_down.x() != m_errorSign)
        polygon<<des_pos_tmp<<m_pos_d<<m_pos_c;
    else
        polygon<<des_pos_tmp<<m_pos_a<<m_pos_b;
    mybrush.setColor(QColor(255, 240, 245, 135));
    path = QPainterPath();
    path.addPolygon(polygon);
    painter.fillPath(path,mybrush);

    polygon.clear();
    polygon<<des_pos_tmp;
    if(des_pos_tmp.x()<m_pos_a.x())
        polygon<<m_pos_a<<m_pos_d;//左边与目标点三角形
    else
        polygon<<m_pos_b<<m_pos_c;//右边与目标点三角形
    mybrush.setColor(QColor(250, 235, 240, 120));
    path = QPainterPath();
    path.addPolygon(polygon);
    painter.fillPath(path,mybrush);

    painter.end();
//  方法一
    this->update();
//方法二
//    QPalette palette;
//    palette.setBrush(QPalette::Window, QBrush(QPixmap::fromImage(image)));
//    this->setPalette(palette);

}

void Tips3DDialog::paintEvent(QPaintEvent *)
{
//    qDebug()<<"1 paintEvent(QPaintEvent *)";
    if(m_widget_parent != NULL && m_widget_goal != NULL)
    {
        QSize parent_size = m_widget_parent->size();
        if(this->size() != parent_size ||
           ui->pushButton->size() != parent_size)
        {
            this->resize(parent_size);
            ui->pushButton->resize(parent_size);
            ui->pushButton->move(0,0);
            updatePosition(m_widget_goal);
            draw3DShadow();
        }
//        qDebug()<<"this->size() = "<<this->size();
    }
//    if(image.isNull() == true)
//        return;

//    qDebug()<<"2 paintEvent(QPaintEvent *)";
//    g_logHere(LOG_INFO(g_log_var_list,"paintEvent(QPaintEvent *)"));
    QPainter mypainter(this);
    mypainter.drawPixmap(0,0, QPixmap::fromImage(m_image));


//    image = QImage();
//    this->setParent(0);
}

void Tips3DDialog::buttonReleased()
{
    LOG_HERE("");

    QPushButton *nowbutton = qobject_cast<QPushButton*>(sender());
    if(nowbutton == 0)
        return;
    QString button_name = nowbutton->objectName();
    LOG_HERE(button_name);

    if(button_name.contains(QRegExp("^pushButton_test_(top|down)_(left|right)$")))
    {
        this->showMessage(button_name,nowbutton);
    }
}

void Tips3DDialog::showMessage(QString message,QWidget *w)
{
    LOG_HERE("");

    ui->label_tips->setText(message);
    ui->label_tips->adjustSize();
    this->show();
    if(IS_DEBUG_MODE > 0)
    {
        ui->label_tips->repaint();
        qApp->processEvents();
    }
    updatePosition(w);
    draw3DShadow();

//    QTimer::singleShot(100,this,SLOT(isEditingFinished()));
}

/*************************************************
** Function: updatePosition
** Description: 指向哪个目标控件，计算指向的坐标点
*************************************************/
void Tips3DDialog::updatePosition(QWidget *des_w)
{
    LOG_HERE("");
    //ui->label_tips的4个顶点
    //          A---------B
    //          |         |
    //          D---------C
    //        /             \
    //    pos_des          pos_des
    int width = ui->label_tips->width();
    int height = ui->label_tips->height();
    //在ui->horizontalWidget中对应的点
    m_pos_a = ui->label_tips->mapTo(ui->horizontalWidget, QPoint(1, 0));
    m_pos_b = ui->label_tips->mapTo(ui->horizontalWidget, QPoint(width, 0));
    m_pos_c = ui->label_tips->mapTo(ui->horizontalWidget, QPoint(width - 1, height));
    m_pos_d = ui->label_tips->mapTo(ui->horizontalWidget, QPoint(1, height));
    m_pos_des_down = QPoint(0, ui->horizontalWidget->height());
    m_pos_des_up = QPoint(0, m_pos_a.y() - (m_pos_des_down.y() - m_pos_d.y()));

//    w_input = qApp->inputContext()->focusWidget();

    if(des_w != 0)
    {
        QObject *obj_parent_tmp = des_w->parent();
        QObject *obj_parent = obj_parent_tmp;

        while(1)
        {
            if(obj_parent_tmp->parent() == 0)
                break;
            obj_parent = obj_parent_tmp->parent();
            obj_parent_tmp = obj_parent;
        }
        if(obj_parent != 0)
        {
            QWidget *parent_w = qobject_cast<QWidget*>(obj_parent);
            m_widget_goal = des_w;
            m_widget_parent = parent_w;
            this->setParent(parent_w);
            this->resize(parent_w->size());
            this->show();
            this->move(QPoint(0,0));
//            qDebug()<<"w_parent_tmp = "<<obj_parent->objectName();

            QPoint des_w_global_pos = des_w->mapTo(parent_w, QPoint(0,0));
            m_des_w_rect = QRect(des_w_global_pos,des_w->size());
//            qDebug()<<"w_pos_global.x() = "<<w_pos_global.x();
//            qDebug()<<"w_pos_global.y() = "<<w_pos_global.y();
//            qDebug()<<"pos_b.x() = "<<pos_b.x();
            int parent_w_width = parent_w->width();
//            int parent_w_height = parent_w->height();
            if((des_w_global_pos.x() + m_pos_b.x()) > parent_w_width)//箭头指向右边
            {
                m_pos_des_down.setX(m_pos_b.x() + m_pos_a.x());
                m_pos_des_up.setX(m_pos_b.x() + m_pos_a.x());
                ui->horizontalWidget->move(des_w_global_pos.x() - m_pos_des_down.x() + 4,
                                           des_w_global_pos.y()-ui->horizontalWidget->height());
//                qDebug()<<"pos_des.setX(pos_a.x()+pos_b.x());";
            }
            else//箭头指向左边
            {
                m_pos_des_down.setX(0);
                m_pos_des_up.setX(0);
                ui->horizontalWidget->move(des_w_global_pos.x() + 4,
                                           des_w_global_pos.y() - ui->horizontalWidget->height());
//                qDebug()<<"pos_des.setX(0);";
            }

            QPoint pos_tmp = ui->horizontalWidget->pos();
//            qDebug()<<"pos_des_down = "<<pos_des_down;
//            qDebug()<<"pos_des_up = "<<pos_des_up;
//            qDebug()<<"pos_a = "<<pos_a;
//            qDebug()<<"pos_b = "<<pos_b;
//            qDebug()<<"pos_c = "<<pos_c;
//            qDebug()<<"pos_d = "<<pos_d;
//            qDebug()<<"des_w_global_pos = "<<des_w_global_pos;
            //上方被遮挡
            if((des_w_global_pos.y() - (m_pos_des_down.y() - m_pos_a.y()))<0)
            {
                m_pos_des_down.setX(m_errorSign);//用作失效判断
                ui->horizontalWidget->move(pos_tmp.x()
                                        ,pos_tmp.y()
                                        //以pos_des_down点作为水平线，做镜像翻转
                                        + (m_pos_des_down.y() - m_pos_a.y())
                                        + (m_pos_des_down.y() - m_pos_d.y())
                                        + des_w->height());//目标控件的高度
            }
            else
            {
                m_pos_des_up .setX(m_errorSign);
            }
        }
    }
}

void Tips3DDialog::on_pushButton_released()
{
    LOG_HERE("");
    this->hide();
//    this->setParent(0);
}





