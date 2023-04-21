#ifndef PLOT_WIDGET_H
#define PLOT_WIDGET_H

#include <QtWidgets/QApplication>
#include <QtCharts>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>

#include "gcode.h"

class PlotWidget : public QWidget
{
    Q_OBJECT

    QChartView* chartView;
    QChart* chart;

    QSpacerItem *hSpacer, *vSpacer;

    QHBoxLayout* hLayout;
    QHBoxLayout* widgetLayout;

public:
    explicit PlotWidget(QWidget *parent = nullptr);
    ~PlotWidget();

    void plot(const std::vector<GPoint>& pts);

protected:
    virtual QSize minimumSizeHint() const;
    virtual QSize sizeHint() const;
};

#endif // PLOT_WIDGET_H
