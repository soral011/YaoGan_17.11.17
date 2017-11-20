#include "Spectrum.h"
#include "ui_Spectrum.h"
#include "MyDebug.cpp"

Spectrum::Spectrum(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Spectrum)
{
    MY_LOG("start");
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);

    m_rubberBand = new QRubberBand(QRubberBand::Rectangle);

    initCustomPlot(ui->customplot_tdlas);
    initCustomPlot(ui->customplot_doas);
    initCustomPlot(ui->customplot_opacity);

    g_connectButtonSignal(this);

    SocketClient *client = SocketClient::getInstance();
    connect(client, SIGNAL(hasClient(bool)),
            this, SLOT(hasClient(bool)));
//    connect(this,SIGNAL(spectrumReady(SocketPacket)),
//            &m_calibration,SLOT(dataPacketReady(SocketPacket)));

    ui->tabWidget->setCurrentWidget(ui->tabWidgetPage1); // 显示紫外光谱

    setToolsWidgetVisible();
    g_clearPushButtonFocus(this);

    MY_LOG("end");
}

Spectrum::~Spectrum()
{
    delete ui;
}

void Spectrum::initCustomPlot(QCustomPlot *customPlot)
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

    QColor  color;
    if(customPlot == ui->customplot_doas)
    {
        color = QColor(128,0,128);
        m_doasRangeX = QCPRange(0, 580);
        m_doasRangeY = QCPRange(0, 52000);
        ui->customplot_doas->xAxis->setRange(m_doasRangeX);
        ui->customplot_doas->yAxis->setRange(m_doasRangeY);
    }
    else if(customPlot == ui->customplot_tdlas)
    {
        color = Qt::red;
        m_tdlasRangeX = QCPRange(0, 590);
        m_tdlasRangeY = QCPRange(0, 70000);
        ui->customplot_tdlas->xAxis->setRange(m_tdlasRangeX);
        ui->customplot_tdlas->yAxis->setRange(m_tdlasRangeY);
    }
    else if(customPlot == ui->customplot_opacity)
    {
        color = Qt::darkGreen;
        m_opacityRangeX = QCPRange(0, 11);
        m_opacityRangeY = QCPRange(0, 4500);
        ui->customplot_opacity->xAxis->setRange(m_opacityRangeX);
        ui->customplot_opacity->yAxis->setRange(m_opacityRangeY);
    }

    customPlot->legend->setVisible(false);


    if(customPlot == ui->customplot_opacity)
    {
        //设置坐标轴标签名称
        customPlot->xAxis->setLabel(tr("光通道"));
        customPlot->yAxis->setLabel(tr("光强"));
        createBarChart(customPlot);

        customPlot->addGraph();//向绘图区域QCustomPlot(从widget提升来的)添加一条曲线

        //设置曲线颜色
        QPen pen;
        pen.setWidth(1);
        pen.setColor(Qt::blue);// line color blue for first graph
        customPlot->graph(0)->setPen(pen);

        QVector<double> xData;
        QVector<double> yData;
        xData<<0 <<10.5;
        yData<<3600 <<3600;
        customPlot->graph(0)->setData(xData, yData);
    }
    else
    {

        //设置坐标轴标签名称
        customPlot->xAxis->setLabel(tr("采样点"));
        customPlot->yAxis->setLabel(tr("幅度"));//(10^3)

        customPlot->addGraph();//向绘图区域QCustomPlot(从widget提升来的)添加一条曲线

        //设置曲线颜色
        QPen pen;
        pen.setWidth(1);
        pen.setColor(color);// line color blue for first graph
        customPlot->graph(0)->setPen(pen);
    }
    customPlot->replot();
    MY_LOG("end");
}

