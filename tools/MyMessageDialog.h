/****************************************************************************
**
** 作者：lth
** 日期：2015-05-13
** 描述：自定义带忽略、取消、确认按钮的的对话框
**
****************************************************************************/

#ifndef MYMESSAGEDIALOG_H
#define MYMESSAGEDIALOG_H

#include <QDialog>

#define LOG_HERE

namespace Ui {
    class MyMessageDialog;
}

class MyMessageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MyMessageDialog(QWidget *parent = 0);

    ~MyMessageDialog();

    enum Icon { NoIcon, Question, Information, Warning };

    Icon icon; //提示图标
    QString exitCode; //记录点击了哪个按钮
//    QString string_Sure;
//    QString string_Discard;
    bool click_to_destroy; //点击任意按钮后是否销毁对象

    void setMessage(QString message=" ",QString title=" ",MyMessageDialog::Icon icon=MyMessageDialog::NoIcon);
    void setVisible_CancelButton(bool isShow);
    void setVisible_SureButton(bool isShow);
    void setButtonsName(QString discardName=" ",QString cancelName=" ",QString sureName=" ");

private:
    Ui::MyMessageDialog *ui;

signals:
    //按钮被点击
    void sure_Clicked();
    void cancel_Clicked();
    void discard_Clicked();
    void buttonClicked(QString);
    void buttonClicked();
    void sendString_Sure(QString);
    void sendString_Discard(QString);

private slots:
    void on_pushButton_discard_released();
    void on_pushButton_cancel_released();
    void on_pushButton_sure_released();
};

#endif // MYMESSAGEDIALOG_H
