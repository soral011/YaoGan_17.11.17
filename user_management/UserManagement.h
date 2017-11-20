/***********************************************************
**
** class: UserManagement
**
** 作者：lth
** 日期：2017-07-25
**
** 用户管理
**
************************************************************/

#ifndef USERMANAGEMENT_H
#define USERMANAGEMENT_H

#include <QDialog>
#include "AddUserDialog.h"
#include "ChangePassword.h"
#include <QSettings>


namespace Ui {
class UserManagement;
}

class UserManagement : public QDialog
{
    Q_OBJECT

public:
    explicit UserManagement(QWidget *parent = 0);
    ~UserManagement();

public:
    bool isUserValid(UserStruct *user);

private:
    Ui::UserManagement *ui;
    QPoint     m_relativePos;
    QSettings *m_userSettings;
    QList<UserStruct> m_userList;
    AddUserDialog     m_addUserDialog;

private:
    void readUser();
    void saveUser();
    void showUser();

private slots:
    void buttonReleased(QWidget *w);
    void paintEvent(QPaintEvent *event);
    void addUserReady(UserStruct user);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
};

#endif // USERMANAGEMENT_H
