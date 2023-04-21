#ifndef PLOT_WIDGET_H
#define PLOT_WIDGET_H

#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QTimer>

#include "gcode.h"

QT_CHARTS_USE_NAMESPACE

class PlotWidget : public QWidget
{
    Q_OBJECT

    QChart* chart;
    QChartView* chartView;

    ContourRange m_range;
    bool m_range_valid;

    QSpacerItem *hSpacer, *vSpacer;

    QHBoxLayout* hLayout;
    QHBoxLayout* widgetLayout;

    void emptyPlot();
    static void addPlot(QChart* const chart, const std::vector<fpoint_t>& points, const QColor& color = Qt::GlobalColor::blue);
    static void addPlot(QChart* const chart, const Dxf* const dxf);
    static void addPlot(QChart* const chart, const DxfEntity* const dxf, const QColor& color = Qt::GlobalColor::yellow);

    void _replaceChart(QChart*& newChart);
    void replaceChart(QChart*& newChart, const ContourRange& range);

public:
    explicit PlotWidget(QWidget *parent = nullptr);
    ~PlotWidget();

    void plot(const std::vector<GPoint>& pts);
    void plot(const ContourList& contours);

protected:
    virtual QSize minimumSizeHint() const;
    virtual QSize sizeHint() const;
};

#endif // PLOT_WIDGET_H
