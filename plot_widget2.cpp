#include "plot_widget2.h"
#include <QDebug>
#include <QList>
#include "gcode.h"
#include <algorithm>

using namespace std;
using namespace QtCharts;

PlotWidget::PlotWidget(QWidget* parent) : QWidget(parent), chartView(nullptr), chart(nullptr) {
    chartView = new QChartView();

    QSizePolicy sp(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sp.setHeightForWidth(true);
    chartView->setSizePolicy(sp);

    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumSize(200, 200);
    chartView->setFixedSize(200, 200);

    widgetLayout = new QHBoxLayout();
    widgetLayout->addWidget(chartView);
    widgetLayout->setAlignment(Qt::AlignRight | Qt::AlignBottom);

    this->setLayout(widgetLayout);
}

PlotWidget::~PlotWidget() {
    delete widgetLayout;
    widgetLayout = nullptr;
    delete chartView;
    chartView = nullptr;
}

void PlotWidget::plot(const std::vector<GPoint>& pts) {
    int margin = 20;

    QChart* newChart = new QChart();
    newChart->legend()->hide();

    if (!pts.empty()) {
        QColor color = pts.begin()->color;

        QLineSeries* series = new QLineSeries();
        series->setColor(color);

        QPointF prev_pt;
        for (const GPoint& pt: pts) {
            if (pt.color == color)
                series->append(pt.point);
            else {
                newChart->addSeries(series);

                color = pt.color;

                series = new QLineSeries();
                series->setColor(color);

                series->append(prev_pt);
                series->append(pt.point);
            }

            prev_pt = pt.point;
        }

        newChart->addSeries(series);
        newChart->createDefaultAxes();
    }

    QList<QAbstractSeries*> list = newChart->series();

    qDebug()<<"QChart series count: "<<list.count();
    int i = 0;
    for (auto s: list) {
        QLineSeries* ls = dynamic_cast<QLineSeries*>(s);
        qDebug() << i++ << ": " << ls->count();
    }

    pair<pair<double, double>, pair<double, double>> range = GCode::MinMaxPlotAxis(pts);

    newChart->axes(Qt::Horizontal).first()->setRange(range.first.first, range.first.second);
    newChart->axes(Qt::Vertical).first()->setRange(range.second.first, range.second.second);

//    chartView->setFixedSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    chartView->setChart(newChart);
    if (chart != nullptr) {
        chart->removeAllSeries();
        delete chart;
    }
    chart = newChart;

    QSize _size = size();
//    if (m_plotSize.width() != 0 && m_plotSize.height() != 0) {
//        _size = m_plotSize;
//    }

    int plotSize;
    if (_size.height() == 0)
        plotSize = _size.width();
    else
        plotSize = min(_size.width(), _size.height()) - margin;

//    int plotSize = min(chartView->size().width(), chartView->size().height());

    if (plotSize < 200) plotSize = 200;
    chartView->setFixedSize(plotSize, plotSize);
    this->updateGeometry();
    this->update();
    chartView->show();
    qDebug() << "chartView size: " << chartView->size();
}

QSize PlotWidget::minimumSizeHint() const {
//    qDebug() << "minimumSizeHint";
    chartView->setFixedSize(200, 200);
    return QSize(200, 200);
}

QSize PlotWidget::sizeHint() const {
    static QSize plotSizeOld(-1, -1);

    QSize widgetSize = size();

    int side;
    if (widgetSize.height() == 0)
        side = widgetSize.width() - 40;
    else
        side = min(widgetSize.width(), widgetSize.height()) - 40;

    if (side < 200) side = 200;

    QSize plotSize(side, side);
    chartView->setFixedSize(plotSize);

    if (plotSize != plotSizeOld) {
        qDebug() << "sizeHint: " << widgetSize;
        qDebug() << "return: " << plotSize;
    }

    plotSizeOld = plotSize;
    return plotSize;
}
