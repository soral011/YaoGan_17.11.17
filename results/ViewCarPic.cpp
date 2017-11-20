#include "ViewCarPic.h"
#include "ui_ViewCarPic.h"
#include "Global.h"
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QCursor>
#include <QPainter>
#include <qlabel.h>
#include <math.h>

ViewCarPic::ViewCarPic(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ViewCarPicWidget)
{
    ui->setupUi(this);
//    this->setAttribute(Qt::WA_TranslucentBackground);
//    ui->label_car_pic->setAttribute(Qt::WA_TranslucentBackground);

    setMouseTracking(true);

    m_arrowRight = new QPushButton(this);
    m_arrowLeft = new QPushButton(this);
    m_arrowRight->hide();
    m_arrowLeft->hide();

    m_arrowRight->setObjectName("pushButton_arrow_right");
    m_arrowLeft->setObjectName("pushButton_arrow_left");

    m_arrowRight->resize(50,50);
    m_arrowLeft->resize(50,50);

    m_arrowRight->setStyleSheet("border-image: url(:/images/arrow_right.png);");
    m_arrowLeft->setStyleSheet("border-image: url(:/images/arrow_left.png);");

    g_connectButtonSignal(this);

//    updateArrowPosition();
    g_clearPushButtonFocus(this);
}

ViewCarPic::~ViewCarPic()
{
    delete ui;
}

void ViewCarPic::resizeEvent(QResizeEvent * event)
{
//    this->updateArrowPosition();
}

void ViewCarPic::mouseMoveEvent(QMouseEvent *event)
{
//    qDebug()<<event->pos();
    int x = event->pos().x();
    int y = event->pos().y();
    int w = this->width();

    if((ui->label_car_pic->y() <= y)&&
            ( y <= ui->label_car_pic->pos().y()+ui->label_car_pic->height()))
    {
        QPixmap pixmap(":/images/arrow_right_blue");
        pixmap = pixmap.scaled(pixmap.width()*2/3,pixmap.height()*2/3);
        if(w/2<x && x<w)
        {
            QCursor cursor(pixmap);
            ui->label_car_pic->setCursor(cursor);
            m_isRight = true;
        }
        else
        {
            QMatrix leftmatrix;
            leftmatrix.rotate(180);
            QCursor cursor(pixmap.transformed(leftmatrix,Qt::SmoothTransformation));
            ui->label_car_pic->setCursor(cursor);
            m_isRight = false;
        }
    }
    else
    {
        QPoint pos = event->globalPos()+ m_relativePos;

        QWidget *parent = this->parentWidget();
        if(parent != 0 && pos.x()>0 && pos.y()>0)
        {
            QRect rect = parent->rect();
            pos.setX(pos.x()+this->width());
            if(pos.x()>parent->width())
            {
               pos.setY(parent->width());
            }
            pos.setY(pos.y()+this->height());
            if(pos.y()>parent->height())
            {
               pos.setY(parent->height());
            }
            if(rect.contains(pos))
            {
                pos = event->globalPos()+ m_relativePos;
                this->move(pos);
            }
        }

        if(m_arrowRight->isVisible())
        {
            m_arrowRight->hide();
            m_arrowLeft->hide();
        }
//        setArrowButtonVisible(false);
    }
}

void ViewCarPic::leaveEvent(QEvent * event)
{
    m_arrowRight->hide();
    m_arrowLeft->hide();
//    setArrowButtonVisible(false);
}

void ViewCarPic::mouseReleaseEvent(QMouseEvent * e)
{
    QPoint pos = e->pos();
    QRect rect = ui->label_car_pic->rect();
    rect.setX(ui->label_car_pic->pos().x());
    rect.setY(ui->label_car_pic->pos().y());
    if(rect.contains(pos))
    {
//        qDebug()<<"contains(pos)";
        emit switchItem(m_isRight);
    }
}

void ViewCarPic::mousePressEvent(QMouseEvent *event)
{
    m_relativePos = this->pos()-event->globalPos();
}

