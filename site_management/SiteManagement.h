/***********************************************************
**
** class: SiteManagement
**
** 作者：lth
** 日期：2017-07-25
**
** 监测点位管理
**
************************************************************/

#ifndef SITEMANAGEMENT_H
#define SITEMANAGEMENT_H

#include <QDialog>
#include <QMouseEvent>
#include <QSettings>
#include "AddSite.h"

namespace Ui {
class SiteManagement;
}

class SiteManagement : public QDialog
{
    Q_OBJECT

public:
    explicit SiteManagement(QWidget *parent = 0);
    ~SiteManagement();
    QList<SiteStruct> m_siteList;

private:
    Ui::SiteManagement *ui;
    QPoint m_relativePos;
    QSettings *m_siteSettings;

private:
    void saveSite();
    void readSite();
    void showSite();

private slots:
    void buttonReleased(QWidget *w);
    void paintEvent(QPaintEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
};

#endif // SITEMANAGEMENT_H
