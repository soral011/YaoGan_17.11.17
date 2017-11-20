/***********************************************************
**
** class: PlateRecogRegion
**
** 作者：lth
** 日期：2017-07-25
**
** 牌识区调节界面
**
************************************************************/

#ifndef PLATERECOGREGION_H
#define PLATERECOGREGION_H

#include <QWidget>
#include <QMouseEvent>

namespace Ui {
class PlateRecogRegion;
}

class PlateRecogRegion : public QWidget
{
    Q_OBJECT

public:
    explicit PlateRecogRegion(QWidget *parent = 0);
    ~PlateRecogRegion();

private:
    Ui::PlateRecogRegion *ui;

    QPoint m_P1_New;//P1代表合格框左上角，P2代表合格框右上角，P3代表合格框右下角
    QPoint m_P2_New;
    QPoint m_P3_New;
    QPoint m_P4_New;
    QPoint m_P_Top;
    QPoint m_P_Right;
    float  m_factorX;
    float  m_factorY;

private slots:
    void mouseMoveEvent(QMouseEvent * event);
    void mousePressEvent(QMouseEvent *event);
    void updateRecogRegion();

};

#endif // PLATERECOGREGION_H
