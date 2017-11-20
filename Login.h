/***********************************************************
**
** class: Login
**
** 作者：lth
** 日期：2017-07-25
**
** 用户登录界面
**
************************************************************/

#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QMouseEvent>
#include "user_management/UserManagement.h"

namespace Ui {
class Login;
}

class User
{
public:
    QString name;
    QString pass;
    QString ID;
};

class Login : public QWidget
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = 0);
    ~Login();

private:
    Ui::Login *ui;

    QPoint  m_relativePos;

private:
    bool login();

signals:
    void loginFinished();

private slots:
//    void paintEvent(QPaintEvent *event);
//    void mousePressEvent(QMouseEvent *event);
//    void mouseMoveEvent(QMouseEvent *event);
    void buttonReleased(QWidget *w);
    void keyPressEvent(QKeyEvent  *event);
};

#endif // LOGIN_H
