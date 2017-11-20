#include "ChangePassword.h"
#include "ui_ChangePassword.h"
#include "Global.h"

ChangePassword::ChangePassword(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangePassword)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    ui->widget_parent->setAutoFillBackground(true);

    changedOk = false;

    g_connectButtonSignal(this);

    g_clearPushButtonFocus(this);
}

ChangePassword::~ChangePassword()
{
    delete ui;
}


void ChangePassword::buttonReleased(QWidget *w)
{
    QString buttonName = w->objectName();

    if(buttonName == "pushButton_ok")
    {
        bool isOk = isPasswordValid();
        if(isOk)
        {
            m_user.m_userPassword = g_codeMd5(ui->lineEdit_password->text());
            changedOk = true;
            this->close();
        }
    }
    else if(buttonName == "pushButton_close" || buttonName == "pushButton_cancel")
    {
        g_closeWithAnim(this);
    }
}

void ChangePassword::paintEvent(QPaintEvent *event)
{
    g_drawShadowBorder(this);
}

void ChangePassword::mousePressEvent(QMouseEvent *event)
{
    m_relativePos = this->pos()-event->globalPos();
}

void ChangePassword::mouseMoveEvent(QMouseEvent *event)
{
    this->move(event->globalPos()+ m_relativePos);
}

bool ChangePassword::changePassword(UserStruct user)
{
    m_user = user;
    ui->lineEdit_user_name->setText(m_user.m_userName);
    this->exec();
}


bool ChangePassword::isPasswordValid()
{
    QString tmpPassword = g_codeMd5(ui->lineEdit_old_password->text());

    if(tmpPassword != m_user.m_userPassword)
    {
        g_showTipsDialog(tr("原密码输入错误。"),  this);
        return false;
    }

    QString password = ui->lineEdit_password->text();
    QString confirmPassword = ui->lineEdit_password_confirm->text();
    if(password != confirmPassword)
    {
        g_showTipsDialog(tr("新密码不匹配。"), this);
        return false;
    }

    QStringList passwordList;
    passwordList << password << confirmPassword;
    foreach(QString pass, passwordList)
    {
        if(pass.isEmpty() || pass.remove(" ").isEmpty())
        {
            g_showTipsDialog(tr("密码不能为空。"), this);
            return false;
        }

        QString tmpPassword = pass;
        tmpPassword.remove(QRegExp("[0-9]"));
        tmpPassword.remove(QRegExp("[a-z]"));
        tmpPassword.remove(QRegExp("[A-Z]"));
        if(tmpPassword.count())
        {
            g_showTipsDialog(tr("密码只能包含字母和数字。"),  this);
            return false;
        }
        if(pass.count() < 6)
        {
            g_showTipsDialog(tr("密码长度不能小于6。"),  this);
            return false;
        }
    }

    return true;
}