void Spectrum::dataPacketReady(SocketPacket packet)
{
    if(packet.dataType == NET_DATA_TYPE_TESTER_STATUS)
    {
//        static long times = 1;
//        qDebug()<<"times="<<times;
//        times++;
        if(this->isVisible() == false)
        {
            return;
        }

//        if(m_calibration.isVisible())
//        {
//            emit spectrumReady(packet);
//            return;
//        }

        NetTesterStatus sysStatusPacket;
        sysStatusPacket.read(packet.data);

//        //临时增加 2017-01-18 SENTENCE_TO_PROCESS
//        if(sysStatusPacket.m_opacityADResults.count())
//            MY_DEBUG("m_opacityADResults.first()="<<sysStatusPacket.m_opacityADResults.first());

        //紫外
        m_dataDoasY.clear();
        m_dataDoasX.clear();
        int count = sysStatusPacket.m_doasSpectrum.count();

        for(int i = 0; i < count; i++)
        {
            m_dataDoasY.append(sysStatusPacket.m_doasSpectrum.at(i));
            m_dataDoasX.append(i + 1);
        }
        if(m_dataDoasY.count() > 0)
        {
            updateDoasPlot(m_dataDoasX,m_dataDoasY);
        }

        //红外
        m_dataTDLAS_Y.clear();
        m_dataTDLAS_X.clear();
        count = sysStatusPacket.m_tdlasSpectrum.count();
        for(int i = 0; i < count; i++)
        {
            double d ;
            if(ui->checkBox_show_raw_tdlas_spectrum->isChecked())
            {
                if(sysStatusPacket.m_tdlasRawSpectrum.count() == 0)
                {
                    break;
                }
                d = sysStatusPacket.m_tdlasRawSpectrum.at(i);
            }
            else if(ui->checkBox_show_tdlas_peak_spectrum->isChecked())
            {
                break;
            }
            else
            {
                d = sysStatusPacket.m_tdlasSpectrum.at(i);
            }

            if(ui->checkBox_use_tdlas_bg_spectrum->isChecked() &&
//                ui->checkBox_show_raw_tdlas_spectrum->isChecked() == false &&
                m_tdlasBgSpectrum.count() > i )
            {
                d = m_tdlasBgSpectrum.at(i) - d;
            }
            m_dataTDLAS_Y.append( d );
            m_dataTDLAS_X.append(i + 1);
        }
        if(m_dataTDLAS_Y.count() > 0)
        {
            updateTDLASPlot(m_dataTDLAS_X,m_dataTDLAS_Y);
        }

        //绿光
        updateOpacities(sysStatusPacket.m_opacityADResults);
    }
    else if(packet.dataType == NET_DATA_TYPE_RT_TEST_RESULT)
    {
        if(ui->checkBox_show_tdlas_peak_spectrum->isChecked() == false)
        {
            return;
        }
        m_dataTDLAS_Y.clear();
        m_dataTDLAS_X.clear();
        QByteArray data = packet.data;
        NetExhaustResults exhaustResults;
        memcpy(&exhaustResults, data.data(), sizeof(NetExhaustResults));
        for(int i = 0; i < 2048; i++)
        {
            float f = exhaustResults.tdlasTestResults.tdlasPeakSpectrum[i];
            m_dataTDLAS_Y.append( f );
            m_dataTDLAS_X.append(i + 1);
        }

//        qDebug()<<"m_dataTDLAS_Y="<<m_dataTDLAS_Y.mid(0,100);

        updateTDLASPlot(m_dataTDLAS_X,m_dataTDLAS_Y);
    }

    //保存光谱到文件170107
    if(packet.dataType == NET_DATA_TYPE_TESTER_STATUS)
    {

        static QFile file;

        if(ui->checkBox_save_tdlas_spectrum_file->isChecked())
        {
            if(file.isOpen() == false)
            {
                QString fileName = "C:\\Users\\zdmqkfyc\\Desktop\\";
                fileName += "TDLAS-Spectrum_"
                        + g_getNowTime("yyyy-MM-dd-hh_mm_ss") + ".txt";
                file.setFileName(fileName);
                bool isOk = file.open(QFile::Append);
                MY_DEBUG("isOk="<<isOk);
            }
            if(file.isOpen())
            {
                static QList<QCPData> dataY;
                QList<QCPData> tmpDataY =
                        ui->customplot_tdlas->graph(0)->data()->values();

                if(dataY.isEmpty() || (dataY.first().value != tmpDataY.first().value))
                {
                    dataY =  tmpDataY;

                    if(tmpDataY.count())
                    {

                        for(int i = 0; i < tmpDataY.count(); i++)
                        {
                            file.write(QByteArray().setNum(tmpDataY.at(i).value) + " ");
                        }
                        file.write("\r\n");
                    }
                }
                file.flush();
            }
        }
        else
        {
            if(file.isOpen())
            {
                file.close();
            }
        }
    }
}

