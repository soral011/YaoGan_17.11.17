#include "ProgressDialog.h"
#include "ui_ProgressDialog.h"
#include <QPixmap>
#include <QMatrix>
#include <QImage>
#include <QDebug>
#include <QTime>
#include <QPainter>
#include <QMargins>
//#include "CommonFuntions.h"

ProgressDialog::ProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint/*| Qt::WindowStaysOnTopHint*/);//设置不显示窗口菜单栏
    connect(&m_updateTimer, SIGNAL(timeout()),this,  SLOT(updateState()));
    m_touchToDisppear=m_isShowingCancelBtn=false;
    m_msg="";
    m_progressBar.m_barPadding=12;
    ui->label_circle_pic->setStyleSheet("");
//    QPalette pal = palette();
//    pal.setColor(QPalette::Background, QColor(0x00,0x00,0x00,0x50));
//    setPalette(pal);
//    setAutoFillBackground(true);

    setAttribute(Qt::WA_TranslucentBackground, true);

//    g_clearPushButtonFocus(this);
}

ProgressDialog::~ProgressDialog()
{
    qDebug()<<"ProgressDialog delete ui";
    delete ui;
}

/*************************************************
** Function: updateState
** Description: 是否该隐藏圆圈progress_bar了
*************************************************/
void ProgressDialog::updateState()
{
    if(this->isHidden())
    {
//        qDebug()<<"this->isHidden()="<<this->isHidden();
        m_updateTimer.stop();
        m_progressBar.hide();
        return;
    }
//    if(ui->label_circle_pic->styleSheet()!="")
//    {
//        ui->label_circle_pic->setStyleSheet("");
//    }
}

void ProgressDialog::disappear()
{
    this->close();
    this->deleteLater();
}

void ProgressDialog::mousePressEvent(QMouseEvent *event)
{
    if(m_touchToDisppear)
        this->close();
    emit mousePress();
}

/*************************************************
** Function: showMessage
** Description: 设置提示信息，并显示进度对话框
** Input: msg -- 要显示的信息
**        position -- 显示的位置，上，中，下
*************************************************/
void ProgressDialog::showMessage(QString msg,QString position)
{
    LOG_HERE("");
    ui->label_show_msg->setText(msg);
    updatePosition(position);
    this->m_msg=msg;
    m_progressBar.display(ui->label_circle_pic);
    //msg在showEvent函数里被使用，但该变量不是volatile，不能及时更新
    int msecs=1;
    QTimer::singleShot(msecs,this,SLOT(show()));

    //防止还没show就执行了hide
    QEventLoop ep;
    QTimer::singleShot(msecs*2,&ep,SLOT(quit()));
    ep.exec();
}

void ProgressDialog::showMessage(QString msg,QWidget *parent,QString position)
{
    this->setParent(parent);
    this->resize(parent->size());
//    qDebug()<<"parent->size()="<<parent->size();
//    qDebug()<<"parent->rect()="<<parent->rect();
//    qDebug()<<"parent->contentsRect()="<<parent->contentsRect();
//    qDebug()<<"parent->height()="<<parent->height();
    this->move(0,0);
    showMessage(msg,position);
}

/*************************************************
** Function: updateText
** Description: 更新显示的信息
*************************************************/
void ProgressDialog::updateText(QString text)
{
//    qDebug()<<"text="<<text;
    QString current_text=ui->label_show_msg->text();
    current_text.remove(QRegExp("[0-9]*/[0-9]*$"));
    ui->label_show_msg->setText(current_text+text);
    this->update();
}

void ProgressDialog::showEvent ( QShowEvent * event )
{
//    qDebug()<<"showEvent !!!";
    if(m_msg.isEmpty()==false)
    {
        ui->label_show_msg->setText(m_msg);
        updatePosition(m_position);
    }
    else if(m_msg!="" && m_msg!=ui->label_show_msg->text())
    {
        ui->label_show_msg->setText(m_msg);
        updatePosition(m_position);
    }
    ui->pushButton_cancel->setVisible(m_isShowingCancelBtn);

    //根据是否隐藏取消按钮，进行调节控件尺寸
    if(m_isShowingCancelBtn==true)
        ui->horizontalWidget_message_box->setContentsMargins(9,9,10,9);
    else
        ui->horizontalWidget_message_box->setContentsMargins(9,9,30,9);

    this->raise();
    m_updateTimer.start(150);

    m_progressBar.display(ui->label_circle_pic);
}

/*************************************************
** Function: updatePosition
** Description: 设置进度对话框显示的位置，上，中，下
*************************************************/
void ProgressDialog::updatePosition(QString position)
{
    QMargins margin;
    if(position=="center")
    {
        ui->horizontalLayout->setContentsMargins(margin);
    }
    else if(position=="bottom")
    {
        margin.setTop(240);
        ui->horizontalLayout->setContentsMargins(margin);
    }
    else if(position=="top")
    {
        margin.setBottom(240);
        ui->horizontalLayout->setContentsMargins(margin);
    }
}

void ProgressDialog::on_pushButton_cancel_clicked()
{
    emit cancelClicked();
}