void ViewCarPic::showCarInfo()
{
    QSize carWidgetSize = ui->label_car_pic->size();
    if(m_pixmap.isNull())
    {
        qDebug()<<"m_pixmap.isNull()";
        return;
    }
    else
    {
        QPixmap pixmapTmp = m_pixmap.scaledToHeight(carWidgetSize.height());
        if(pixmapTmp.width() <= carWidgetSize.width())
        {
            //画阴影边框
            QPixmap carPixmap(pixmapTmp.size());
            carPixmap.fill(Qt::transparent);
            QPainter painter(&carPixmap);
            QPainterPath path;
            path.setFillRule(Qt::WindingFill);
            int grayWidth = 5;
            path.addRect(grayWidth, grayWidth, pixmapTmp.width()-grayWidth*2,
                         pixmapTmp.height()-grayWidth*2);

            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.fillPath(path, QBrush(Qt::white));

            QColor color(0, 0, 0, 15);
            for(int i = 0; i<grayWidth; i++)
            {
                QPainterPath path;
                path.setFillRule(Qt::WindingFill);
                path.addRect(grayWidth-i, grayWidth-i, pixmapTmp.width()-(grayWidth-i)*2,
                             pixmapTmp.height()-(grayWidth-i)*2);
                int max_alpha = 55;
                color.setAlpha((sqrt(grayWidth)*max_alpha+1) - sqrt(i)*max_alpha);
                painter.setPen(color);
                painter.drawPath(path);
            }

            //画车牌图像到画布
            pixmapTmp = m_pixmap.scaled(pixmapTmp.width()-grayWidth*2,
                                      pixmapTmp.height()-grayWidth*2);
            painter.drawPixmap(grayWidth,grayWidth,pixmapTmp);
            painter.end();

            //将车牌图像和阴影边框画到整个Widget窗口尺寸中间
            m_pixmap = m_pixmap.scaled(carWidgetSize);
            m_pixmap.fill(QColor(48,48,48));
            painter.begin(&m_pixmap);
            painter.drawPixmap((carWidgetSize.width()-carPixmap.width())/2,0,
                               carPixmap);
            painter.end();
        }
    }
    ui->label_license->setText(this->m_license);

    //使用属性动画来切换图片
    //..准备图片
    QPixmap pixmap(carWidgetSize.width()*2,carWidgetSize.height());
    QPainter painter(&pixmap);
    if(this->m_isRight)
    {
        painter.drawPixmap(0,0,ui->label_car_pic->grab());
        painter.drawPixmap(carWidgetSize.width(),0,m_pixmap.scaled(carWidgetSize));
    }
    else
    {
        painter.drawPixmap(0,0,m_pixmap.scaled(carWidgetSize));
        painter.drawPixmap(carWidgetSize.width(),0,ui->label_car_pic->grab());
    }
    painter.end();

    //..创建QLabel控件，用作动画窗口
    QLabel *label = new QLabel(ui->label_car_pic);
    label->resize(carWidgetSize.width()*2,carWidgetSize.height());
    label->setPixmap(pixmap);
    label->show();
//    qDebug()<<"pixmap.size() = "<<pixmap.size();
//    qDebug()<<"label.size() = "<<label->size();
    QPropertyAnimation *anim = new QPropertyAnimation(label,"pos");
    anim->setDuration(380);
    anim->setEasingCurve(QEasingCurve::OutQuad);
    if(this->m_isRight)
    {
        anim->setStartValue(QPoint(-carWidgetSize.width()/2,0));
        anim->setEndValue(QPoint(-carWidgetSize.width(),0));
    }
    else
    {
        anim->setStartValue(QPoint(-carWidgetSize.width()/2,0));
        anim->setEndValue(QPoint(0,0));
    }
    anim->start();
    connect(anim,SIGNAL(finished()),label,SLOT(deleteLater()));

    //动画显示完毕，将呈现ui->label_car_pic
    ui->label_car_pic->setPixmap(m_pixmap.scaled(carWidgetSize));
    ui->label_car_pic->update();
    //    m_pixmap.save("test.jpg","JPG");//保存图片知本身存在黑边
}

void ViewCarPic::updateArrowPosition()
{
    int width = this->width();
    int height = this->height();

    m_arrowLeft->move(0,(height-m_arrowLeft->height())/2);
    m_arrowRight->move(width-m_arrowRight->width()
                       ,(height-m_arrowLeft->height())/2);
}


void ViewCarPic::buttonReleased(QWidget *w)
{
//    QString nowbutton_name = w->objectName();
////    qDebug()<<"nowbutton_name = "<<nowbutton_name;

//    if(nowbutton_name =  = "pushButton_arrow_right")
//    {
//        emit this->switchItem(true);
//    }
//    else if(nowbutton_name =  = "pushButton_arrow_left")
//    {
//        emit this->switchItem(false);
//    }
}

void ViewCarPic::setArrowButtonVisible(bool visible)
{
//    int duration = 2050;
//    QEasingCurve esasing_curve = QEasingCurve::OutQuad;
//    static QPropertyAnimation *animation_right = NULL;
//    if(animation_right =  = NULL)
//    {
//        animation_right = new QPropertyAnimation(m_arrowRight,"windowOpacity");
//        animation_right->setDuration(duration);
//        animation_right->setEasingCurve(esasing_curve);
//    }

//    static QPropertyAnimation *animation_left = NULL;
//    if(animation_left =  = NULL)
//    {
//        animation_left = new QPropertyAnimation(m_arrowLeft,"windowOpacity");
//        animation_left->setDuration(duration);
//        animation_left->setEasingCurve(esasing_curve);
//    }

//    if(animation_left->state() =  = QPropertyAnimation::Running)
//    {
//        QEventLoop ep;
//        connect(animation_left,SIGNAL(finished()),&ep,SLOT(quit()));
//        ep.exec();
//    }

//    int start,end;
//    if(visible =  = true)
//    {
//        m_arrowRight->show();
//        m_arrowLeft->show();
//        start = 0.2,end = 1.0;
//    }
//    else
//    {
//        m_arrowRight->show();
//        m_arrowLeft->show();
//        start = 1.0,end = 0.0;
//    }

//    animation_right->setStartValue(start);
//    animation_right->setEndValue(end);
//    animation_left->setStartValue(start);
//    animation_left->setEndValue(end);

//    static QParallelAnimationGroup *group = NULL;
//    if(group =  = NULL)
//    {
//        group = new QParallelAnimationGroup();
//        group->addAnimation(animation_left);
//        group->addAnimation(animation_right);
//    }
//    group->start();
}


