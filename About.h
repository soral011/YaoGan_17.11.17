/***********************************************************
**
** class:About
**
** 作者：lth
** 日期：2017-07-25
**
** “关于”显示界面
**
************************************************************/

#ifndef ABOUT_H
#define ABOUT_H

#include <QMouseEvent>
#include <QWidget>

namespace Ui {
class About;
}

class About : public QWidget
{
    Q_OBJECT

public:
    explicit About(QWidget *parent = 0);
    ~About();

private:
    Ui::About *ui;

    QPoint m_relativePos;

private slots:
    void paintEvent(QPaintEvent *event);
    void on_pushButton_close_clicked();

    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
};

#endif // ABOUT_H
