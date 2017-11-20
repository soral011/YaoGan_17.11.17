#include "UpdateSoftwareARM.h"
#include "ui_UpdateSoftwareARM.h"
#include "SocketClient.h"
#include "MyDebug.cpp"
#include <QFileDialog>
#include <QDirModel>
#include <QTreeView>
#include "tools/ChooseFileDialog.h"

UpdateSoftwareARM::UpdateSoftwareARM(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateSoftwareARM)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    ui->widget_parent->setAutoFillBackground(true);

    ui->progressBar->setValue(0);
    ui->lineEdit->clear();
    ui->stackedWidget->setCurrentWidget(ui->page_choose_file);

    g_connectButtonSignal(this);

    SocketClient *client = SocketClient::getInstance();
    connect(client, SIGNAL(sendingFilePercent(int)),
            this, SLOT(updatePercent(int)));
    g_clearPushButtonFocus(this);
}

UpdateSoftwareARM::~UpdateSoftwareARM()
{
    delete ui;
}

void UpdateSoftwareARM::updatePercent(int percent)
{
    ui->progressBar->setValue(percent);
    if(percent == 100)
    {
        g_showTipsDialog(tr("更新成功！\n测量主机将重启！"));
        this->close();
    }
    else if(percent < 0)
    {
        g_showTipsDialog(tr("更新失败，网络异常！"));
        buttonReleased(ui->pushButton_stop);
    }
}

void UpdateSoftwareARM::paintEvent(QPaintEvent *event)
{
    g_drawShadowBorder(this);
}

void UpdateSoftwareARM::mousePressEvent(QMouseEvent *event)
{
    m_relativePos = this->pos() - event->globalPos();
}

void UpdateSoftwareARM::mouseMoveEvent(QMouseEvent *event)
{
    this->move(event->globalPos() + m_relativePos);
}

void UpdateSoftwareARM::buttonReleased(QWidget *w)
{
    QString buttonName = w->objectName();

    SocketClient *client = SocketClient::getInstance();
    if(buttonName == "pushButton_choose_file")
    {
        if(client->isConnected() == false)
        {
            g_showTipsDialog(tr("测量主机未连接！"), this);
            return ;
        }

//        ChooseFileDialog chooseFileDialog;
        QString fileName /*= chooseFileDialog.getOpenFileName()*/;
        fileName =  QFileDialog::getOpenFileName(this
                                                ,"选择测量主机更新程序"
                                                ,""
                                                ,"");

        if(!fileName.isEmpty())
        {
            ui->lineEdit->setText(fileName);
        }

    }
    else if(buttonName == "pushButton_update")
    {
        if(client->isConnected() == false)
        {
            g_showTipsDialog(tr("测量主机未连接！"), this);
            return ;
        }

        MY_DEBUG("");
        QString fileName = ui->lineEdit->text();
        QFileInfo fileInfo(fileName);

        //临时屏蔽  测试网卡传输速率 170602 SENTENCE_TO_PROCESS
        if(fileInfo.isFile() && fileName.contains(TEST_HOST_SOFTWARE_NAME))
        {
            ui->stackedWidget->setCurrentWidget(ui->page_update_status);
            SocketClient *client = SocketClient::getInstance();
            MY_DEBUG("");
            if(client->isSendingFile() == false)
            {
                client->sendFile(fileName);
            }
        }
        else
        {
            g_showTipsDialog(tr("请选择正确的文件！"));
        }
    }
    else if(buttonName == "pushButton_stop")
    {
        SocketClient *client = SocketClient::getInstance();
        client->stopSendFile();
        ui->stackedWidget->setCurrentWidget(ui->page_choose_file);
    }
    else if(buttonName == "pushButton_close")
    {
        g_closeWithAnim(this);
    }
}


void UpdateSoftwareARM::keyPressEvent(QKeyEvent *event)
{
    MY_DEBUG("event->key()="<<event->key());
    switch (event->key())
    {
    case Qt::Key_Escape:
    case Qt::Key_Enter:
    case Qt::Key_Return:
        event->ignore();
        break;
    default:
        QDialog::keyPressEvent(event);
    }
}



