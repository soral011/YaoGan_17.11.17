/***********************************************************
**
** class: Settings
**
** 作者：lth
** 日期：2017-07-25
**
** 设置界面
**
************************************************************/

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWidget>
#include <QMouseEvent>
#include <QPainter>

namespace Ui {
class Settings;
}

class Settings : public QWidget
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = 0);
    ~Settings();

private:
    Ui::Settings *ui;

    QString m_checkedStyle,m_uncheckedStyle;

    QPoint  m_relativePos;

    void updateComList();
    void updateCameraList();

private slots:
    void buttonReleased(QWidget *w);

    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void showEvent(QShowEvent *event);
    void paintEvent(QPaintEvent *event);

    void comboBoxActivated(QString value);
    void availableCameraReady(QStringList cameraIpList);
    void on_comboBox_camera_ip_currentIndexChanged(const QString &arg1);
    bool eventFilter(QObject * watched, QEvent * event);
};

#endif // SETTINGS_H

