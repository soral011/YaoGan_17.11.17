/****************************************************************************
**
** 作者：lth
** 日期：2015-05-13
** 描述：模仿Android圆形转动控件
**
****************************************************************************/

#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QLinearGradient>
#include <QConicalGradient>
#include <QPainterPath>
#include <QLabel>
#include <QTimer>
#include <QColor>
#include <QPoint>

#define LOG_HERE

class ProgressBar : public QLabel
{
    Q_OBJECT
public:
    ProgressBar(QWidget *parent=0);
    ~ProgressBar();

    //圆形对话框显示在界面的位置
    enum DisplayPosition{
        left,
        center,
        right};

    QColor m_barColor; //圆圈颜色
    int m_barPadding; //圆圈到控件边上的距离,用于控制圆圈的大小
    int m_barWidth; //圆圈的宽度(不是直径)

    void display(QWidget *parent,QColor color=QColor(150,150,150),int padding=10);
    void display(QWidget *parent,DisplayPosition m_displayPosition,int padding=10);
    void display(QWidget *parent,int padding);

private:
    QTimer   m_updateTimer; //圆圈转动定时器
    QWidget *m_parentBackup;
    DisplayPosition m_displayPosition;
    int m_angle;

private slots:
    void updateProcess();
    virtual void showEvent (QShowEvent * event );
};

#endif // PROGRESSBAR_H
