/****************************************************************************
**
** 作者：lth
** 日期：2015-05-13
** 描述：构建进度对话框
**
****************************************************************************/

#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <stdio.h>
#include <QPainter>
#include <QRunnable>
#include <QThread>
#include "ProgressBar.h"


namespace Ui {
    class ProgressDialog;
}

class ProgressDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ProgressDialog(QWidget *parent = 0);
    ~ProgressDialog();
    bool m_touchToDisppear; //点击该界面是否立即销毁该对象
    bool m_isShowingCancelBtn; //是否显示界面中的取消按钮
    QString m_msg;//提示信息

    void showMessage(QString m_msg,QString m_position="center");
    void showMessage(QString m_msg,QWidget *parent,QString m_position="center");

private:
    Ui::ProgressDialog *ui;

    QTimer m_updateTimer; //判断是否该隐藏圆圈progress_bar了

    QString m_position; //显示的位置，上、中、下

    ProgressBar m_progressBar; //圆圈

    void disappear();
    void updatePosition(QString m_position);

signals:
    void mousePress(); //界面被点击了
    void cancelClicked(); //取消按钮被点击了

private slots:
    void on_pushButton_cancel_clicked();
    void updateState();
    void updateText(QString text);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void showEvent ( QShowEvent * event );

};

#endif // PROGRESSDIALOG_H
