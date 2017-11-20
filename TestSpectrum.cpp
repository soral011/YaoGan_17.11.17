#include "TestSpectrum.h"
#include "ui_TestSpectrum.h"
#include "MyDebug.cpp"
#include <QVector>
#include "Constant.h"
//#include <QtXlsx/QtXlsx>
#include "xlsxdocument.h"
#include "Global.h"

TestSpectrum::TestSpectrum(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TestSpectrum)
{
    ui->setupUi(this);

    m_rubberBand = new QRubberBand(QRubberBand::Rectangle);

    initCustomPlot(ui->widget_customplot);

    g_clearPushButtonFocus(this);
}

TestSpectrum::~TestSpectrum()
{
    delete ui;
}

void TestSpectrum::initCustomPlot(QCustomPlot *customPlot)
{

    MY_LOG("start");

    //QCustomPlot自身没有选中区域并放大的功能，需要自己补充增加
    connect(customPlot, SIGNAL(mousePress(QMouseEvent*)),
            this, SLOT(mousePress(QMouseEvent*)));
    connect(customPlot, SIGNAL(mouseMove(QMouseEvent*)),
            this, SLOT(mouseMove(QMouseEvent*)));
    connect(customPlot, SIGNAL(mouseRelease(QMouseEvent*)),
            this, SLOT(mouseRelease(QMouseEvent*)));

    //设置属性可缩放，移动等
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);

    QColor color = Qt::red;

    customPlot->legend->setVisible(false);


    //设置坐标轴标签名称
    customPlot->xAxis->setLabel(tr("采样点"));
    customPlot->yAxis->setLabel(tr("幅度"));

    customPlot->addGraph();//向绘图区域QCustomPlot(从widget提升来的)添加一条曲线

    //设置曲线颜色
    QPen pen;
    pen.setWidth(1);
    pen.setColor(color);// line color blue for first graph
    customPlot->graph(0)->setPen(pen);
    customPlot->replot();
    MY_LOG("end");
}


//记录鼠标右键在customPlot按下的坐标
void TestSpectrum::mousePress(QMouseEvent* mevent)
{
    if(mevent->button() == Qt::RightButton)
    {
        m_rubberOrigin = mevent->globalPos();
        m_rubberBand->setGeometry(QRect(m_rubberOrigin, QSize()));
        m_rubberBand->show();
    }
}

//rubberBand随着鼠标移动而改变尺寸大小
void TestSpectrum::mouseMove(QMouseEvent *mevent)
{
    if(m_rubberBand->isVisible())
    {
        m_rubberBand->setGeometry(QRect(m_rubberOrigin, mevent->globalPos())
                                  .normalized());
    }
}

/* 放大图表选中区域
 * 参考 http://blog.csdn.net/onlyshi/article/details/51508178
 */
void TestSpectrum::mouseRelease(QMouseEvent *mevent)
{
//    Q_UNUSED(mevent);
    if (m_rubberBand->isVisible())
    {
        QCustomPlot *customPlot = qobject_cast<QCustomPlot*>(sender());
        if(customPlot == 0)
        {
            return;
        }

        //选中的矩形框必须大于3×3，避免右键点击后直接放开，属于无操作
        QRect zoomRect = m_rubberBand->geometry();
        QSize size = zoomRect.size();
        if(size.width() < 3 || size.height() < 3)
        {
            m_rubberBand->hide();
            return ;
        }

        //鼠标右键从按下到放开，形成一个矩形，这里寻找矩形左上角坐标
        QPoint releasePos = mevent->pos();
        QPoint pressPos   = customPlot->mapFromGlobal(m_rubberOrigin);
        QPoint topLeftPos;
        topLeftPos.setX(qMin(releasePos.x(), pressPos.x()));
        topLeftPos.setY(qMin(releasePos.y(), pressPos.y()));

        //生成新的矩形，是在customPlot坐标上的矩形
        zoomRect = QRect(topLeftPos, size);

        int xp1, yp1, xp2, yp2;
        zoomRect.getCoords(&xp1, &yp1, &xp2, &yp2);
        double x1 = customPlot->xAxis->pixelToCoord(xp1);
        double x2 = customPlot->xAxis->pixelToCoord(xp2);
        double y1 = customPlot->yAxis->pixelToCoord(yp1);
        double y2 = customPlot->yAxis->pixelToCoord(yp2);

        customPlot->xAxis->setRange(x1, x2);
        customPlot->yAxis->setRange(y1, y2);

        m_rubberBand->hide();
        customPlot->replot();
    }
}

