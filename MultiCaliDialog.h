/***********************************************************
**
** class: MultiCaliDialog
**
** 作者：lth
** 日期：2017-07-25
**
** 多点标定界面
**
************************************************************/

#ifndef MULTICALIDIALOG_H
#define MULTICALIDIALOG_H

#include <QDialog>
#include "Global.h"
#include "SocketClient.h"
#include "tools/ProgressDialog.h"
#include "operation_flow/CaliDialog.h"

#define  SAVE_CALI_VALUE_NUM  15 //保存多少个标定数据

#define  MUTI_CALI_SETTINGS_FILE QString("config/MutiCaliSettings.ini")

namespace Ui {
class MultiCaliDialog;
}

class MultiCaliDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MultiCaliDialog(QWidget *parent = 0);
    ~MultiCaliDialog();

private:
    Ui::MultiCaliDialog *ui;

    CaliDialog      m_caliDialog;
    SocketClient   *m_socketClient;
    ProgressDialog  m_progressDialog;
    QTimer          m_saveCaliDataTimer;
    bool            m_isSavingCaliData;
    QPushButton    *m_button;
    QList<double>   m_caliDataList;
    QMap<QPushButton *, QLineEdit *> m_buttonAndeditMap;
    QSettings *m_mutiCaliSettings;

private:
    void readSettings(QWidget *widget);
    void saveSettings(QWidget *widget);
    void showCaliData(double value);
    void saveCliData(NetExhaustResults exhaustResults);
    void showResultText(QStringList resultText );
    void calcPloyfit(QList<float> xList,
                      QList<float> yList,
                               int poly_n,
                            double *args);

private slots:
    void dataPacketReady(SocketPacket socketPackage);
    void buttonReleased(QWidget *w);
    void cancelSaving();

    void on_pushButton_HC_Cali_finish_clicked();
    void on_pushButton_NO_Cali_finish_clicked();
    void on_pushButton_CO2_Cali_finish_clicked();
    void on_pushButton_clear_plain_text_clicked();
    void setCsliWidgetEnebled(bool enabled);
    void caliFinished(bool value);
    void hasClient(bool value);
    void showEvent(QShowEvent *event);
    void closeEvent(QCloseEvent *event);
    void on_pushButton_HC_Cali_l_clicked();
    void on_pushButton_HC_Cali_2_clicked();
    void on_pushButton_HC_Cali_3_clicked();
};

#endif // MULTICALIDIALOG_H
