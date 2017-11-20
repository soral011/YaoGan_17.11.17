/***********************************************************
**
** class: AdjustCamera
**
** 作者：lth
** 日期：2017-07-25
**
** 浏览抓拍机视频、调节云台、焦距等 UI 操作界面
**
************************************************************/

#ifndef ADJUSTCAMERA_H
#define ADJUSTCAMERA_H

#include <QWidget>
#include "Camera.h"
#include "Global.h"
#include "PlateRecogRegion.h"
#include <QMouseEvent>
#include <QSignalMapper>
#include <QShowEvent>
#include <QMessageBox>

namespace Ui {
class AdjustCamera;
}

class AdjustCamera : public QDialog
{
    Q_OBJECT

protected://构造函数是保护型的，此类使用单例模式
    explicit AdjustCamera();
    ~AdjustCamera();

private:
    Ui::AdjustCamera *ui;
    Camera *m_camera;
    PlateRecogRegion *m_plateRecogRegion;
    QSignalMapper m_signalMapper;

public:
    static AdjustCamera *getInstance();
    bool isAdjustingCamera();

private:
    void setYunTaiControlVisible(bool value);

signals:
    void controlYunTai(DWORD dwPTZCommand,DWORD dwStop);

private slots:
    void buttonReleased(QWidget *w);
    void stopYunTai();
    void turnYunTai();
    void mouseMoveEvent(QMouseEvent *event);
    void isTimeToHide();
    void showEvent(QShowEvent * event);
    void closeEvent(QCloseEvent * event);
    void hideEvent(QHideEvent * event);
};

#endif // ADJUSTCAMERA_H
