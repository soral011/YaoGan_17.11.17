/****************************************************************************
**
** Author：lth
** Date：2015-05-14
** Description：显示3D提示效果
**
****************************************************************************/

#ifndef TIPS3DDIALOG_H
#define TIPS3DDIALOG_H

#include <QDialog>
#include <QImage>

#define LOG_HERE

namespace Ui {
    class Tips3DDialog;
}

class Tips3DDialog : public QDialog
{
    Q_OBJECT

public:
    explicit Tips3DDialog(QWidget *parent = 0);
    ~Tips3DDialog();
    //ui->label_tips的4个顶点
    QPoint m_pos_a;
    QPoint m_pos_b;
    QPoint m_pos_c;
    QPoint m_pos_d;

    QPoint m_pos_des_down;
    QPoint m_pos_des_up;//箭头往上指还往下

    QRect  m_des_w_rect; //在箭头指向的目标控件上画一个不同颜色的方框

    QWidget *m_widget_goal; //箭头指向的目标控件
    QWidget *m_widget_parent; //该对象的父控件

    QImage m_image; //经过一番计算后需要绘制的图像

    int m_errorSign; //出错标识

    void showMessage(QString message,QWidget *focus_widget);

private:
    Ui::Tips3DDialog *ui;

    void updatePosition(QWidget *w);

public slots:
    void draw3DShadow();

protected:
    virtual void paintEvent(QPaintEvent *);

private slots:
    void on_pushButton_released();
    void buttonReleased();
};

#endif // TIPS3DDIALOG_H