void Spectrum::createBarChart(QCustomPlot *customPlot)
{
    MY_LOG("start");
    /*QCPBars **/
    m_opacityBar   = new QCPBars(customPlot->xAxis, customPlot->yAxis);
    customPlot->addPlottable(m_opacityBar);

    m_opacityBar->setWidth(0.3);

    // set names and colors:
    QPen pen;
    pen.setWidthF(0.8);

//    opacity->setName("Regenerative");
    pen.setColor(Qt::darkGreen/*QColor(150, 222, 0)*/);
    m_opacityBar->setPen(pen);
    m_opacityBar->setBrush(QColor(150, 222, 0, 70));

    // prepare x axis with country labels:
    QVector<double> ticks;
    QVector<QString> labels;
    ticks << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9 << 10;
    labels << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "10";

    customPlot->xAxis->setAutoTicks(false);
    customPlot->xAxis->setAutoTickLabels(false);
    customPlot->xAxis->setTickVector(ticks);
    customPlot->xAxis->setTickVectorLabels(labels);
//    customPlot->xAxis->setTickLabelRotation(60);
    customPlot->xAxis->setSubTickCount(0);
    customPlot->xAxis->setTickLength(0, 4);
//    customPlot->xAxis->grid()->setVisible(true);
//    customPlot->xAxis->setRange(0, 11);

    // prepare y axis:
//    customPlot->yAxis->setRange(0, 12.1);
    customPlot->yAxis->setPadding(5); // a bit more space to the left border

    QPen gridPen;
    gridPen.setStyle(Qt::SolidLine);
    gridPen.setColor(QColor(0, 0, 0, 25));
    customPlot->yAxis->grid()->setPen(gridPen);
    gridPen.setStyle(Qt::DotLine);
    customPlot->yAxis->grid()->setSubGridPen(gridPen);

    // Add data:
    QVector<double> opacityData;
//    opacityData  << 0.86*10.5 << 0.83*5.5 << 0.84*5.5
//                 << 0.52*5.8 << 0.89*5.2 << 0.90*4.2
//                 << 0.52*5.8 << 0.89*5.2 << 0.90*4.2 << 0.67*11.2;
    for(int i = 0; i < 10; i++)
    {
        opacityData.append(0);
    }
    m_opacityBar->setData(ticks, opacityData);

    MY_LOG("end");
}


void Spectrum::updateDoasPlot(QVector<double> dataX,QVector<double> dataY)
{
//    MY_DEBUG("x.count()="<<dataX.count()<<"y.count()="<<dataY.count());
    ui->customplot_doas->graph(0)->clearData();
    ui->customplot_doas->graph(0)->setData(dataX,dataY);
    if(m_doasRangeX.upper != dataX.count())
    {
        m_doasRangeX = QCPRange(0, dataX.count());
        ui->customplot_doas->xAxis->setRange(m_doasRangeX);
    }
    ui->customplot_doas->replot();
}

void Spectrum::updateTDLASPlot(QVector<double> dataX, QVector<double> dataY)
{
//    MY_DEBUG("x.count()="<<dataX.count()<<"y.count()="<<dataY.count());
    ui->customplot_tdlas->graph(0)->clearData();
    ui->customplot_tdlas->graph(0)->setData(dataX, dataY);

    if(m_tdlasRangeX.upper != dataX.count())
    {
        m_tdlasRangeX = QCPRange(0, dataX.count());
        ui->customplot_tdlas->xAxis->setRange(m_tdlasRangeX);
    }

    ui->customplot_tdlas->replot();
}

void Spectrum::updateOpacities(QList<int> opacityADResults)
{
    QVector<double> opacityData;
    for(int i = 0; i < opacityADResults.count(); i++)
    {
        opacityData.append(opacityADResults.at(i));
    }
    QVector<double> ticks;
    ticks << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9 << 10;
    m_opacityBar->setData(ticks, opacityData);
//    ui->customplot_opacity->yAxis->setRange(0, 4500);
    ui->customplot_opacity->replot();
}

