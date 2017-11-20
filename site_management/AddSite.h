/***********************************************************
**
** class: AddSite
**
** 作者：lth
** 日期：2017-07-25
**
** 添加监测点位
**
************************************************************/

#ifndef ADDSITE_H
#define ADDSITE_H

#include <QDialog>
#include <qsettings.h>

#define HKEY_SITE_NAME          "site_name"
#define HKEY_SITE_LONGITUDE     "site_longitude"
#define HKEY_SITE_LATITUDE      "site_latitude"
#define HKEY_SITE_GRADIANT      "site_gradiant"

class SiteStruct
{
public:
    //这四个参数都是《遥感检测法二次征求稿》里要求的
    QString m_siteName;      //监测地点
    QString m_siteLongitude; //经度，经纬度可通过手机“指南针APP”查询（eg.华为P9）
    QString m_siteLatitude;  //纬度
    QString m_siteGradiant;  //坡度
};

namespace Ui {
class AddSite;
}

class AddSite : public QDialog
{
    Q_OBJECT

public:
    explicit AddSite(QWidget *parent = 0);
    ~AddSite();
    static QString getSiteMD5(SiteStruct site);

    SiteStruct m_site;

private:
    Ui::AddSite *ui;
    QPoint     m_relativePos;

    bool       m_isAddOk;

public:
    bool addSite();

private:
    bool isSiteValid(SiteStruct site);

private slots:
    void buttonReleased(QWidget *w);
    void paintEvent(QPaintEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    void on_plainTextEdit_site_textChanged();
};

#endif // ADDSITE_H
