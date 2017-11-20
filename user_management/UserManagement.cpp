#include "UserManagement.h"
#include "ui_UserManagement.h"
#include "Global.h"

UserManagement::UserManagement(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UserManagement)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    ui->widget_parent->setAutoFillBackground(true);

    m_userSettings = new QSettings(USER_FILE_PATH, QSettings::IniFormat);

    connect(&m_addUserDialog, SIGNAL(addUserReady(UserStruct)),
            this, SLOT(addUserReady(UserStruct)));

    g_connectButtonSignal(this);

    g_clearPushButtonFocus(this);

    readUser();
    showUser();
}

UserManagement::~UserManagement()
{
    delete m_userSettings;
    delete ui;
}


void UserManagement::buttonReleased(QWidget *w)
{
    QString buttonName = w->objectName();

    if(buttonName == "pushButton_add")
    {
        m_addUserDialog.exec();
    }
    if(buttonName == "pushButton_delete")
    {
        int row = ui->tableWidget->currentRow();
        if(row >= 0)
        {
            if(m_userList.count() == 1)
            {
                g_showTipsDialog(tr("只有一个用户，删除后无法登陆。"), this);
                return;
            }

            UserStruct user = m_userList.at(row);

            if(user.m_userName == g_currentUser.m_userName)
            {
                g_showTipsDialog(tr("此用户处于登陆状态，请退出并切换用户登陆。"), this);
                return;
            }

            bool isSure = g_showAskingDialog(tr("确定要删除")
                                             + QString(" %1 ").arg(user.m_userName)
                                             + tr("用户吗？")
                                             , this);
            if(isSure)
            {
                m_userList.removeAt(row);
                saveUser();
                showUser();
            }
        }
        MY_DEBUG("row="<<row);
    }
    else if(buttonName == "pushButton_change_password")
    {
        int row = ui->tableWidget->currentRow();
        if(row >= 0)
        {
            UserStruct user = m_userList.at(row);
            ChangePassword changePassword;
            changePassword.changePassword(user);
            if(changePassword.changedOk)
            {
                m_userList.replace(row, changePassword.m_user);
                this->saveUser();
                g_showTipsDialog(tr("修改成功。"), this);
            }
        }

    }
    else if(buttonName == "pushButton_close" ||
            buttonName == "pushButton_cancel" ||
            buttonName == "pushButton_finish")
    {
        g_closeWithAnim(this);
    }
}

void UserManagement::paintEvent(QPaintEvent *event)
{
    g_drawShadowBorder(this);
}

void UserManagement::mousePressEvent(QMouseEvent *event)
{
    m_relativePos = this->pos() - event->globalPos();
}

void UserManagement::mouseMoveEvent(QMouseEvent *event)
{
    this->move(event->globalPos()+ m_relativePos);
}

void UserManagement::addUserReady(UserStruct user)
{
    foreach(UserStruct tmpUser, m_userList)
    {
        if(tmpUser.m_userName == user.m_userName)
        {
            g_showTipsDialog(tr("此用户已经存在。"), this);
            return;
        }
    }

    m_userList.append(user);
    this->saveUser();
    this->showUser();
    g_showTipsDialog(tr("添加成功。"), this);
}

void UserManagement::readUser()
{
    QStringList userNameList = m_userSettings->childGroups();
    foreach(QString userName, userNameList)
    {
        m_userSettings->beginGroup(userName);

        UserStruct user;
        user.m_userName = userName;
        user.m_userPassword    = m_userSettings->value(HKEY_USER_PASSWORD, "").toString();
        user.m_userChineseName = m_userSettings->value(HKEY_USER_CHINESE_NAME, "").toString();
        user.m_userDiscription = m_userSettings->value(HKEY_USER_DESCRIPTION, "").toString();

        m_userList.append(user);

        m_userSettings->endGroup();
    }

}

void UserManagement::saveUser()
{
    if(m_userList.count() <= 0)
    {
        return;
    }

    m_userSettings->clear();

    foreach(UserStruct user, m_userList)
    {
        m_userSettings->beginGroup(user.m_userName);
        m_userSettings->setValue(HKEY_USER_PASSWORD, user.m_userPassword);
        m_userSettings->setValue(HKEY_USER_CHINESE_NAME, user.m_userChineseName);
        m_userSettings->setValue(HKEY_USER_DESCRIPTION, user.m_userDiscription);
        m_userSettings->endGroup();
    }

    m_userSettings->sync();

}

void UserManagement::showUser()
{
    int userCount = m_userList.count();
//    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(userCount);
    for(int row = 0; row < userCount; row++)
    {
        QTableWidgetItem *item;
        UserStruct user = m_userList.at(row);
        int columnCount = ui->tableWidget->columnCount();
        for(int column = 0; column < columnCount; column++)
        {
            item = ui->tableWidget->item(row, column);
            if(item == 0)
            {
                item = new QTableWidgetItem;
                ui->tableWidget->setItem(row, column, item);
            }
        }

        item = ui->tableWidget->item(row, 0);
        item->setText(QString().setNum(row + 1));

        item = ui->tableWidget->item(row, 1);
        item->setText(user.m_userName);

        item = ui->tableWidget->item(row, 2);
        item->setText(user.m_userChineseName);

        item = ui->tableWidget->item(row, 3);
        item->setText(user.m_userDiscription);
    }
}

bool UserManagement::isUserValid(UserStruct *user)
{
    foreach(UserStruct tmpUser, m_userList)
    {
        if(tmpUser.m_userName == user->m_userName &&
           tmpUser.m_userPassword == user->m_userPassword)
        {
            *user = tmpUser;
            return true;
        }
    }
    return false;
}



