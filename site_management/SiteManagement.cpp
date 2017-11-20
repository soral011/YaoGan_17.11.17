#include "SiteManagement.h"
#include "ui_SiteManagement.h"
#include "Global.h"


SiteManagement::SiteManagement(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SiteManagement)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    ui->widget_parent->setAutoFillBackground(true);

    m_siteSettings = new QSettings(SITE_FILE_PATH, QSettings::IniFormat);

    g_connectButtonSignal(this);

    ui->tableWidget->horizontalHeader()->resizeSection(0, 310);

    readSite();
    showSite();

    g_clearPushButtonFocus(this);
}

SiteManagement::~SiteManagement()
{
    delete m_siteSettings;
    delete ui;
}


void SiteManagement::buttonReleased(QWidget *w)
{
    QString buttonName = w->objectName();

    if(buttonName == "pushButton_add")
    {
        AddSite addSite;
        bool isOk = addSite.addSite();
        if(isOk)
        {
            foreach(SiteStruct tmpSite, m_siteList)
            {
                if(tmpSite.m_siteName == addSite.m_site.m_siteName)
                {
                    g_showTipsDialog(tr("此地址已经存在。"), this);
                    return;
                }
            }

            this->m_siteList.append(addSite.m_site);
            this->saveSite();
            this->showSite();

            g_showTipsDialog(tr("添加成功。"), this);
        }
    }
    if(buttonName == "pushButton_delete")
    {
        int row = ui->tableWidget->currentRow();
        if(row >= 0)
        {
            SiteStruct site = m_siteList.at(row);

            bool isSure = g_showAskingDialog(tr("确定要删除")
                                             + QString(" \"%1\" ").arg(site.m_siteName)
                                             + tr("监测地点吗？")
                                             , this);
            if(isSure)
            {
                m_siteList.removeAt(row);
                saveSite();
                showSite();
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

void SiteManagement::paintEvent(QPaintEvent *event)
{
    g_drawShadowBorder(this);
}

void SiteManagement::mousePressEvent(QMouseEvent *event)
{
    m_relativePos = this->pos() - event->globalPos();
}

void SiteManagement::mouseMoveEvent(QMouseEvent *event)
{
    this->move(event->globalPos() + m_relativePos);
}

void SiteManagement::readSite()
{
    QStringList siteNameList = m_siteSettings->childGroups();
    foreach(QString siteName, siteNameList)
    {
        m_siteSettings->beginGroup(siteName);

        SiteStruct site;
        site.m_siteName = siteName;
        site.m_siteGradiant = m_siteSettings->value(HKEY_SITE_GRADIANT, "").toString();
        site.m_siteLatitude = m_siteSettings->value(HKEY_SITE_LATITUDE, "").toString();
        site.m_siteLongitude= m_siteSettings->value(HKEY_SITE_LONGITUDE, "").toString();

        m_siteList.append(site);

        m_siteSettings->endGroup();
    }
}

void SiteManagement::saveSite()
{
    m_siteSettings->clear();

    foreach(SiteStruct site, m_siteList)
    {
        m_siteSettings->beginGroup(site.m_siteName);
        m_siteSettings->setValue(HKEY_SITE_GRADIANT, site.m_siteGradiant);
        m_siteSettings->setValue(HKEY_SITE_LATITUDE, site.m_siteLatitude);
        m_siteSettings->setValue(HKEY_SITE_LONGITUDE, site.m_siteLongitude);
        m_siteSettings->endGroup();
    }

    m_siteSettings->sync();
}


void SiteManagement::showSite()
{
    int siteCount = m_siteList.count();
//    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(siteCount);
    for(int row = 0; row < siteCount; row++)
    {
        QTableWidgetItem *item;
        SiteStruct site = m_siteList.at(row);
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
        item->setText(site.m_siteName);

        item = ui->tableWidget->item(row, 1);
        item->setText(site.m_siteLongitude);

        item = ui->tableWidget->item(row, 2);
        item->setText(site.m_siteLatitude);

        item = ui->tableWidget->item(row, 3);
        item->setText(site.m_siteGradiant);
    }
}




