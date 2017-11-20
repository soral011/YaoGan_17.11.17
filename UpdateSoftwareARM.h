/***********************************************************
**
** class: UpdateSoftwareARM
**
** 作者：lth
** 日期：2017-07-25
**
** 测量主机更新界面
**
************************************************************/

#ifndef UPDATESOFTWAREARM_H
#define UPDATESOFTWAREARM_H

#include <QDialog>
#include <QMouseEvent>

namespace Ui {
class UpdateSoftwareARM;
}

class UpdateSoftwareARM : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateSoftwareARM(QWidget *parent = 0);
    ~UpdateSoftwareARM();

private:
    Ui::UpdateSoftwareARM *ui;
    QPoint m_relativePos;


private slots:
    void paintEvent(QPaintEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    void updatePercent(int percent);
    void buttonReleased(QWidget *w);
    void keyPressEvent(QKeyEvent *event);
};

#endif // UPDATESOFTWAREARM_H
