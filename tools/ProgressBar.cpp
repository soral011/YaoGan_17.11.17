#include "ProgressBar.h"
#include <QDebug>
#include <QRgb>
//#include "CommonFuntions.h"

ProgressBar::ProgressBar(QWidget *parent):QLabel(parent)
{
    LOG_HERE("");
    this->setWindowFlags(Qt::FramelessWindowHint);
    connect(&m_updateTimer,SIGNAL(timeout()),this,SLOT(updateProcess()));
    if(parent != 0)
    {
        this->resize(parent->size());
        this->setParent(parent);
    }
    m_angle  =  0;

    m_barColor = QColor(150,150,150);
    m_barPadding = 10;
    m_barWidth = 3;
    m_parentBackup = NULL;
    m_displayPosition = ProgressBar::center;
    this->setStyleSheet("background-color:transparent;");//否则有时受父控件的style影响而得不到透明背景
}

ProgressBar::~ProgressBar()
{
    //假如把parent_backup设置为parent，先this->setParent(0)，否则可能会重复删除this
    this->setParent(0);
    delete m_parentBackup;
    LOG_HERE("");
}

/*************************************************
** Function: updateProcess
** Description: 定时器时间到，使圆圈转动一定角度
** Input:
** Output:
** Return:
*************************************************/
void ProgressBar::updateProcess()
{
    if(this->isVisible()  == false)
    {
        LOG_HERE("this->isVisible()  == false");
        if(m_parentBackup != NULL)
            m_parentBackup->hide();
        m_updateTimer.stop();
//        this->setParent(0);
        return;
    }
//    this->show();

//    qDebug()<<"angle = "<<angle;
    int width = this->width();
    int height = this->height();

    QPixmap pixmap(width,height);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
//    //create gradient
    int r = m_barColor.red();
    int g = m_barColor.green();
    int b = m_barColor.blue();
    int position_x = width/2;

    //diameter = 直径
    int bar_diameter = qMin(width,height)-m_barPadding;

    if(m_displayPosition  == ProgressBar::center)
        position_x = width/2;
    else if(m_displayPosition  == ProgressBar::left)
        position_x = bar_diameter/2+10;
    else if(m_displayPosition  == ProgressBar::right)
        position_x = width-10-bar_diameter/2;

    QConicalGradient conical_gradient(position_x, height/2,m_angle);
    conical_gradient.setColorAt(1, QColor(r,g,b,0)); //这三行里到QColor(255,255,255,x)，3 个255随便值，关键是第四位的值
    conical_gradient.setColorAt(0.5, QColor(r,g,b,80));
    conical_gradient.setColorAt(0, QColor(r,g,b,235));

    painter.setPen(QPen(QBrush(conical_gradient),m_barWidth));
    painter.drawEllipse(QPoint(position_x, height/2), bar_diameter/2, bar_diameter/2);

    painter.end();

    this->setPixmap(pixmap);
    m_angle -= 3;
    if(m_angle<-360)
        m_angle = 0;
}

/*************************************************
** Function: display
** Description: 开始显示圆圈
** Input: parent -- 在指定的控件上显示
**        color -- 圆圈显示指定的颜色
**        padding -- 用于控制圆圈的大小
** Output:
** Return:
*************************************************/
void ProgressBar::display(QWidget *parent,QColor color,int padding)
{
    LOG_HERE("");
//    qDebug()<<"display(QWidget *parent,QColor color,int padding)";
    m_barColor = color;
    m_barPadding = padding;

    this->setParent(parent);

    this->resize(parent->size());
    this->show();
}

void ProgressBar::display(QWidget *parent,int padding)
{
    LOG_HERE("");
    display(parent,Qt::green,padding);
}

void ProgressBar::display(QWidget *parent,DisplayPosition display_position_tmp,int padding)
{
    LOG_HERE("");
    m_displayPosition = display_position_tmp;
    display(parent,Qt::white,padding);
}

void ProgressBar::showEvent( QShowEvent * event)
{
    LOG_HERE("");
//    qDebug()<<"showEvent";
    m_updateTimer.start(15);
}
