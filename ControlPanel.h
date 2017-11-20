/***********************************************************
**
** class: ControlPanel
**
** 作者：lth
** 日期：2017-07-25
**
** 控制面板界面
**
************************************************************/

#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>
#include <QSignalMapper>
#include "SocketConstant.h"
#include "TestSpectrum.h"
#include "CalibrationData.h"
#include "TdlasSettings.h"
#include "MultiCaliDialog.h"

namespace Ui {
class ControlPanel;
}

class ControlPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ControlPanel(QWidget *parent = 0);
    ~ControlPanel();

private:
    Ui::ControlPanel *ui;
    bool             m_hasClient;
    TestSpectrum     m_testSpectrum;
    CalibrationData  m_calibrationData;
    TdlasSettings    m_tdlasSettings;
    MultiCaliDialog *m_multiCaliDialog;
    QPoint           m_relativePos;
    QSize            m_originalSize;

private:
    void ioControl();
    void setOtherToolsVisible();

signals:
    void send(QByteArray data,QByteArray dataType);
    void dataPacketReady2(SocketPacket socketPackage);

private slots:
    void buttonReleased(QWidget *w);
    void hasClient(bool value);
    void dataPacketReady(SocketPacket packet);
    void testerIpReady(QList<QString> ipList);

    void on_checkBox_pump_clicked(bool checked);
    void on_checkBox_valve1_clicked(bool checked);
    void on_checkBox_valve2_clicked(bool checked);
    void on_checkBox_start_samba_clicked(bool checked);

    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void showEvent(QShowEvent *event);
    void checkBoxReleased(QWidget * w);
};

#endif // CONTROLPANEL_H
