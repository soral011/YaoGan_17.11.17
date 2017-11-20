#include "PlateRecogRegion.h"
#include "ui_PlateRecogRegion.h"
#include <QPainter>
#include <QDebug>
#include <QSize>
#include <QString>
#include "Constant.h"
#include "Global.h"
#include <QFont>

PlateRecogRegion::PlateRecogRegion(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlateRecogRegion)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint
                                   |Qt::Tool/*|Qt::SubWindow*/);
//    Qt::FramelessWindowHint加上以下这句使得窗口背景透明
    this->setAttribute(Qt::WA_TranslucentBackground, true);

//    connect(this,SIGNAL())

    ui->label->move(0,0);

    updateRecogRegion();
//    g_clearPushButtonFocus(this);
}

PlateRecogRegion::~PlateRecogRegion()
{
    delete ui;
}

void PlateRecogRegion::mouseMoveEvent(QMouseEvent * event)
{

    QPoint pos = event->pos();
    if(ui->pushButton_top->hasFocus())
    {
        if(m_P1_New.x() > pos.x())
        {
            pos.setX( m_P1_New.x() );
        }
        if(m_P2_New.x() < pos.x())
        {
            pos.setX( m_P2_New.x() );
        }
        ui->pushButton_top->move(pos.x(),
                   ui->pushButton_top->pos().y());
        m_P_Top = QPoint(pos.x(), m_P1_New.y());
    }
    else if(ui->pushButton_right->hasFocus())
    {
        if(m_P2_New.y() > pos.y())
        {
            pos.setY(m_P2_New.y());
        }
        if(m_P3_New.y() < pos.y())
        {
            pos.setY(m_P3_New.y());
        }
        ui->pushButton_right->move(ui->pushButton_right->pos().x(),
                                   pos.y()-ui->pushButton_right->height());
        m_P_Right = QPoint(m_P2_New.x(), pos.y());
    }
    updateRecogRegion();
}

void PlateRecogRegion::mousePressEvent(QMouseEvent *event)
{

}

void PlateRecogRegion::updateRecogRegion()
{
//    qDebug()<<"updateRecogRegion()!";

    QSize size = this->size();
    m_factorX = size.width() / CAPTURE_CAR_PIC_WIDTH;//1920*1080是视频的分辨率，注意不要去掉“.0”
    m_factorY = size.height() / CAPTURE_CAR_PIC_HEIGHT;
    if(ui->label->size() !=  size)
    {
        ui->label->resize(size);
        m_P1_New = QPoint(g_PlateRecogRegion_P1.x()*m_factorX
                          ,g_PlateRecogRegion_P1.y()*m_factorY);
        m_P2_New = QPoint(g_PlateRecogRegion_P2.x()*m_factorX
                          ,g_PlateRecogRegion_P2.y()*m_factorY);
        m_P3_New = QPoint(g_PlateRecogRegion_P3.x()*m_factorX
                          ,g_PlateRecogRegion_P3.y()*m_factorY);
        m_P4_New = QPoint(g_PlateRecogRegion_P4.x()*m_factorX
                          ,g_PlateRecogRegion_P4.y()*m_factorY);

        if(g_settings.value(HKEY_PLATE_RECOG_REGION_TOP).isNull())
        {
            m_P_Top = QPoint((m_P1_New.x() + m_P2_New.x())/2
                           ,m_P1_New.y());
            m_P_Right = QPoint(m_P2_New.x(),(m_P3_New.y() + m_P2_New.y())/2);
        }
        else
        {
            QStringList strList = g_settings.value(HKEY_PLATE_RECOG_REGION_TOP)
                    .toString().split(",");

            QPoint pos;
            pos.setX(strList.first().toInt());
            pos.setY(strList.last().toInt());
            pos.setX(pos.x()*m_factorX);
            pos.setX(qMax(pos.x(),m_P1_New.x()));
            pos.setX(qMin(pos.x(),m_P2_New.x()));
            m_P_Top = QPoint(pos.x(),m_P1_New.y());

            strList = g_settings.value(HKEY_PLATE_RECOG_REGION_RIGHT)
                                .toString().split(",");
            pos.setX(strList.first().toInt());
            pos.setY(strList.last().toInt());

            pos.setY(pos.y()*m_factorY);
            pos.setY(qMax(pos.y(),m_P2_New.y()));
            pos.setY(qMin(pos.y(),m_P3_New.y()));
            m_P_Right = QPoint(m_P2_New.x(),pos.y());
        }


        ui->pushButton_top->move(m_P_Top.x(),m_P1_New.y()-ui->pushButton_top->height());
        ui->pushButton_right->move(m_P_Right.x(),m_P_Right.y()-ui->pushButton_right->height());
    }

    g_PlateRecogRegion_Top = QPoint(m_P_Top.x()/m_factorX,
                                  m_P_Top.y()/m_factorY);
    g_PlateRecogRegion_Right = QPoint(m_P_Right.x()/m_factorX,
                                    m_P_Right.y()/m_factorY);

    QString str = QString("%1,%2").arg(g_PlateRecogRegion_Top.x())
                              .arg(g_PlateRecogRegion_Top.y());
    g_settings.setValue(HKEY_PLATE_RECOG_REGION_TOP,str);

    str = QString("%1,%2").arg(g_PlateRecogRegion_Right.x())
                                  .arg(g_PlateRecogRegion_Right.y());
    g_settings.setValue(HKEY_PLATE_RECOG_REGION_RIGHT,str);

    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);

    QPen pen(Qt::red);
    pen.setWidth(1);
    QVector<qreal> dashes;
    qreal space = 8;
    dashes << 15 << space << 15 <<space;

    pen.setDashPattern(dashes);
    painter.setPen(pen);

    QVector<QPoint> points;
    points<<m_P1_New<<m_P2_New<<m_P3_New<<m_P4_New;

    QPolygon polygon(points);
    painter.drawPolygon(polygon);

    pen = QPen(Qt::yellow);
    pen.setWidth(2);
    painter.setPen(pen);

    points.clear();
    points<<m_P1_New<<m_P_Top<<m_P_Right<<m_P3_New<<m_P4_New;
    polygon.clear();
    polygon.append(points);
    painter.drawPolygon(polygon);

    QFont font;
    font.setPointSize(11);
    painter.setFont(font);
    painter.drawText(QPointF(size.width() / 2 - 20, size.height() / 2 + 25)
                             ,tr("牌识区"));

    painter.end();
    ui->label->setPixmap(pixmap);

}


