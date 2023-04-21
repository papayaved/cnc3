//#include "plot_view.h"

//using namespace std;

//PlotView::PlotView() :
//    margin(40),
//    chartView(new QChartView),
//    chart(nullptr),
//    m_range(ContourRange()),
//    m_range_valid(false),
//    m_defaultPlotSize(QSize(200, 200)),
//    owner(true)
//{
//    chartView->setRenderHint(QPainter::Antialiasing);
//    emptyPlot();
//}

//PlotView::PlotView(const PlotView& other) :
//    margin(other.margin),
//    chartView(new QChartView(other.chartView)),
//    chart(new QChart(other.chart)),
//    m_range(other.m_range),
//    m_range_valid(other.m_range_valid),
//    m_defaultPlotSize(other.m_defaultPlotSize)
//{
//}

//PlotView::PlotView(PlotView&& other) :
//    margin(other.margin),
//    chartView(other.chartView),
//    chart(other.chart),
//    m_range(other.m_range),
//    m_range_valid(other.m_range_valid),
//    m_defaultPlotSize(other.m_defaultPlotSize)
//{
//    other.chartView = nullptr;
//    other.chart = nullptr;
//}

//PlotView::~PlotView() {
//    if (owner) {
//        delete chart;
//        delete chartView;
//    }
//}

//QChartView* PlotView::give() {
//    owner = false;
//    return chartView;
//}

//void PlotView::emptyPlot() {
//    m_range_valid = false;

//    if (chartView != nullptr) {
//        QChart* newChart = new QChart();
//        newChart->legend()->hide();

//        replaceChart(newChart);

//        QSizePolicy sp(QSizePolicy::Fixed, QSizePolicy::Fixed);
//        //    QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Preferred);
//        sp.setHeightForWidth(true);
//        chartView->setSizePolicy(sp);

//        chartView->setRenderHint(QPainter::Antialiasing);
//        chartView->setMinimumSize(200, 200);
//        chartView->setFixedSize(200, 200);
//        //    chartView->scale(1, 1);
//    }
//}

////void PlotView::plot(const ContourList& contourList, const QSize& size) {
////    QChart* newChart = new QChart();
////    newChart->legend()->hide();

////    ContourRange range;
////    vector<fpoint_t> pts;

////    for (const ContourPair& pair: contourList.contours()) {
////        pts = pair.getPointsBot();
////        range.append(pts);
////        addPlot(newChart, pts);

////        pts = pair.getPointsTop();
////        range.append(pts);
////        addPlot(newChart, pts);
////    }

////    addPlot(newChart, contourList.selected());

////    if (newChart->series().empty())
////        emptyPlot();
////    else {
////        newChart->createDefaultAxes();
////        replaceChart(newChart, range, size);
////    }
////}

//void PlotView::plot(const ContourList& contourList, const QSize& size) {
//    QChart* newChart = new QChart();
//    newChart->legend()->hide();

//    ContourRange range;
//    vector<fpoint_t> pts;

//    int i = 0, j = 0;
//    bool passed_reg = true;

//    for (const ContourPair& pair: contourList.contours()) {
//        for (const DxfEntity* ent: pair.bot().entities()) {
//            bool passed = i < contourList.currentContour() || (i == contourList.currentContour() && j <= contourList.currentSegment());

//            if (passed ^ passed_reg) {
//                if (!pts.empty()) {
//                    range.append(pts);
//                    addPlot(newChart, pts, passed_reg ? Qt::GlobalColor::red : Qt::GlobalColor::blue);
//                    pts.clear();
//                }
//                passed_reg = passed;
//            }

//            copy_back(pts, ent->getPoints());

//            j++;
//        }
//        i++;
//    }

//    if (!pts.empty()) {
//        range.append(pts);
//        addPlot(newChart, pts, passed_reg ? Qt::GlobalColor::red : Qt::GlobalColor::blue);
//        pts.clear();
//    }

//    addPlot(newChart, contourList.selected());

////    fpoint_t pt(0,0);
////    range.append({pt});
////    addPoint(newChart, &pt, nullptr);

//    addPoint(newChart, contourList.botPos(), contourList.topPos());

//    if (newChart->series().empty())
//        emptyPlot();
//    else {
//        newChart->createDefaultAxes();
//        replaceChart(newChart, range, size);
//    }
//}

//void PlotView::plot(const Dxf& ctr, const QSize& size) {
//    QChart* newChart = new QChart();
//    newChart->legend()->hide();

//    vector<fpoint_t> pts( ctr.getPoints() );
//    ContourRange range(pts);
//    addPlot(newChart, pts);

//    newChart->createDefaultAxes();
//    replaceChart(newChart, range, size);
//}

//void PlotView::replaceChart(QChart*& newChart) {
//    if (newChart != nullptr) {
//        chartView->setChart(newChart);

//        if (chart != nullptr) {
//            chart->removeAllSeries();
//            delete chart;
//        }

//        chart = newChart;
//        newChart = nullptr;
//    }
//}

