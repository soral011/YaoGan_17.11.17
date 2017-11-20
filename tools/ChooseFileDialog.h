/***********************************************************
**
** class: ChooseFileDialog
**
** 作者：lth
** 日期：2017-07-25
**
** 选择（打开、保存）文件对话框
**
************************************************************/

#ifndef CHOOSEFILEDIALOG_H
#define CHOOSEFILEDIALOG_H

#include <QDialog>
#include <QMouseEvent>
#include "Global.h"
#include <QDirModel>
#include <QFileSystemModel>

namespace Ui {
class ChooseFileDialog;
}

class ChooseFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChooseFileDialog(QWidget *parent = 0);
    ~ChooseFileDialog();

    QString getOpenFileName(QStringList filters = QStringList());

    QString m_fileName;

private:
    Ui::ChooseFileDialog *ui;
    QPoint m_relativePos;
    QFileSystemModel m_fileSystemModel;

    QModelIndex m_modelIndex;

private slots:
    void paintEvent(QPaintEvent *event);
    void on_pushButton_close_clicked();
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    void on_pushButton_open_clicked();
    void on_pushButton_cancel_clicked();
    void on_treeView_clicked(const QModelIndex &index);
    void on_treeView_doubleClicked(const QModelIndex &index);
};

#endif // CHOOSEFILEDIALOG_H
