/***********************************************************
**
** class: TdlasSettings
**
** 作者：lth
** 日期：2017-07-25
**
** TDLAS 系统参数配置界面
**
************************************************************/

#ifndef TDLASSETTINGS_H
#define TDLASSETTINGS_H

#include <QWidget>
#include "SocketClient.h"

namespace Ui {
class TdlasSettings;
}

struct TdlasSettingsData
{
    bool          hasData;
    unsigned char workMode;        //工作模式
    float         temperatre;      //固定温度，2字节
    unsigned char scanCurrentStart;//扫描电流起始值
    unsigned char scanCurrentRange;//扫描电流跨度
    unsigned char current;         //电流调制值
    int           phase;           //相位
    unsigned char gain;            //增益
    unsigned char averageNum;      //平均次数
};

class TdlasSettings : public QWidget
{
    Q_OBJECT

public:
    explicit TdlasSettings(QWidget *parent = 0);
    ~TdlasSettings();

private:
    Ui::TdlasSettings *ui;
    QByteArray m_currentSettings;
    QByteArray m_backupSettings;
    volatile bool m_isSpinValueChanged;
    QTimer   m_updateTimer;

private:
    TdlasSettingsData parseData(QByteArray settings);
    void showSettings(TdlasSettingsData data);
    void setSpinBoxEnabled(bool enabled);
    void addOperationRecord(QString text);

private slots:
    void buttonReleased(QWidget * );
    void dataPacketReady(SocketPacket packet);
    void dSpinBoxValueChanged(double value);
    void hasClient(bool value);
    void updateTesterInfo();
    void updateSettings();
};

#endif // TDLASSETTINGS_H
