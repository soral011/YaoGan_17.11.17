#include "Login.h"
#include "ui_Login.h"
#include "Global.h"
#include "MyDebug.cpp"
#include <QCryptographicHash>
#include "AccessDatabase.h"

Login::Login(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
//    this->setAttribute(Qt::WA_TranslucentBackground);
//    ui->widget_parent->setAutoFillBackground(true); //否则窗口会变透明
//    ui->stackedWidget->setAutoFillBackground(true);
    ui->stackedWidget->setCurrentIndex(0);

    this->showMaximized();

    //密码输入框只能输入数字和字母，长度为6
    QRegExp regx("[a-zA-Z0-9]+");
    QRegExpValidator *validator = new QRegExpValidator(regx, ui->lineEdit_user_password);
    connect(this, SIGNAL(destroyed()), validator, SLOT(deleteLater()));
    ui->lineEdit_user_password->setValidator(validator);
    ui->lineEdit_user_password->setMaxLength(6);
    ui->lineEdit_user_name->setFocus();

    g_connectButtonSignal(this);
    g_clearPushButtonFocus(this);
}

Login::~Login()
{
    delete ui;
}

void Login::buttonReleased(QWidget *w)
{
    QString buttonName = w->objectName();

    if(buttonName == "pushButton_close")
    {
//        g_closeWithAnim(this);
        this->close();
    }
    else if(buttonName == "pushButton_login")
    {
        if (!login())
        {
            ui->lineEdit_user_password->selectAll();
            MY_DEBUG("");
            return;
        }

//        connect(g_closeWithAnim(this),SIGNAL(finished())
//                ,this, SIGNAL(loginFinished()));
        this->close();
        emit loginFinished();

    }
}


bool Login::login()
{
    UserManagement userManagement;
    UserStruct user;
    user.m_userName = ui->lineEdit_user_name->text();
    user.m_userPassword = g_codeMd5(ui->lineEdit_user_password->text());
    bool isUserValid = userManagement.isUserValid(&user);
//    if(isUserValid)
//    {
//        g_currentUser = user;

//        AccessDatabase accessDatabase;
//        accessDatabase.saveCurrentUser(user);

//        return true;
//    }
//    else
//    {
//        g_showTipsDialog(tr("用户名或密码错误！"), this);
//        return false;
//    }
    return true;
}

void Login::keyPressEvent(QKeyEvent  *event)
{
    if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        if(ui->stackedWidget->currentIndex() == 0)
        {
            this->buttonReleased(ui->pushButton_login);
        }
    }
}


