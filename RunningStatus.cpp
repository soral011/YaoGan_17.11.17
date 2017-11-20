#include "RunningStatus.h"
#include "ui_RunningStatus.h"
#include "Global.h"
#include <QDateTime>

RunningStatus::RunningStatus(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShowTipsWidget)
{
    m_tipsLabel=NULL;

    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint
                                   |Qt::Tool/*|Qt::SubWindow*/);
//    Qt::FramelessWindowHint加上以下这句使得窗口背景透明
    this->setAttribute(Qt::WA_TranslucentBackground);

    connect(&g_mySignal,SIGNAL(showTips(QString)),
            this,SLOT(showTips(QString)));
    ui->plainTextEdit_tips->clear();
}

RunningStatus::~RunningStatus()
{
    delete ui;
}

void RunningStatus::paintEvent(QPaintEvent *event)
{
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);

    int grayWidth=6;

    path.addRect(grayWidth, grayWidth, this->width()-grayWidth*2,
                 this->height()-grayWidth*2);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillPath(path, QBrush(Qt::white));

    QColor color(0, 0, 0, 15);
    for(int i=0; i<grayWidth; i++)
    {
        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        path.addRect(grayWidth-i, grayWidth-i, this->width()-(grayWidth-i)*2,
                     this->height()-(grayWidth-i)*2);
        int max_alpha=25;
        color.setAlpha((sqrt(grayWidth)*max_alpha+1) - sqrt(i)*max_alpha);
        painter.setPen(color);
        painter.drawPath(path);
    }
}

void RunningStatus::showTips(QString tips)
{
    tips.insert(0,QDateTime::currentDateTime().toString("hh:mm:ss.zzz "));
//    tips.append("\n");
    ui->plainTextEdit_tips->moveCursor(QTextCursor::End);
    ui->plainTextEdit_tips->insertPlainText(tips+"\n");

    m_tipList.append(tips);
    while(m_tipList.count()>1000)
    {
        m_tipList.removeFirst();
    }
    if(m_tipsLabel!=NULL)
    {
        g_showLabelTextWithAnim(m_tipsLabel,tips);
//        m_tipsLabel->setText(tips);
    }
}