void Spectrum::buttonReleased(QWidget *w)
{
    QString buttonName = w->objectName();

//    m_calibration.hide();

    if(buttonName == "pushButton_zoom_reset_tdlas")
    {
        MY_DEBUG("");
        ui->customplot_tdlas->xAxis->setRange(m_tdlasRangeX);
        ui->customplot_tdlas->yAxis->setRange(m_tdlasRangeY);
        ui->customplot_tdlas->replot();
    }
    else if(buttonName == "pushButton_zoom_reset_doas")
    {
        ui->customplot_doas->xAxis->setRange(m_doasRangeX);
        ui->customplot_doas->yAxis->setRange(m_doasRangeY);
        ui->customplot_doas->replot();
    }
    else if(buttonName == "pushButton_zoom_reset_opacity")
    {
        ui->customplot_opacity->xAxis->setRange(m_opacityRangeX);
        ui->customplot_opacity->yAxis->setRange(m_opacityRangeY);
        ui->customplot_opacity->replot();
    }
    else if(buttonName == "pushButton_minimize")
    {
        this->showMinimized();
    }
    else if(buttonName == "pushButton_close")
    {
        g_closeWithAnim(this);
    }
    else if(buttonName == "pushButton_save_bg_spectrum")
    {

        m_tdlasBgSpectrum.clear();
        for(int i = 0; i < m_dataTDLAS_Y.count(); i++)
        {
            m_tdlasBgSpectrum.append(m_dataTDLAS_Y.at(i));
        }
        MY_DEBUG("m_tdlasBgSpectrum.count()"<<m_tdlasBgSpectrum.count());
//        m_tdlasBgSpectrum = m_dataTDLAS_Y;
    }
    else if(buttonName == "pushButton_copy_tdlas_spectrum")
    {
        QList<QCPData> dataY = ui->customplot_tdlas->graph(0)->data()->values();
        QString str;
        for(int i = 0; i < dataY.count(); i++)
        {
            char buffer[100] = {0};
            sprintf(buffer, "%1.2f\n", dataY.at(i).value);
            str.append(QString(buffer));
        }
        QClipboard *board = QApplication::clipboard();
        board->setText(str);
    }
    else if(buttonName == "pushButton_copy_doas_spectrum")
    {
        QList<QCPData> dataY = ui->customplot_doas->graph(0)->data()->values();
        QString str;
        for(int i = 0; i < dataY.count(); i++)
        {
            char buffer[100] = {0};
            sprintf(buffer, "%1.2f\n", dataY.at(i).value);
            str.append(QString(buffer));
        }
        QClipboard *board = QApplication::clipboard();
        board->setText(str);
    }
}

void Spectrum::setToolsWidgetVisible()
{
    ui->widget_tdlas_tools->setVisible(g_isDebugMode);
    ui->pushButton_copy_doas_spectrum->setVisible(g_isDebugMode);
}

//记录鼠标右键在customPlot按下的坐标
void Spectrum::mousePress(QMouseEvent* mevent)
{
    if(mevent->button() == Qt::RightButton)
    {
        m_rubberOrigin = mevent->globalPos();
        m_rubberBand->setGeometry(QRect(m_rubberOrigin, QSize()));
        m_rubberBand->show();
    }
}

void Spectrum::mousePressEvent(QMouseEvent *event)
{
    m_relativePos = this->pos()-event->globalPos();
}

void Spectrum::mouseMoveEvent(QMouseEvent *event)
{
//    this->move(event->globalPos()+ m_relativePos);
}

//rubberBand随着鼠标移动而改变尺寸大小
void Spectrum::mouseMove(QMouseEvent *mevent)
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
void Spectrum::mouseRelease(QMouseEvent *mevent)
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


void Spectrum::on_tabWidget_currentChanged(int index)
{
//    m_calibration.hide();
//    if(index == 3)
//    {
//        m_calibration.setParent(this);
////        m_calibration.setAutoFillBackground(true);
//        m_calibration.showSpectrumOnly();

//        QWidget *w = ui->tabWidget->widget(index);
//        if(w != NULL)
//        {
//            m_calibration.resize(w->size());
//            m_calibration.move(w->mapToGlobal(QPoint(0,0)));
//        }
//        m_calibration.show();
//    }
}

void Spectrum::showEvent(QShowEvent * event)
{
    setToolsWidgetVisible();
    g_updateTesterInfo(ui->label_tester_info);
}

void Spectrum::resizeEvent(QResizeEvent * event)
{
    if(ui->tabWidget->currentIndex() == 3)
    {
        on_tabWidget_currentChanged(3);
    }
}

void Spectrum::hasClient(bool value)
{
    g_updateTesterInfo(ui->label_tester_info);
    MY_DEBUG("");
}

void Spectrum::on_checkBox_show_tdlas_peak_line_clicked(bool checked)
{
    QCustomPlot *customPlot = ui->customplot_tdlas;
    while(customPlot->graphCount() < 2)
    {
        customPlot->addGraph();
    }

    //设置曲线颜色
    QPen pen;
    pen.setWidth(1);
    pen.setColor(Qt::blue);// line color blue for first graph
    customPlot->graph(1)->setPen(pen);

    if(checked)
    {
        QVector<double> xData;
        QVector<double> yData;
        xData<<430 <<430;
        yData<<-100000 <<100000;
        customPlot->graph(1)->setData(xData, yData);
    }
    else
    {
        customPlot->graph(1)->clearData();
    }
    customPlot->replot();
}
