/***********************************************************
**
** class: CalibrationData
**
** 作者：lth
** 日期：2017-07-25
**
** 查看标定数据界面
**
************************************************************/

#ifndef VIEWCALIDATA_H
#define VIEWCALIDATA_H

#include <QWidget>
#include "tools/qcustomplot.h"
#include "SocketClient.h"

namespace Ui {
class ViewCaliData;
}

class CalibrationData : public QWidget
{
    Q_OBJECT

public:
    explicit CalibrationData(QWidget *parent = 0);
    ~CalibrationData();

private:
    Ui::ViewCaliData *ui;

    QRubberBand *m_rubberBand;
    QPoint       m_rubberOrigin;

    NetCalibrationData m_caliData;
    QVector<double> m_dataY;

private:
    double calcPloyfitY(double x, QList<float> args);
    void initCustomPlot(QCustomPlot *customPlot);
    void updateCaliSpectrumPlot(QVector<double> dataX,QVector<double> dataY);
    void updateCaliCurvePlot(QVector<double> dataX, QVector<double> dataY);

signals:
    void send(QByteArray data,QByteArray dataType);

private slots:
    void on_radioButton_show_doas_bg_spectrum_clicked();

    void on_radioButton_show_hc_cali_spectrum_clicked();

    void on_radioButton_show_no_cali_spectrum_clicked();

    void on_radioButton_show_tdlas_cali_spectrum_clicked();

    void on_pushButton_get_cali_data_clicked();

    void dataPacketReady(SocketPacket socketPackage);

    void mousePress(QMouseEvent* mevent);
    void mouseMove(QMouseEvent *mevent);
    void mouseRelease(QMouseEvent *mevent);

    void on_pushButton_copy_cali_data_clicked();
    void on_radioButton_show_hc_cali_curve_clicked();
    void on_radioButton_show_no_cali_curve_clicked();
};

#endif // VIEWCALIDATA_H