//void PlotView::replaceChart(QChart*& newChart, const ContourRange& range, const QSize& size) {
//    if (newChart && !newChart->series().empty()) {
//        QList<QAbstractSeries*> list = newChart->series();

//        if (!list.empty()) {
//            m_range = range;
//            m_range_valid = true;

//            qDebug()<<"QChart series count: "<<list.count();
//            int i = 0;
//            for (auto s: list) {
//                if (s->type() == QAbstractSeries::SeriesType::SeriesTypeLine) {
//                    QLineSeries* ls = dynamic_cast<QLineSeries*>(s);
//                    qDebug() << i++ << ": " << ls->count();
//                }
//            }

//            newChart->axes(Qt::Horizontal).first()->setRange(m_range.x_min, m_range.x_max);
//            newChart->axes(Qt::Vertical).first()->setRange(m_range.y_min, m_range.y_max);

//            replaceChart(newChart);

//            QSize plotSize = QSize(int(m_range.width() * 1000), int(m_range.height() * 1000));
//            qDebug() << "Axes ratio: " << plotSize;

//            plotSize.scale(size.width() - margin, size.height() - margin, Qt::KeepAspectRatio);
//            qDebug() << "Screen axes ratio: " << plotSize;

//            chartView->setFixedSize(plotSize);
//            chartView->updateGeometry();
//            chartView->update();
//            chartView->show();

//            qDebug() << "chartView size: " << chartView->size();
//        }
//        else {
//            m_range_valid = false;
//            replaceChart(newChart);
//        }
//    }
//    else {
//        m_range_valid = false;
//        replaceChart(newChart);
//    }
//}

//void PlotView::addPlot(QChart* const chart, const vector<fpoint_t>& pts, const QColor& color) {
//    if (chart) {
//        QLineSeries* series = new QLineSeries();
//        series->setColor(color);

//        for (const fpoint_t& pt: pts)
//            series->append(pt.x, pt.y);

//        chart->addSeries(series);
//    }
//}

//void PlotView::addPlot(QChart* const chart, const Dxf* const dxf) {
//    if (chart && dxf && !dxf->empty()) {
//        list<DxfEntity*> contour = dxf->entities();

//        if (!contour.empty()) {
//            QLineSeries* series = new QLineSeries();
//            series->setColor(dxf->color());

//            fpoint_t cur_pt = contour.front()->point_0();
//            series->append(cur_pt.x, cur_pt.y);

//            for (const auto& o: contour) {
//                if (o->point_0() == cur_pt) {
//                    cur_pt = o->point_1();
//                    series->append(cur_pt.x, cur_pt.y);
//                }
//                else {
//                    series->append(o->point_0().x, o->point_0().y);
//                    cur_pt = o->point_1();
//                    series->append(cur_pt.x, cur_pt.y);
//                }
//            }

//            chart->addSeries(series);
//        }
//    }
//}

//void PlotView::addPlot(QChart* const chart, const DxfEntity* const entity, const QColor& color) {
//    if (chart && entity) {
//        vector<fpoint_t> pts = entity->getPoints();

//        QLineSeries* highlight = new QLineSeries();
//        highlight->setPen(QPen(QBrush(color), 3));

//        for (const fpoint_t& pt: pts)
//            highlight->append(pt.x, pt.y);

//        chart->addSeries(highlight);
//    }
//}

//void PlotView::addPoint(QChart* const chart, const fpoint_t* const bot, const fpoint_t* const top, const QColor& botColor, const QColor& topColor) {
//    if (chart && bot) {
//        QScatterSeries* wire = new QScatterSeries;
//        wire->setColor(botColor);
//        wire->setMarkerSize(30);

//        wire->append(bot->x, bot->y);
//        chart->addSeries(wire);
//    }

//    if (chart && top) {
//        QScatterSeries* wire = new QScatterSeries;
//        wire->setColor(topColor);
//        wire->setMarkerSize(10);

//        wire->append(bot->x, bot->y);
//        chart->addSeries(wire);
//    }
//}

//bool PlotView::onResizeEvent(const QSize& size) {
//    if (m_range_valid) {
//        QSize plotSize = QSize(int(m_range.width() * 1000), int(m_range.height() * 1000));
////        qDebug() << "Axes ratio: " << plotSize;

//        plotSize.scale(size.width() - margin, size.height() - margin, Qt::KeepAspectRatio);
////        qDebug() << "Screen axes ratio: " << plotSize;

//        chartView->setFixedSize(plotSize);
////        qDebug() << "chartView size: " << chartView->size();

//        chartView->updateGeometry();
//        chartView->update();

//        return true;
//    }

//    return false;
//}
//void PlotView::copy_back(vector<fpoint_t>& pts, const vector<fpoint_t>& ent_pts) { // to template with float, or uint16_t and scale
//    fpoint_t cur;

//    for (const fpoint_t& pt: ent_pts) {
//        if (pts.empty()) {
//            pts.push_back(pt);
//            cur = pt;
//        }
//        else if (pt != cur) {
//            pts.push_back(pt);
//            cur = pt;
//        }
//    }
//}

