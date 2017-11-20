/***********************************************************
**
** class: ChangePassword
**
** 作者：lth
** 日期：2017-07-25
**
** 修改用户密码对话框
**
************************************************************/

#ifndef CHANGEPASSWORD_H
#define CHANGEPASSWORD_H

#include <QDialog>
#include <QMouseEvent>
#include "AddUserDialog.h"

namespace Ui {
class ChangePassword;
}

class ChangePassword : public QDialog
{
    Q_OBJECT

public:
    explicit ChangePassword(QWidget *parent = 0);
    ~ChangePassword();

public:
    UserStruct m_user;
    bool changedOk;

private:
    Ui::ChangePassword *ui;
    QPoint m_relativePos;

public:
    bool changePassword(UserStruct user);

private:
    bool isPasswordValid();

private slots:
    void buttonReleased(QWidget *w);
    void paintEvent(QPaintEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
};

#endif // CHANGEPASSWORD_H
