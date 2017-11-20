/***********************************************************
**
** class:
**
** 作者：lth
** 日期：2017-07-25
**
**
**
************************************************************/

#ifndef SHOWTIPSWIDGET_H
#define SHOWTIPSWIDGET_H

#include <QWidget>
#include <QPainter>
#include <math.h>
#include <QLabel>

namespace Ui {
class ShowTipsWidget;
}

class RunningStatus : public QWidget
{
    Q_OBJECT

public:
    explicit RunningStatus(QWidget *parent = 0);
    ~RunningStatus();
    QLabel *m_tipsLabel;

private:
    Ui::ShowTipsWidget *ui;
    QStringList m_tipList;

private slots:
    void paintEvent(QPaintEvent *event);
    void showTips(QString tips);

};

#endif // SHOWTIPSWIDGET_H