void TestSpectrum::readSpectrumFiles()
{
    QDir dir(RESULT_SAVE_PATH);
    QList<QString> spectrumFileList = dir.entryList(QDir::Files,QDir::Name);
    QList<QString> strListTmp;
    foreach(QString name, spectrumFileList)
    {
        if(name.startsWith("Spectrum_") && name.endsWith(".xlsx"))
        {
            strListTmp.append(name);
        }
    }
    spectrumFileList = strListTmp;

    ui->comboBox_date->clear();
    ui->comboBox_date->addItems(spectrumFileList);
    ui->comboBox_date->setCurrentIndex(spectrumFileList.count() - 1);

//    if(spectrumFileList.count() == ui->comboBox_date->count() &&
//       ui->comboBox_car->count() == 0)
//    {
//        return;
//    }
    on_comboBox_date_activated(ui->comboBox_date->currentText());
}

void TestSpectrum::on_comboBox_date_activated(const QString &arg1)
{
    QXlsx::Document xlsx(RESULT_SAVE_PATH + arg1);
    int rowCount = xlsx.read("W1").toInt(); //记录总数存储在“W1”

    rowCount = rowCount - 1;//减去表头

    QList<QString> strListTmp;
    for(int i = 0; i < rowCount; i++)
    {
        //第一行是表头，所以从i+2开始
        QString car = xlsx.read(i+2, 1).toString().right(12)
                + "_" + xlsx.read(i+2, 2).toString();
        strListTmp.append(car);
    }

    ui->comboBox_car->clear();
    ui->comboBox_car->addItems(strListTmp);
    if(ui->comboBox_car->count())
    {
        ui->comboBox_car->setCurrentIndex(ui->comboBox_car->count() - 1);
        on_comboBox_car_activated(ui->comboBox_car->count() - 1);
    }

    QString text = QString("%1%2%3").arg(tr("总计："))
            .arg(strListTmp.count())
            .arg(tr("辆"));
    ui->label_car_count->setText(text);
}

void TestSpectrum::on_comboBox_car_activated(int index)
{
    if(ui->radioButton_show_doas_bg_spectrum->isChecked())
    {
        on_radioButton_show_doas_bg_spectrum_clicked();
    }
    else if(ui->radioButton_show_doas_test_spectrum->isChecked())
    {
        on_radioButton_show_doas_test_spectrum_clicked();
    }
    else if(ui->radioButton_show_tdlas_test_spectrum->isChecked())
    {
        on_radioButton_show_tdlas_test_spectrum_clicked();
    }
    else if(ui->radioButton_show_tdlas_bg_spectrum->isChecked())
    {
        on_radioButton_show_tdlas_bg_spectrum_clicked();
    }
    else if(ui->radioButton_show_tdlas_ercixiebo_spectrum->isChecked())
    {
        on_radioButton_show_tdlas_ercixiebo_spectrum_clicked();
    }
    else
    {
        ui->radioButton_show_doas_bg_spectrum->setChecked(true);
        on_radioButton_show_doas_bg_spectrum_clicked();
    }
}

void TestSpectrum::on_radioButton_show_doas_bg_spectrum_clicked()
{
    showSpectrum(4);
}

void TestSpectrum::on_radioButton_show_doas_test_spectrum_clicked()
{
    showSpectrum(3);
}

