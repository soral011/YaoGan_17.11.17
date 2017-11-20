/***********************************************************
**
** class: RealTimeTest
**
** 作者：lth
** 日期：2017-07-25
**
** 实时测量界面
**
************************************************************/

#ifndef REALTIMETEST_H
#define REALTIMETEST_H

#include <QWidget>
#include "SocketConstant.h"
#include "NetDataStructure.h"
#include "tools/Tips3DDialog.h"
#include "tools/MyMessageDialog.h"
#include "tools/ProgressDialog.h"
#include "SystemStatus.h"
#include "MultiCaliDialog.h"

namespace Ui {
class RealTimeTest;
}

class RealTimeTest : public QWidget
{
    Q_OBJECT

public:
    explicit RealTimeTest(QWidget *parent = 0);
    ~RealTimeTest();  

private:
    Ui::RealTimeTest *ui;
    bool            m_hasClient;
    bool            m_isSavingBg;
    Tips3DDialog    m_tips3DDialog;
    MyMessageDialog m_messageDialog;
    ProgressDialog  m_progressDialog;
    QTimer          m_saveBgSpectrumTimer;
    SystemStatus   *m_systemStatus; // 这是一个单例模式的指针不要删除
//    MultiCaliDialog m_multiCaliDialog;

    void showResultText(QStringList resultText );
    void controlIO();
    void setEnabledWidgets(bool value);
    void copyOpacityData(QString sectionName);

signals:
    void send(QByteArray data,QByteArray dataType);

private slots:
    void hasClient(bool value);
    void dataPacketReady(SocketPacket packet);
    void on_checkBox_real_time_test_clicked(bool checked);
    void buttonReleased(QWidget *w);
    void on_checkBox_clear_gas_clicked();
    void on_checkBox_inflate_NO_clicked();
    void on_checkBox_inflate_HC_CO_CO2_clicked();
    void on_pushButton_save_bg_spectrum_clicked();
    void cancelSavingBgSpectrum();
    void saveBgSpectrum();
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent * event);
    void mousePressEvent(QMouseEvent * event);
    void updateTesterInfo();
};

#endif // REALTIMETEST_H
