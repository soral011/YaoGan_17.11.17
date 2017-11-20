#include "ChooseFileDialog.h"
#include "ui_ChooseFileDialog.h"
#include "MyDebug.cpp"

ChooseFileDialog::ChooseFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseFileDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    ui->widget_parent->setAutoFillBackground(true);

//    m_modelIndex = NULL;
//    ui->treeView->header()->setClickable(true);
}

ChooseFileDialog::~ChooseFileDialog()
{
    delete ui;
}


void ChooseFileDialog::paintEvent(QPaintEvent *event)
{
    g_drawShadowBorder(this);
}

void ChooseFileDialog::on_pushButton_close_clicked()
{
    g_closeWithAnim(this);
}

void ChooseFileDialog::mousePressEvent(QMouseEvent *event)
{
    m_relativePos = this->pos()-event->globalPos();
}

void ChooseFileDialog::mouseMoveEvent(QMouseEvent *event)
{
    this->move(event->globalPos()+ m_relativePos);
}

void ChooseFileDialog::on_pushButton_open_clicked()
{
    MY_DEBUG("");
    QString fileName = m_fileSystemModel.filePath(m_modelIndex);
    qDebug()<<"fileName="<<fileName;
    QFileInfo fileInfo(fileName);
    if(fileInfo.isFile())
    {
        m_fileName = fileInfo.absoluteFilePath();
        g_closeWithAnim(this);
    }
    else if(fileInfo.isDir())
    {
        MY_DEBUG("");
        bool isp = ui->treeView->isExpanded(m_modelIndex);
        ui->treeView->setExpanded(m_modelIndex, !isp);
    }
}

void ChooseFileDialog::on_pushButton_cancel_clicked()
{
    g_closeWithAnim(this);
}

QString ChooseFileDialog::getOpenFileName(QStringList filters)
{
    m_fileSystemModel.setRootPath("/");
    m_fileSystemModel.setReadOnly(true);
//    m_dirModel.setSorting(QDir::DirsFirst | QDir::IgnoreCase | QDir::Name);
//    QStringList nameFilter;
//    nameFilter << "*.cpp" << "*.h";
    if(filters.isEmpty() == false)
    {
        m_fileSystemModel.setNameFilters(filters);
    }

    // 没有通过过滤器的文件隐藏
    m_fileSystemModel.setNameFilterDisables(false);

    ui->treeView->setModel(&m_fileSystemModel);

    ui->treeView->setIconSize(QSize(38,38));

    ui->treeView->resizeColumnToContents(0);
    ui->treeView->header()->resizeSection(0, 350);
    ui->treeView->header()->setSortIndicator(0, Qt::DescendingOrder);
    ui->treeView->header()->setSortIndicatorShown(true);

    this->exec();
    return m_fileName;
}

void ChooseFileDialog::on_treeView_clicked(const QModelIndex &index)
{
    MY_DEBUG("");
    m_modelIndex = index;
}

void ChooseFileDialog::on_treeView_doubleClicked(const QModelIndex &index)
{
    MY_DEBUG("");
    QFileInfo fileInfo = m_fileSystemModel.filePath(index);
    if(fileInfo.isFile())
    {
        m_fileName = fileInfo.absoluteFilePath();
        g_closeWithAnim(this);
    }
}
