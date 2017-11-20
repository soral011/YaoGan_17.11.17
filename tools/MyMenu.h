/***********************************************************
**
** class: MyMenu
**
** 作者：lth
** 日期：2017-07-25
**
** 自定义菜单栏
**
************************************************************/

#ifndef MYMENU_H
#define MYMENU_H

#include <QWidget>
#include <QPushButton>
#include <QDebug>

namespace Ui {
class MyMenu;
}

class MyMenuItem
{
public:
    QString m_name;
    QIcon   m_icon;

    MyMenuItem(QString name, QIcon icon)
    {
        m_name = name;
        m_icon = icon;
        if(m_icon.isNull())
        {

            m_icon = QIcon(":/images/transparent4.png");
        }
    }

};

class MyMenu : public QWidget
{
    Q_OBJECT

public:
    MyMenu(QStringList buttonNames);
    MyMenu(QList<MyMenuItem> menuItems);
    ~MyMenu();

private:
    Ui::MyMenu *ui;

    QList<QPushButton *> m_buttonList;
    QRect m_endRect;

signals:
    void itemClicked(QString item);

private slots:
    void buttonClicked();
    void showEvent(QShowEvent * event);
    void showWithAnimation();
};

#endif // MYMENU_H
