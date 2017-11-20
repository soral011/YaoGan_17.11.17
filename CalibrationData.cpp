#include "CalibrationData.h"
#include "ui_CalibrationData.h"
#include "MyDebug.cpp"
#include <QVector>

CalibrationData::CalibrationData(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ViewCaliData)
{
    ui->setupUi(this);

    m_rubberBand = new QRubberBand(QRubberBand::Rectangle);

    initCustomPlot(ui->widget_cali_data_plot);
    initCustomPlot(ui->widget_A_C_curve_plot);
    ui->tabWidget->setCurrentIndex(0);

    g_clearPushButtonFocus(this);
}

CalibrationData::~CalibrationData()
{
    delete ui;
}

void CalibrationData::initCustomPlot(QCustomPlot *customPlot)
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

    customPlot->addGraph();//向绘图区域QCustomPlot(从widget提升来的)添加一条曲线


    //设置坐标轴标签名称
    if(customPlot == ui->widget_cali_data_plot)
    {
        customPlot->xAxis->setLabel(tr("采样点"));
        customPlot->yAxis->setLabel(tr("幅度"));
    }
    else if(customPlot == ui->widget_A_C_curve_plot)
    {
        customPlot->xAxis->setLabel(tr("浓度 C"));
        customPlot->yAxis->setLabel(tr("吸光度 A"));
        customPlot->graph(0)->setScatterStyle(QCPScatterStyle::ssDisc);
        color = Qt::blue;
    }



    //设置曲线颜色
    QPen pen;
    pen.setWidth(1);
    pen.setColor(color);// line color blue for first graph
    customPlot->graph(0)->setPen(pen);

    customPlot->replot();
    MY_LOG("end");
}

void CalibrationData::dataPacketReady(SocketPacket packet)
{
//    MY_DEBUG("");

    if(packet.dataType == NET_DATA_TYPE_ALL_CALI_DATA)
    {
        MY_DEBUG("");

        m_caliData.read(packet.data);

        ui->plainTextEdit->clear();
        ui->plainTextEdit->appendPlainText(m_caliData.m_text);
    }
}


void CalibrationData::on_radioButton_show_doas_bg_spectrum_clicked()
{
    QVector<double> dataX;
    QVector<double> dataY;
    int count = m_caliData.m_doasSpectrumCaliBg.count();
    for(int i = 0; i < count; i++)
    {
        dataY.append(m_caliData.m_doasSpectrumCaliBg.at(i));
        dataX.append(i + 1);
    }
    if(dataY.count() > 0)
    {
        updateCaliSpectrumPlot(dataX, dataY);
    }
}

void CalibrationData::on_radioButton_show_hc_cali_spectrum_clicked()
{
    QVector<double> dataX;
    QVector<double> dataY;
    int count = m_caliData.m_doasSpectrumCaliHC.count();
    for(int i = 0; i < count; i++)
    {
        dataY.append(m_caliData.m_doasSpectrumCaliHC.at(i));
        dataX.append(i + 1);
    }
    if(dataY.count() > 0)
    {
        updateCaliSpectrumPlot(dataX, dataY);
    }
}

void CalibrationData::on_radioButton_show_no_cali_spectrum_clicked()
{
    QVector<double> dataX;
    QVector<double> dataY;
    int count = m_caliData.m_doasSpectrumCaliNO.count();
    for(int i = 0; i < count; i++)
    {
        dataY.append(m_caliData.m_doasSpectrumCaliNO.at(i));
        dataX.append(i + 1);
    }
    if(dataY.count() > 0)
    {
        updateCaliSpectrumPlot(dataX, dataY);
    }
}

void CalibrationData::on_radioButton_show_tdlas_cali_spectrum_clicked()
{
    QVector<double> dataX;
    QVector<double> dataY;
    int count = m_caliData.m_tdlasSpectrumCali.count();
    for(int i = 0; i < count; i++)
    {
        dataY.append(m_caliData.m_tdlasSpectrumCaliBg.at(i)
                     - m_caliData.m_tdlasSpectrumCali.at(i));
        dataX.append(i + 1);
    }
    if(dataY.count() > 0)
    {
        updateCaliSpectrumPlot(dataX, dataY);
    }
}

void CalibrationData::on_pushButton_get_cali_data_clicked()
{
    MY_DEBUG("");
    emit send(NET_CMD_GET_ALL_CALI_DATA, NET_DATA_TYPE_CMD);
}

