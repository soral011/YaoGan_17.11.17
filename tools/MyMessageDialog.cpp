#include "MyMessageDialog.h"
#include "ui_MyMessageDialog.h"
#include <QSettings>
#include <QDebug>
//#include "CommonFuntions.h"

MyMessageDialog::MyMessageDialog(QWidget *parent) :
    QDialog(parent),ui(new Ui::MyMessageDialog)
{
    this->setWindowFlags(Qt::WindowCloseButtonHint);

    ui->setupUi(this);
    ui->label_show_title->hide();
    ui->pushButton_discard->setVisible(false);
    click_to_destroy = false;

//    g_clearPushButtonFocus(this);
}

MyMessageDialog::~MyMessageDialog()
{
    delete ui;
}

/*************************************************
** Function: setMessage
** Description: 显示指定信息
** Input: message -- 指定信息
**        title -- 对话框显示标题
**        icon -- 显示图标
** Output:
** Return:
*************************************************/
void MyMessageDialog::setMessage(QString message,QString title,MyMessageDialog::Icon icon)
{
    ui->label_show_message->setText(message);
    if(title!=" ")
    {
        ui->label_show_title->show();
        ui->label_show_title->setText(title);
    }
    QString style="border-image: url(:/images/images/messagebox_";//question.png);";

    if(icon==MyMessageDialog::NoIcon)
    {
        ui->label_message_icon->hide();
        return;
    }
    else if(icon==MyMessageDialog::Question)
    {
        style+="question";
    }
    else if(icon==MyMessageDialog::Information)
    {
        style+="infor";
    }
    else if(icon==MyMessageDialog::Warning)
    {
        style+="warning";
    }

    style+=".png);";
    style+="background-color: rgba(255, 255, 255, 0);";
    ui->label_message_icon->setStyleSheet(style);
    ui->label_message_icon->show();
}

/*************************************************
** Function: setButtonsName
** Description: 设置对话框上三个按钮上的文字
** Input:
** Output:
** Return:
*************************************************/
void MyMessageDialog::setButtonsName(QString discardName,QString cancelName,QString sureName)
{
//    ui->pushButton_cancel->setVisible(isShow);
    if(discardName!=" ")
    {
        ui->pushButton_discard->setText(discardName);
        ui->pushButton_discard->setVisible(true);
    }
    if(cancelName!=" ")
    {
        ui->pushButton_cancel->setText(cancelName);
    }
    if(sureName!=" ")
    {
        ui->pushButton_sure->setText(sureName);
    }
}

void MyMessageDialog::setVisible_CancelButton(bool isShow)
{
    ui->pushButton_cancel->setVisible(isShow);
}

void MyMessageDialog::setVisible_SureButton(bool isShow)
{
    ui->pushButton_sure->setVisible(isShow);
}

void MyMessageDialog::on_pushButton_sure_released()
{

    LOG_HERE("");
    exitCode="sure";
    emit sure_Clicked();
    emit buttonClicked();
//    if(string_Sure.isEmpty()==false)
//        emit sendString_Sure(string_Sure);
    if(click_to_destroy==true){
        this->deleteLater();
        return;
    }
    this->hide();
}

void MyMessageDialog::on_pushButton_cancel_released()
{

    LOG_HERE("");
    exitCode="cancel";
    emit cancel_Clicked();
    emit buttonClicked();
    if(click_to_destroy==true){
        this->deleteLater();
        return;
    }
    this->hide();
}

void MyMessageDialog::on_pushButton_discard_released()
{

    LOG_HERE("");
    exitCode="discard";
    emit discard_Clicked();
    emit buttonClicked();
//    if(string_Discard.isEmpty()==false)
//        emit sendString_Discard(string_Discard);
    if(click_to_destroy==true){
        this->deleteLater();
        return;
    }
    this->hide();
}
