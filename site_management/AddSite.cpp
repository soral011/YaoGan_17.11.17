#include "AddSite.h"
#include "ui_AddSite.h"
#include "Global.h"

AddSite::AddSite(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddSite)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    ui->widget_parent->setAutoFillBackground(true);

    g_connectButtonSignal(this);

    m_isAddOk = false;

    g_clearPushButtonFocus(this);
}

AddSite::~AddSite()
{
    delete ui;

}


void AddSite::buttonReleased(QWidget *w)
{
    QString buttonName = w->objectName();

    if(buttonName == "pushButton_ok")
    {
        SiteStruct site;
        site.m_siteName      = ui->plainTextEdit_site->toPlainText();
        site.m_siteLongitude = ui->spinBox_longitude->text();
        site.m_siteLatitude  = ui->spinBox_latitude->text();
        site.m_siteGradiant  = ui->spinBox_gradiant->text();

        bool isOk = isSiteValid(site);
        if(isOk)
        {
            m_site = site;
            m_isAddOk = true;
            this->close();
        }
    }
    else if(buttonName == "pushButton_close" || buttonName == "pushButton_cancel")
    {
        g_closeWithAnim(this);
    }
}

void AddSite::paintEvent(QPaintEvent *event)
{
    g_drawShadowBorder(this);
}

void AddSite::mousePressEvent(QMouseEvent *event)
{
    m_relativePos = this->pos() - event->globalPos();
}

void AddSite::mouseMoveEvent(QMouseEvent *event)
{
    this->move(event->globalPos() + m_relativePos);
}

bool AddSite::isSiteValid(SiteStruct site)
{
    QString siteName = site.m_siteName;
    if(siteName.remove(" ").isEmpty())
    {
        g_showTipsDialog(tr("监测地点不能为空。"));
        return false;
    }

    return true;
}

void AddSite::on_plainTextEdit_site_textChanged()
{
    int maxLength = 60;
    QString text = ui->plainTextEdit_site->toPlainText();
    if(text.count() > maxLength)
    {
        text = text.left(maxLength);
        ui->plainTextEdit_site->clear();
        ui->plainTextEdit_site->insertPlainText(text);
    }
}

bool AddSite::addSite()
{
    this->exec();
    return m_isAddOk;
}

QString AddSite::getSiteMD5(SiteStruct site)
{
    QString siteID = g_codeMd5(site.m_siteName +
                               site.m_siteGradiant +
                               site.m_siteLatitude +
                               site.m_siteLongitude);
    return siteID;
}