void TestSpectrum::on_radioButton_show_tdlas_bg_spectrum_clicked()
{
    showSpectrum(6);
}

void TestSpectrum::on_radioButton_show_tdlas_test_spectrum_clicked()
{
    showSpectrum(5);
}

void TestSpectrum::on_radioButton_show_tdlas_ercixiebo_spectrum_clicked()
{
    QString fileName = ui->comboBox_date->currentText();
    if(fileName.isEmpty())
    {
        MY_DEBUG("");
        return;
    }

    QXlsx::Document xlsx(RESULT_SAVE_PATH + fileName);

    int index = ui->comboBox_car->currentIndex();

    QString spectrumData = xlsx.read(index + 2, 5).toString();
    QString spectrumDataBg = xlsx.read(index + 2, 6).toString();
    if(spectrumData.isEmpty())
    {
        MY_DEBUG("");
        return;
    }
    if(spectrumDataBg.isEmpty())
    {
        MY_DEBUG("");
        return;
    }

    QStringList spectrum = spectrumData.split(" ");
    QStringList spectrumBg = spectrumDataBg.split(" ");
    if(spectrum.last().isEmpty())
    {
        spectrum.removeLast();
    }
    if(spectrumBg.last().isEmpty())
    {
        spectrum.removeLast();
    }

    QVector<double> dataX;
    QVector<double> dataY;
    int count = spectrum.count();
    for(int i = 0; i < count; i++)
    {
        dataY.append(spectrumBg.at(i).toDouble() - spectrum.at(i).toDouble());
        dataX.append(i + 1);
    }
    if(dataY.count() > 0)
    {
        updateCustomPlot(dataX, dataY);
    }
}

void TestSpectrum::showSpectrum(int column)
{
    QString fileName = ui->comboBox_date->currentText();
    if(fileName.isEmpty())
    {
        MY_DEBUG("");
        return;
    }

    QXlsx::Document xlsx(RESULT_SAVE_PATH + fileName);

    int index = ui->comboBox_car->currentIndex();

    QString spectrumData = xlsx.read(index + 2, column).toString();
    if(spectrumData.isEmpty())
    {
        MY_DEBUG("");
        return;
    }

    QStringList spectrum = spectrumData.split(" ");
    if(spectrum.last().isEmpty())
    {
        spectrum.removeLast();
    }

    QVector<double> dataX;
    QVector<double> dataY;
    int count = spectrum.count();
    for(int i = 0; i < count; i++)
    {
        dataY.append(spectrum.at(i).toDouble());
        dataX.append(i + 1);
    }
    if(dataY.count() > 0)
    {
        updateCustomPlot(dataX, dataY);
    }
}

void TestSpectrum::updateCustomPlot(QVector<double> dataX,QVector<double> dataY)
{
    if(dataX.count() > 2048)
    {
        dataX = dataX.mid(0, 2048);
        dataY = dataY.mid(0, 2048);
    }
    m_dataY = dataY;

    ui->tabWidget->setCurrentIndex(1);
    ui->widget_customplot->graph(0)->clearData();
    ui->widget_customplot->graph(0)->setData(dataX, dataY);

    QCPRange rangeX = QCPRange(0, dataX.count());
    ui->widget_customplot->xAxis->setRange(rangeX);

    qSort(dataY);

    QCPRange rangeY = QCPRange(dataY.first() - 150, dataY.last() + 150);
    ui->widget_customplot->yAxis->setRange(rangeY);

    ui->widget_customplot->replot();


}

void TestSpectrum::on_pushButton_copy_spectrum_clicked()
{
    QString str;
    for(int i = 0; i<m_dataY.count(); i++)
    {
        char buffer[100] = {0};
        sprintf(buffer, "%1.2f\n", m_dataY.at(i));
        str.append(QString(buffer));
    }
    QClipboard *board = QApplication::clipboard();
    board->setText(str);
}

void TestSpectrum::showEvent(QShowEvent * event)
{
    readSpectrumFiles();
}


