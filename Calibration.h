/***********************************************************
**
** class: Calibrate
**
** 作者：lth
** 日期：2016-03-21
**
** 1.负责标定流程；
** 2.非标定流程时，作为显示光谱数据界面。
**
************************************************************/

#ifndef CALIBRATE_H
#define CALIBRATE_H

#include <QWidget>
#include "SocketConstant.h"
//#include <qwt_plot_grid.h>
//#include <qwt_plot_zoomer.h>
//#include <qwt_plot_curve.h>
//#include <qwt_plot_magnifier.h>
#include "NetDataStructure.h"
#include <QTimer>
#include <QRubberBand>
#include "tools/qcustomplot.h"
#include "tools/ProgressBar.h"
#include "tools/MyMessageDialog.h"
#include "CalibrationData.h"

namespace Ui {
class Calibrate;
}

class Calibration : public QWidget
{
    Q_OBJECT



private:
    Ui::Calibrate *ui;

    volatile bool m_updateFinished;

    QCPBars      *m_opacityBar;

    QVector<double> m_dataDoasY;
    QVector<double> m_dataDoasX;

    QVector<double> m_dataTDLAS_Y;
    QVector<double> m_dataTDLAS_X;

    ProgressBar     m_progressBar;
    MyMessageDialog m_messageDialog;

    QRubberBand *m_rubberBand;
    QPoint       m_rubberOrigin;

    QCPRange     m_tdlasRangeX;
    QCPRange     m_tdlasRangeY;
    QCPRange     m_doasRangeX;
    QCPRange     m_doasRangeY;
    QCPRange     m_opacityRangeX;
    QCPRange     m_opacityRangeY;

//    CalibrationData m_viewCaliData;

public:
    explicit Calibration(QWidget *parent = 0);
    ~Calibration();

    void showSpectrumOnly();


private:
    void updateDoasPlot(QVector<double> dataX,QVector<double> dataY);
    void updateTDLASPlot(QVector<double> dataX,QVector<double> dataY);
    void updateOpacities(QList<int> opacityADResults);
    void createBarChart(QCustomPlot *customPlot);

signals:
    void send(QByteArray data,QByteArray dataType);
//    void dataPacketReady2(SocketPacket socketPackage);
    void calibrationFinished();
    void quit();

private slots:
    void dataPacketReady(SocketPacket socketPackage);

    void initCustomPlot(QCustomPlot *customPlot);
    void buttonReleased(QWidget *w);

    void mousePress(QMouseEvent *mevent);
    void mouseMove(QMouseEvent *mevent);
    void mouseRelease(QMouseEvent *mevent);
};

#endif // CALIBRATE_H
