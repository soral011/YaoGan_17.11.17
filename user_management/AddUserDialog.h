/***********************************************************
**
** class: AddUserDialog
**
** 作者：lth
** 日期：2017-07-25
**
** 添加用户对话框
**
************************************************************/

#ifndef ADDUSERDIALOG_H
#define ADDUSERDIALOG_H

#include <QDialog>
#include <QKeyEvent>
#include "tools/Tips3DDialog.h"



#define HKEY_USER_NAME          "user_name"
#define HKEY_USER_PASSWORD      "user_password"
#define HKEY_USER_CHINESE_NAME  "user_chinese_name"
#define HKEY_USER_DESCRIPTION   "user_discription"

//[2017-03-21_140135]
//user_name=
//user_password=
//user_chinese_name=
//user_discription=

//[2017-03-21_140502]
//user_name=
//user_password=
//user_chinese_name=
//user_discription=

class UserStruct
{
public:
    QString m_userName;
    QString m_userPassword;
    QString m_userChineseName;
    QString m_userDiscription;
};

namespace Ui {
class AddUserDialog;
}

class AddUserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddUserDialog(QWidget *parent = 0);
    ~AddUserDialog();
    static QString getUserMD5(UserStruct user);

private:
    Ui::AddUserDialog *ui;

    QPoint m_relativePos;
    Tips3DDialog m_tips3DDialog;

private:
    bool isUserValid(UserStruct user);
    bool isPasswordValid();
    bool isChineseNameValid(QString chineseName);

signals:
    void addUserReady(UserStruct user);

private slots:
    void buttonReleased(QWidget *w);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void showEvent(QShowEvent *event);
};

#endif // ADDUSERDIALOG_H
