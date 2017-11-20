#include "MyMenu.h"
#include "ui_MyMenu.h"
#include <QDebug>
#include <QSizePolicy>
#include <QPropertyAnimation>
#include <QTimer>

MyMenu::MyMenu(QStringList buttonNames):ui(new Ui::MyMenu)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint);

    int count = buttonNames.count();
    this->resize(this->width(), 9*2 + count*30);

    foreach(QString name,buttonNames)
    {
        QPushButton *button = new QPushButton(name);
        m_buttonList.append(button);
        button->setSizePolicy(QSizePolicy::Preferred,
                              QSizePolicy::Preferred);
        ui->verticalLayout_buttons->addWidget(button);
        connect(button,SIGNAL(released()),this,SLOT(buttonClicked()));

        button->setStyleSheet("padding-left:16px;");
//        button->setFont(QFont("Microsoft YaHei"));
        button->setFocusPolicy(Qt::NoFocus);
    }

//    g_clearPushButtonFocus(this);
}

MyMenu::MyMenu(QList<MyMenuItem> menuItems):ui(new Ui::MyMenu)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint);

    int count = menuItems.count();
    this->resize(this->width(), 9*2 + count*30);

    foreach(MyMenuItem menuItem, menuItems)
    {
        QPushButton *button = new QPushButton(menuItem.m_name);
        button->setIcon(menuItem.m_icon);
        button->setIconSize(QSize(22, 22));
        button->setSizePolicy(QSizePolicy::Preferred,
                              QSizePolicy::Preferred);
        ui->verticalLayout_buttons->addWidget(button);
        connect(button,SIGNAL(released()),this,SLOT(buttonClicked()));

        button->setStyleSheet("padding-left:16px;");
//        button->setFont(QFont("Microsoft YaHei"));

        m_buttonList.append(button);
    }
}

MyMenu::~MyMenu()
{
    foreach(QPushButton *button,m_buttonList)
    {
        delete button;
    }
    qDebug()<<"~MyMenu()";
    delete ui;
}

void MyMenu::buttonClicked()
{
    QPushButton *button = qobject_cast<QPushButton *>(QObject::sender());

    emit itemClicked(button->text());
    this->hide();
    m_endRect.setWidth(0);
//    qDebug()<<"button = "<<button->text();
}

void MyMenu::showEvent(QShowEvent * event)
{
    QTimer::singleShot(10, this, SLOT(showWithAnimation()));
}

void MyMenu::showWithAnimation()
{
    int duration = 300;
    QEasingCurve curve = QEasingCurve::OutQuad/*InOutCubic*/;

    QPoint pos = this->pos();
    //第1.1个开始出现
    QPropertyAnimation *anim_geometry =
            new QPropertyAnimation(this,"geometry");
    anim_geometry->setDuration(duration);
    QRect startRect(pos.x(),pos.y(),this->width(),0);
    anim_geometry->setStartValue(startRect);

    if(m_endRect.width()  ==  0)
    {
        m_endRect = QRect(pos.x(),pos.y(),this->width(),this->height());
    }
    anim_geometry->setEndValue(m_endRect);
    anim_geometry->setEasingCurve(curve);
    anim_geometry->start();


    connect(anim_geometry, SIGNAL(finished())
            ,anim_geometry, SLOT(deleteLater()));
}
