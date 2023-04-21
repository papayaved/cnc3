#include "plot_widget.h"
#include "form_edit.h"
#include "ui_formedit.h"

#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QTextCodec>
#include <QSplitter>
#include <QList>
#include <algorithm>

#include "gcode.h"

using namespace std;

PlotWidget::PlotWidget(QWidget* parent) :
    QWidget(parent),
    chart(nullptr),
    chartView(nullptr),
    m_range_valid(false)
{
    chartView = new QChartView;
    chartView->setRenderHint(QPainter::Antialiasing, true);
    emptyPlot();

    QSizePolicy sp(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sp.setHeightForWidth(true);
    chartView->setSizePolicy(sp);

    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumSize(200, 200);
    chartView->setFixedSize(200, 200);

    widgetLayout = new QHBoxLayout();
    widgetLayout->addWidget(chartView);
    widgetLayout->setAlignment(Qt::AlignRight | Qt::AlignTop);

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

void PlotWidget::emptyPlot() {
    m_range_valid = false;

    if (chartView != nullptr) {
        QChart* newChart = new QChart();
        newChart->legend()->hide();

        _replaceChart(newChart);

        QSizePolicy sp(QSizePolicy::Fixed, QSizePolicy::Fixed);
    //    QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sp.setHeightForWidth(true);
        chartView->setSizePolicy(sp);

        chartView->setRenderHint(QPainter::Antialiasing);
        chartView->setMinimumSize(200, 200);
        chartView->setFixedSize(200, 200);
    //    chartView->scale(1, 1);
    }
}

void PlotWidget::plot(const ContourList& contours) {
    QChart* newChart = new QChart();
    newChart->legend()->hide();

    ContourRange range;

    for (const ContourPair& pair: contours.contours()) {
        vector<fpoint_t> pts( pair.getPointsBot() );
        range.append(pts);
        addPlot(newChart, pts);

        pts = pair.getPointsTop();
        range.append(pts);
        addPlot(newChart, pts);
    }

    addPlot(newChart, contours.botSelected());

    if (newChart->series().empty())
        emptyPlot();
    else {
        newChart->createDefaultAxes();
        replaceChart(newChart, range);
    }
}

void PlotWidget::addPlot(QChart* const chart, const std::vector<fpoint_t>& pts, const QColor& color) {
    if (chart) {
        QLineSeries* series = new QLineSeries();
        series->setColor(color);

        for (const fpoint_t& pt: pts)
            series->append(pt.x, pt.y);

        chart->addSeries(series);
    }
}

void PlotWidget::addPlot(QChart* const chart, const Dxf* const dxf) {
    if (chart && dxf && !dxf->empty()) {
        const list<DxfEntity*>& ctr = dxf->entities();

        QLineSeries* series = new QLineSeries();
        series->setColor(dxf->color());

        fpoint_t pt_reg = ctr.front()->point_0();
        series->append(pt_reg.x, pt_reg.y);

        for (const DxfEntity* o: ctr) {
            const vector<fpoint_t> pts = o->getPoints();

            for (const fpoint_t& pt: pts) {
                if (pt != pt_reg) {
                    series->append(pt.x, pt.y);
                    pt_reg = pt;
                }
            }
        }

        chart->addSeries(series);
    }
}

void PlotWidget::addPlot(QChart* const chart, const DxfEntity* const entity, const QColor& color) {
    if (chart && entity) {
        vector<fpoint_t> pts = entity->getPoints();

        QLineSeries* highlight = new QLineSeries();
        highlight->setPen(QPen(QBrush(color), 3));

        for (const fpoint_t& pt: pts)
            highlight->append(pt.x, pt.y);

        chart->addSeries(highlight);
    }
}

void PlotWidget::_replaceChart(QChart*& newChart) {
    if (newChart != nullptr) {
        chartView->setChart(newChart);

        if (chart != nullptr) {
            chart->removeAllSeries();
            delete chart;
        }

        chart = newChart;
        newChart = nullptr;
    }
}

void PlotWidget::replaceChart(QChart*& newChart, const ContourRange& range) {
    //    int margin = 20;

    if (newChart && !newChart->series().empty()) {
        QList<QAbstractSeries*> list = newChart->series();

        if (!list.empty()) {
            m_range = range;
            m_range_valid = true;

            qDebug()<<"QChart series count: "<<list.count();
            int i = 0;
            for (auto s: list) {
                QLineSeries* ls = dynamic_cast<QLineSeries*>(s);
                qDebug() << i++ << ": " << ls->count();
            }

            newChart->axes(Qt::Horizontal).first()->setRange(m_range.x_min, m_range.x_max);
            newChart->axes(Qt::Vertical).first()->setRange(m_range.y_min, m_range.y_max);

        //    chartView->setFixedSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
            _replaceChart(newChart);

            QSize widgetSize = size();
            qDebug() << "Widget size: " << widgetSize;

        //    QSize plot = QSize(formSize.width() - txtSize.width() - 40, formSize.height() - btnSize.height() - 40);
            QSize plotSize = QSize(int(m_range.width() * 1000), int(m_range.height() * 1000));
            qDebug() << "Axes ratio: " << plotSize;

            plotSize.scale(widgetSize.width() - 40, widgetSize.height() - 40, Qt::KeepAspectRatio);
            qDebug() << "Screen axes ratio: " << plotSize;

            chartView->setFixedSize(plotSize);

            this->updateGeometry();
            this->update();

            chartView->show();
            qDebug() << "chartView size: " << chartView->size();
        }
        else {
            m_range_valid = false;
            _replaceChart(newChart);
        }
    }
    else {
        m_range_valid = false;
        _replaceChart(newChart);
    }
}

QSize PlotWidget::minimumSizeHint() const {
//    qDebug() << "minimumSizeHint";
    chartView->setFixedSize(200, 200);
    return QSize(200, 200);
}

QSize PlotWidget::sizeHint() const {
    if (m_range_valid) {
        QSize widgetSize = size();
        int height = widgetSize.height();
        int width = widgetSize.width();

        QSize plotSize = QSize(int(m_range.width() * 1000), int(m_range.height() * 1000));
        plotSize.scale(width - 40, height - 40, Qt::KeepAspectRatio);

//        qDebug() << "sizeHint: " << plotSize;
        chartView->setFixedSize(plotSize);
        return plotSize;
    }
    return QSize(200, 200);
}
