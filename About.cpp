#include "About.h"
#include "ui_About.h"
#include <QDebug>
#include <QPainter>
#include <math.h>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include "Global.h"
#include "Version.h"

About::About(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    ui->widget_parent->setAutoFillBackground(true);

    ui->label_version_number->setText(SOFTWARE_VERSION_NUMBER);

    g_clearPushButtonFocus(this);
}

About::~About()
{
    delete ui;
    qDebug()<<"About::~About()";
}

void About::paintEvent(QPaintEvent *event)
{
    g_drawShadowBorder(this);
}

void About::on_pushButton_close_clicked()
{
    g_closeWithAnim(this);
}

void About::mousePressEvent(QMouseEvent *event)
{
    m_relativePos = this->pos()-event->globalPos();
}

void About::mouseMoveEvent(QMouseEvent *event)
{
    this->move(event->globalPos()+ m_relativePos);
}

