/***********************************************************
**
** class: Spectrum
**
** 作者：lth
** 日期：2016-03-21
**
** 用于显示红外、紫外光谱、绿光光强
**
************************************************************/
#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <QWidget>
#include "tools/qcustomplot.h"
#include "SocketClient.h"


namespace Ui {
class Spectrum;
}

class Spectrum : public QWidget
{
    Q_OBJECT

public:
    explicit Spectrum(QWidget *parent = 0);
    ~Spectrum();

private:
    Ui::Spectrum *ui;

    QCPBars      *m_opacityBar;


    QVector<double> m_dataDoasY;
    QVector<double> m_dataDoasX;

    QVector<double> m_dataTDLAS_Y;
    QVector<double> m_dataTDLAS_X;

    QList<int>      m_tdlasBgSpectrum;

    QRubberBand *m_rubberBand;
    QPoint       m_rubberOrigin;

    QCPRange     m_tdlasRangeX;
    QCPRange     m_tdlasRangeY;
    QCPRange     m_doasRangeX;
    QCPRange     m_doasRangeY;
    QCPRange     m_opacityRangeX;
    QCPRange     m_opacityRangeY;

    QPoint       m_relativePos;

private:
    void initCustomPlot(QCustomPlot *customPlot);
    void createBarChart(QCustomPlot *customPlot);
    void updateDoasPlot(QVector<double> dataX,QVector<double> dataY);
    void updateTDLASPlot(QVector<double> dataX,QVector<double> dataY);
    void updateOpacities(QList<int> opacityADResults);
    void setToolsWidgetVisible();

private slots:
    void dataPacketReady(SocketPacket socketPackage);
    void buttonReleased(QWidget *w);

    void mousePress(QMouseEvent* mevent);
    void mouseMove(QMouseEvent *mevent);
    void mouseRelease(QMouseEvent *mevent);
    void showEvent(QShowEvent * event);
    void on_tabWidget_currentChanged(int index);
    void resizeEvent(QResizeEvent * event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    void hasClient(bool value);
    void on_checkBox_show_tdlas_peak_line_clicked(bool checked);
};

#endif // SPECTRUM_H
