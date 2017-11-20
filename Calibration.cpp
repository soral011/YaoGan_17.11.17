#include "Calibration.h"
#include "ui_Calibration.h"
#include "MyDebug.cpp"
#include <QThread>
#include "Global.h"

Calibration::Calibration(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Calibrate)
{
    MY_LOG("start");
    ui->setupUi(this);

//    //发送Socket数据
//    connect(&m_viewCaliData, SIGNAL(send(QByteArray,QByteArray)),
//            this, SIGNAL(send(QByteArray,QByteArray)));

//    //接收Socket数据
//    connect(this, SIGNAL(dataPacketReady2(SocketPacket)),
//            &m_viewCaliData, SLOT(dataPacketReady(SocketPacket)));

    m_rubberBand = new QRubberBand(QRubberBand::Rectangle);

    ui->stackedWidget_main->setCurrentIndex(1);

    initCustomPlot(ui->customplot_tdlas);
    initCustomPlot(ui->customplot_doas);
    initCustomPlot(ui->customplot_opacity);

    g_connectButtonSignal(this);

    ui->pushButton_calibration_cancel->setEnabled(false);
    ui->pushButton_calibration_quit->hide();

    MY_LOG("end");
}

Calibration::~Calibration()
{
    delete ui;
}

void Calibration::initCustomPlot(QCustomPlot *customPlot)
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
    }
    else
    {

        //设置坐标轴标签名称
        customPlot->xAxis->setLabel(tr("采样点"));
        customPlot->yAxis->setLabel(tr("幅度"));

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

void Calibration::createBarChart(QCustomPlot *customPlot)
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

void Calibration::dataPacketReady(SocketPacket packet)
{
//    MY_DEBUG("");
//    emit dataPacketReady2(packet);

    if(packet.dataType == NET_DATA_TYPE_TESTER_STATUS)
    {
        if(this->isVisible() == false)
        {
            return;
        }
        NetTesterStatus testerStatus;
        testerStatus.read(packet.data);

//        MY_DEBUG("m_startRunTime="<<testerStatus.m_startRunSecs);
//        MY_DEBUG("m_caliRunTime="<<testerStatus.m_caliRunSecs);
//        MY_DEBUG("m_doasIntensity="<<testerStatus.m_doasIntensity<<"\n");

        //紫外
        m_dataDoasY.clear();
        m_dataDoasX.clear();
        int count = testerStatus.m_doasSpectrum.count();
        for(int i = 0; i < count; i++)
        {
            m_dataDoasY.append(testerStatus.m_doasSpectrum.at(i));
            m_dataDoasX.append(i + 1);
        }
        if(m_dataDoasY.count() > 0)
        {
            updateDoasPlot(m_dataDoasX,m_dataDoasY);
        }

        //红外
        m_dataTDLAS_Y.clear();
        m_dataTDLAS_X.clear();
        count = testerStatus.m_tdlasRawSpectrum.count();
        for(int i = 0; i < count; i++)
        {
            m_dataTDLAS_Y.append(testerStatus.m_tdlasRawSpectrum.at(i));
            m_dataTDLAS_X.append(i + 1);
        }
        if(m_dataTDLAS_Y.count() > 0)
        {
            updateTDLASPlot(m_dataTDLAS_X,m_dataTDLAS_Y);
        }

        //绿光
        updateOpacities(testerStatus.m_opacityADResults);
    }
    else if(packet.dataType == NET_DATA_TYPE_CALI_STATUS)
    {
//        MY_LOG("");
        if(packet.data.count() != sizeof(NetCalibrationStatus))
        {
            MY_DEBUG("packet.data.count() != sizeof(NetCalibrationStatus)");
            return;
        }

        NetCalibrationStatus caliStatus;
        memcpy(&caliStatus, packet.data.data(), sizeof(NetCalibrationStatus));

        QString text(caliStatus.text);
        ui->label_calibration_status->setText(text+"...");
        if(caliStatus.finished == true)
        {
            ui->label_calibration_status->setText(text);
            ui->pushButton_calibration_cancel->setEnabled(false);
            ui->pushButton_calibration_start->setEnabled(true);
            this->m_progressBar.hide();

            emit calibrationFinished();
        }

    }
//    else if(packet.dataType == NET_DATA_TYPE_ALL_CALI_DATA)
//    {
////        if(packet.data.count() != sizeof(NetCalibrationData))
////        {
////            MY_DEBUG("packet.data.count() != sizeof(NetCalibrationStatus)");
////            return;
////        }

//        MY_DEBUG("");

//        NetCalibrationData caliData;
//        caliData.read(packet.data);

//        qDebug()<<"All cali data ------------------------------------------------";

//        qDebug()<<"m_doasSpectrumCaliBg="<<caliData.m_doasSpectrumCaliBg<<"\n\n";
//        qDebug()<<"m_tdlasSpectrumCaliBg="<<caliData.m_tdlasSpectrumCaliBg<<"\n\n";

//        qDebug()<<"m_doasSpectrumCaliNO="<<caliData.m_doasSpectrumCaliNO<<"\n\n";   //NO标定光谱
//        qDebug()<<"m_doasSpectrumCaliHC="<<caliData.m_doasSpectrumCaliHC<<"\n\n";   //HC标定光谱
//        qDebug()<<"m_tdlasSpectrumCali="<<caliData.m_tdlasSpectrumCali<<"\n\n";    //红外标定光谱
//        qDebug()<<"m_opacityADResultsCali="<<caliData.m_opacityADResultsCali<<"\n"; //10路不透光度标定AD值


//        qDebug()<<"m_F_NO_AC_Args="<<caliData.m_F_NO_AC_Args;
//        qDebug()<<"m_NO_CA_Args="<<caliData.m_NO_CA_Args<<"\n";

//        qDebug()<<"m_F_NO_AA_Points="<<caliData.m_F_NO_AA_Points<<"\n";  //NO [出厂] 补偿曲线数据点，A_无HC影响 = f( A_有HC影响 )
//        qDebug()<<"m_F_NO_HC_C="<<caliData.m_F_NO_HC_C; //NO [出厂] 补偿3项式曲线对应的HC浓度值
//        qDebug()<<"m_F_NO_C="<<caliData.m_F_NO_C<<"\n";    //NO [出厂] 标定浓度值（吸光度浓度曲线中最大浓度值）

//        qDebug()<<"m_HC_Cali_A="<<caliData.m_HC_Cali_A; //HC标定吸光度
//        qDebug()<<"m_NO_Cali_A="<<caliData.m_NO_Cali_A<<"\n"; //NO标定吸光度 add by lth 20160624

//        qDebug()<<"m_CO2_Cali_P="<<caliData.m_CO2_Cali_P;
//        qDebug()<<"m_CO_Cali_P="<<caliData.m_CO_Cali_P<<"\n";

//        qDebug()<<"m_HC_Cali_C="<<caliData.m_HC_Cali_C; //HC标定浓度值
//        qDebug()<<"m_NO_Cali_C="<<caliData.m_NO_Cali_C; //NO标定浓度值
//        qDebug()<<"m_CO_Cali_C="<<caliData.m_CO_Cali_C; //CO标定浓度值
//        qDebug()<<"m_CO2_Cali_C="<<caliData.m_CO2_Cali_C<<"\n";//CO2标定浓度值

//        //采样点波长在截取的光谱中的位置
//        qDebug()<<"m_spectrumIndexHC="<<caliData.m_spectrumIndexHC; //229.02nm
//        qDebug()<<"m_spectrumIndexNO="<<caliData.m_spectrumIndexNO; //226.21nm
//    }
}

void Calibration::updateDoasPlot(QVector<double> dataX,QVector<double> dataY)
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

void Calibration::updateTDLASPlot(QVector<double> dataX,QVector<double> dataY)
{
//    MY_DEBUG("x.count()="<<dataX.count()<<"y.count()="<<dataY.count());
    ui->customplot_tdlas->graph(0)->clearData();
    ui->customplot_tdlas->graph(0)->setData(dataX,dataY);

    if(m_tdlasRangeX.upper != dataX.count())
    {
        m_tdlasRangeX = QCPRange(0, dataX.count());
        ui->customplot_tdlas->xAxis->setRange(m_tdlasRangeX);
    }

    ui->customplot_tdlas->replot();
}

void Calibration::updateOpacities(QList<int> opacityADResults)
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

void Calibration::buttonReleased(QWidget *w)
{
    QString buttonName=w->objectName();

    if(buttonName == "pushButton_calibration_next_page")
    {
        if( SocketClient::getInstance()->isConnected() == false )
        {
            g_showTipsDialog(tr("测量主机未连接!"), this);
            return;
        }
        NetCaliGasConcentration netCaliGasC;
        netCaliGasC.HC_C = ui->doubleSpinBox_C_HC->value();
        netCaliGasC.NO_C = ui->doubleSpinBox_C_NO->value();
        netCaliGasC.CO_C = ui->doubleSpinBox_C_CO->value();
        netCaliGasC.CO2_C = ui->doubleSpinBox_C_CO2->value();
        netCaliGasC.clearGasTime = ui->doubleSpinBox_clear_gas_time->value();
        QByteArray ba((char *)&netCaliGasC, sizeof(NetCaliGasConcentration));
        emit send(ba, NET_DATA_TYPE_CALI_GAS_C);

        ui->stackedWidget_calibration_steps->setCurrentIndex(1);

    }
    else if(buttonName == "pushButton_calibration_previous_page")
    {
        ui->stackedWidget_calibration_steps->setCurrentIndex(0);
    }
    else if(buttonName == "pushButton_calibration_start")
    {
        emit send(NET_CMD_CALIBRATE_AUTOMATICALLY, NET_DATA_TYPE_CMD);
        m_progressBar.display(ui->label_progress_bar);
        ui->label_calibration_status->setText(tr("正在标定..."));
        ui->pushButton_calibration_cancel->setEnabled(true);
        ui->pushButton_calibration_start->setEnabled(false);
    }
    else if(buttonName == "pushButton_calibration_cancel")
    {
        bool isSure = g_showAskingDialog(tr("确定要退出吗？"), this);
        if(isSure)
        {
            m_progressBar.hide();
            ui->label_calibration_status->setText(tr(""));
            ui->pushButton_calibration_cancel->setEnabled(false);
            ui->pushButton_calibration_start->setEnabled(true);
            emit send(NET_CMD_STOP_CALIBRATING, NET_DATA_TYPE_CMD);
        }
    }
//    else if(buttonName == "pushButton_get_all_cali_data")
//    {
//        emit send(NET_CMD_GET_ALL_CALI_DATA, NET_DATA_TYPE_CMD);
//        m_viewCaliData.close();
//        m_viewCaliData.showMaximized();
//    }
    else if(buttonName == "pushButton_zoom_reset_tdlas")
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
    else if(buttonName == "pushButton_calibration_quit")
    {
        emit quit();
    }
}

void Calibration::showSpectrumOnly()
{
    ui->pushButton_calibration_quit->hide();
    ui->verticalWidget_calibration_panel->hide();
    ui->verticalSpacer_top->changeSize(0,0,QSizePolicy::Ignored,QSizePolicy::Ignored);
    ui->verticalSpacer_buttom->changeSize(0,0,QSizePolicy::Ignored,QSizePolicy::Ignored);
    ui->horizontalSpacer_right->changeSize(0,0,QSizePolicy::Ignored,QSizePolicy::Ignored);
}

//记录鼠标右键在customPlot按下的坐标
void Calibration::mousePress(QMouseEvent* mevent)
{
    if(mevent->button() == Qt::RightButton)
    {
        m_rubberOrigin = mevent->globalPos();
        m_rubberBand->setGeometry(QRect(m_rubberOrigin, QSize()));
        m_rubberBand->show();
    }
}

//rubberBand随着鼠标移动而改变尺寸大小
void Calibration::mouseMove(QMouseEvent *mevent)
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
void Calibration::mouseRelease(QMouseEvent *mevent)
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

