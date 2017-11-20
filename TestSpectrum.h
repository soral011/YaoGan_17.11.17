/***********************************************************
**
** class: TestSpectrum
**
** 作者：lth
** 日期：2016-10-11
**
** 用于查看测量结果对应的背景、测量光谱
**
************************************************************/
#ifndef TESTSPECTRUM_H
#define TESTSPECTRUM_H

#include <QWidget>
#include "tools/qcustomplot.h"

namespace Ui {
class TestSpectrum;
}

class TestSpectrum : public QWidget
{
    Q_OBJECT

public:
    explicit TestSpectrum(QWidget *parent = 0);
    ~TestSpectrum();
private:
    Ui::TestSpectrum *ui;

    QRubberBand *m_rubberBand;
    QPoint       m_rubberOrigin;

    QVector<double> m_dataY;

private:
    void initCustomPlot(QCustomPlot *customPlot);
    void updateCustomPlot(QVector<double> dataX,QVector<double> dataY);
    void readSpectrumFiles();
    void showSpectrum(int column);

private slots:
    void mousePress(QMouseEvent* mevent);
    void mouseMove(QMouseEvent *mevent);
    void mouseRelease(QMouseEvent *mevent);
    void on_comboBox_date_activated(const QString &arg1);
    void on_comboBox_car_activated(int index);
    void on_radioButton_show_doas_bg_spectrum_clicked();
    void on_radioButton_show_doas_test_spectrum_clicked();
    void on_radioButton_show_tdlas_bg_spectrum_clicked();
    void on_radioButton_show_tdlas_test_spectrum_clicked();
    void on_radioButton_show_tdlas_ercixiebo_spectrum_clicked();
    void on_pushButton_copy_spectrum_clicked();
    void showEvent(QShowEvent * event);
};

#endif // TESTSPECTRUM_H
