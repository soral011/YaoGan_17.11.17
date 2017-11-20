/***********************************************************
**
** class: ViewCarPic
**
** 作者：lth
** 日期：2017-07-25
**
** 查看监测结果车辆图片
**
************************************************************/

#ifndef VIEWCARPICWIDGET_H
#define VIEWCARPICWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QPushButton>

namespace Ui {
class ViewCarPicWidget;
}

class ViewCarPic : public QWidget
{
    Q_OBJECT

public:
    explicit ViewCarPic(QWidget *parent = 0);
    ~ViewCarPic();

    QPixmap m_pixmap;
    QString m_license;
    QPushButton *m_arrowRight;
    QPushButton *m_arrowLeft;

private:
    Ui::ViewCarPicWidget *ui;

//    QTimer timer;
    bool   m_isRight;
    QPoint m_relativePos;

signals:
    void switchItem(bool isDown);

private slots:
    void showCarInfo();
    void updateArrowPosition();
    void buttonReleased(QWidget *w);
    void mouseMoveEvent(QMouseEvent *event);
    void leaveEvent(QEvent *event);
    void resizeEvent(QResizeEvent * event);
    void setArrowButtonVisible(bool visible);
    void mouseReleaseEvent(QMouseEvent * e);
    void mousePressEvent(QMouseEvent *event);
};

#endif // VIEWCARPICWIDGET_H
