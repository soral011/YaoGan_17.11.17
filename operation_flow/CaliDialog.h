/***********************************************************
**
** class: CaliDialog
**
** 作者：lth
** 日期：2017-07-25
**
** （用户）标定操作界面
**
************************************************************/

#ifndef CALIDIALOG_H
#define CALIDIALOG_H

#include <QDialog>
#include "Global.h"
#include "SocketClient.h"
#include "tools/ProgressDialog.h"

namespace Ui {
class CaliDialog;
}

class CaliDialog : public QDialog
{
    Q_OBJECT

public://不要设计为单例模式，否则在关闭程序时出现：程序异常结束
    explicit CaliDialog(QWidget *parent = 0);
    ~CaliDialog();

    bool isCalibrating();

private:
    Ui::CaliDialog *ui;
    SocketClient  *m_socketClient;
    ProgressDialog m_progressDialog;
    QSize m_oldSize;
    QPoint m_relativePos;

signals:
    void send(QByteArray data,QByteArray dataType);
    void calibrationFinished(bool succeeded);

private slots:
    void buttonReleased(QWidget *w);
    void cancelCalibration();
    void dataPacketReady(SocketPacket packet);
    void showEvent(QShowEvent * event);
    void closeEvent(QCloseEvent * event);
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    void testHostStatus(bool isConnected);
    void stopCalibration();
    void mouseReleaseEvent(QMouseEvent * event);
};

#endif // CALIDIALOG_H