void CalibrationData::updateCaliSpectrumPlot(QVector<double> dataX,QVector<double> dataY)
{
    m_dataY = dataY;

    ui->tabWidget->setCurrentIndex(1);
    ui->widget_cali_data_plot->graph(0)->clearData();
    ui->widget_cali_data_plot->graph(0)->setData(dataX, dataY);

    QCPRange rangeX = QCPRange(0, dataX.count());
    ui->widget_cali_data_plot->xAxis->setRange(rangeX);

    qSort(dataY);

    QCPRange rangeY = QCPRange(dataY.first() - 50, dataY.last() + 50);
    ui->widget_cali_data_plot->yAxis->setRange(rangeY);

    ui->widget_cali_data_plot->replot();


}

//记录鼠标右键在customPlot按下的坐标
void CalibrationData::mousePress(QMouseEvent* mevent)
{
    if(mevent->button() == Qt::RightButton)
    {
        m_rubberOrigin = mevent->globalPos();
        m_rubberBand->setGeometry(QRect(m_rubberOrigin, QSize()));
        m_rubberBand->show();
    }
}

//rubberBand随着鼠标移动而改变尺寸大小
void CalibrationData::mouseMove(QMouseEvent *mevent)
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
void CalibrationData::mouseRelease(QMouseEvent *mevent)
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

void CalibrationData::on_pushButton_copy_cali_data_clicked()
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

void CalibrationData::on_radioButton_show_hc_cali_curve_clicked()
{
    MY_DEBUG("m_caliData.m_F_HC_AC_Args="<<m_caliData.m_F_HC_AC_Args);
    if(m_caliData.m_F_HC_AC_Args.count() < 4)
    {
        return;
    }

    QList<float> args = m_caliData.m_F_HC_AC_Args;
    MY_DEBUG("args="<<args);

    QVector<double> dataX;
    QVector<double> dataY;
    dataX << 0 << 40 << 100 << 160;
    for(int i = 0; i < dataX.count(); i++)
    {
        dataY.append(calcPloyfitY(dataX.at(i), args));
    }

    this->updateCaliCurvePlot(dataX, dataY);
}

void CalibrationData::on_radioButton_show_no_cali_curve_clicked()
{
    if(m_caliData.m_F_NO_AC_Args.count() < 4)
    {
        return;
    }

    QList<float> args = m_caliData.m_F_NO_AC_Args;

    QVector<double> dataX;
    QVector<double> dataY;
    dataX << 0 << 1000 << 1500 << 2500 << 4000;
    for(int i = 0; i < dataX.count(); i++)
    {
        dataY.append(calcPloyfitY(dataX.at(i), args));
    }

    this->updateCaliCurvePlot(dataX, dataY);
}

//绘制标定曲线
void CalibrationData::updateCaliCurvePlot(QVector<double> dataX, QVector<double> dataY)
{
    MY_DEBUG("dataX="<<dataX);
    MY_DEBUG("dataY="<<dataY);
    ui->tabWidget->setCurrentIndex(2);
    ui->widget_A_C_curve_plot->graph(0)->clearData();
    ui->widget_A_C_curve_plot->graph(0)->setData(dataX, dataY);

    QCPRange rangeX = QCPRange(dataX.first() * 0.9, dataX.last() * 1.1);
    ui->widget_A_C_curve_plot->xAxis->setRange(rangeX);

    qSort(dataY);

    QCPRange rangeY = QCPRange(dataY.first() * 0.9, dataY.last() * 1.1);
    ui->widget_A_C_curve_plot->yAxis->setRange(rangeY);

    ui->widget_A_C_curve_plot->replot();


}


//计算Y = args.at(0) + args.at(1)*pow(x,1) + args.at(2)*pow(x,2)+...
double CalibrationData::calcPloyfitY(double x, QList<float> args)
{
//    MY_LOG("start");
    int count = args.count();
    double Y = 0;
    //计算A = args.at(0) + args.at(1)*pow(C,1) + args.at(2)*pow(C,2)+...
    for(int i = 0; i < count; i++)
    {
        if(i == 0 )
        {
            Y += args.at(0);
        }
        else
        {
            Y += args.at(i)*pow(x, i);
        }
    }

    return Y;
}




