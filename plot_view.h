//#ifndef PLOT_VIEW_H
//#define PLOT_VIEW_H

//#include <QtCharts>
//#include "contour_list.h"

//class PlotView {
//    const int margin;

//    QChartView* chartView;
//    QChart* chart;

//    ContourRange m_range;
//    bool m_range_valid;

//    QSize m_defaultPlotSize;
//    bool owner;

//    void addPlot(QChart* const chart, const std::vector<fpoint_t>& pts, const QColor& color = Qt::GlobalColor::blue);
//    static void addPlot(QChart* const chart, const Dxf* const dxf);
//    static void addPlot(QChart* const chart, const DxfEntity* const dxf, const QColor& color = Qt::GlobalColor::yellow);
//    static void addPoint(QChart* const chart, const fpoint_t* const bot, const fpoint_t* const top, const QColor& botColor = Qt::GlobalColor::blue, const QColor& topColor = Qt::GlobalColor::blue);

//    void replaceChart(QChart*& newChart);
//    void replaceChart(QChart*& newChart, const ContourRange& range, const QSize& size);

//public:
//    PlotView();
//    PlotView(const PlotView& other);
//    PlotView(PlotView&& other);
//    ~PlotView();

//    QChartView* get() const { return chartView; }
//    QChartView* give();

//    void plot(const ContourList& contourList, const QSize& size);
//    void plot(const Dxf& contour, const QSize& size);
//    void emptyPlot();

//    bool onResizeEvent(const QSize& size);
//    const QSize& defaultPlotSize() const { return m_defaultPlotSize; }

//    static void copy_back(std::vector<fpoint_t>& pts, const std::vector<fpoint_t>& ent_pts);
//};

//#endif // PLOT_VIEW_H
