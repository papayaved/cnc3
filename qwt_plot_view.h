#ifndef PLOT_VIEW_H
#define PLOT_VIEW_H

#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_picker.h>
#include <qwt_symbol.h>

#include "contour_list.h"
#define D_WIRE_PX (15)
#define INPUT_SQUARE_PX (10)

class QwtPlotView : public QObject {
    Q_OBJECT

    const int m_margin;

    bool m_owner {false};
    QwtPlot *m_qwtPlot {nullptr};
    QwtPlotGrid *m_grid {nullptr};
    QwtPlotPicker *m_picker {nullptr}, *m_picker2 {nullptr};

    ContourRange m_range;
    bool m_swapXY {false}, m_inverseX {false}, m_inverseY {false};
    bool m_showXY {false};

//    QVector<QVector<QPointF>> m_pts_bot, m_pts_top;

    static void swapXY(QVector<QVector<QPointF>>& pts_set);
    static void swapXY(QVector<QPointF>& pts);

    static void addPlot(
        QwtPlot* const newPlot, ContourRange& newRange,
        QVector<QVector<QPointF>>& pts,
        bool swapXY,
        const QColor& color, // Qt::GlobalColor::blue
        int width = 2, Qt::PenStyle = Qt::SolidLine
    );

    static void addPlot(
        QwtPlot* const newPlot, ContourRange& newRange,
        QVector<QPointF>& pts,
        bool swapXY,
        const QColor& color, // Qt::GlobalColor::blue
        int width = 2, Qt::PenStyle = Qt::SolidLine
    );

    static void addPlot(
        QwtPlot* const newPlot, ContourRange& newRange,
        const ContourPair* const pair,
        bool swapXY,
        const QColor& colorBot, const QColor& colorTop,
        int width = 2,
        Qt::PenStyle style = Qt::SolidLine
    );
    static void addPlot(
        QwtPlot* const newPlot, ContourRange& newRange,
        const Dxf* const dxf,
        bool swapXY,
        const QColor& color, // Qt::GlobalColor::blue
        int width = 2,
        Qt::PenStyle = Qt::SolidLine
    );

    static void addPlot(
        QwtPlot* const newPlot, ContourRange& newRange,
        const DxfEntity* const entity,
        bool swapXY,
        const QColor& color, // Qt::GlobalColor::yellow
        int width = 4, Qt::PenStyle = Qt::SolidLine
    );
    static void addPoint(
        QwtPlot* const plot,
        const fpoint_t* const pt,
        bool swapXY,
        const QColor& color, // Qt::GlobalColor::blue
        QwtSymbol::Style style = QwtSymbol::Style::Ellipse,
        int size = D_WIRE_PX
    );
    static void addPoints(
        QwtPlot* const plot,
        const fpoint_t* const botPoint, const fpoint_t* const topPoint,
        bool swapXY,
        const QColor& botColor, // Qt::GlobalColor::blue,
        const QColor& topColor, // Qt::GlobalColor::cyan,
        QwtSymbol::Style style = QwtSymbol::Style::Ellipse,
        int size = D_WIRE_PX
    );

    static void addSymbolHollow(
        QwtPlot* const plot,
        const fpoint_t* const bot,
        bool swapXY,
        const QColor& botColor, // Qt::GlobalColor::red,
        const QwtSymbol::Style& style = QwtSymbol::Style::Ellipse,
        int size = D_WIRE_PX
    );

    static void addSymbolCircle(
        QwtPlot* const plot,
        const fpoint_t* const bot,
        bool swapXY,
        const QColor& botColor, // Qt::GlobalColor::red
        int size = D_WIRE_PX
    );

    void addLine(
        QwtPlot* const newPlot,
        const fpoint_t* const bot, const fpoint_t* const top,
        bool swapXY,
        const QColor& color, // Qt::GlobalColor::blue
        int width = 2, Qt::PenStyle = Qt::DotLine
    );

    void tweakPlot(QwtPlot* const newPlot, const ContourRange& range, const QSize& frameSize);
    void setAxis(const QSize& frameSize);
//    void replaceChart(QwtPlot*& newChart);
//    void replaceChart(QwtPlot*& newChart, const ContourRange& range, const QSize& size);

public:
    explicit QwtPlotView(QObject *parent = nullptr);
    QwtPlotView(const QwtPlotView& other);
    QwtPlotView(QwtPlotView&& other);
    ~QwtPlotView();

    QwtPlot* get() const { return m_qwtPlot; }
    QwtPlot* widget();

    QSize size() const { return m_qwtPlot ? m_qwtPlot->size() : QSize(0, 0); }

//    void clearPoints();
//    const QVector<QVector<QPointF>>& pointsBot() const { return m_pts_bot; };
//    const QVector<QVector<QPointF>>& pointsTop() const { return m_pts_top; };
    const ContourRange& range() const { return m_range; };

//    bool find(const QPointF& pt, size_t& ctr_num, size_t& row_num, size_t& col_num) const;

    void plot(const ContourList& contourList, const QSize& frameSize);
    void plot(const ContourList& contourList);

    void plot(const Dxf& contour, const QSize& frameSize);
    void plot(const Dxf& contour);

    void plot(QVector<QPointF>& pts, ContourRange& range, const QSize& frameSize, const QColor& color, const QColor& tipColor, int width, Qt::PenStyle style);
    void plot(QVector<QPointF>& pts, ContourRange& range, const QColor& color, const QColor& tipColor, int width, Qt::PenStyle style);
    void plot(QVector<QPointF>& pts, const QColor& color, const QColor& tipColor, int width, Qt::PenStyle style);

//    void plot(const QVector<QPointF>& pts, const QSize& frameSize, const QColor& color, const QColor& tipColor, int width, Qt::PenStyle style);

    void emptyPlot(const QSize& frameSize);
    void emptyPlot();

    bool onResizeEvent(const QSize& size);

    static void copy_back(std::vector<fpoint_t>& pts, const std::vector<fpoint_t>& ent_pts);
    static void copy_back(QVector<QPointF>& pts, const std::vector<fpoint_t>& ent_pts);

    void addSymbolSquare(
        const fpoint_t* const botPoint,        
        const QColor& botColor, // Qt::GlobalColor::red,
        int size = INPUT_SQUARE_PX
    );

    bool isSwapXY() const;
    void setSwapXY(bool value);

    bool isInverseX() const;
    void setInverseX(bool value);

    bool isInverseY() const;
    void setInverseY(bool value);

    bool isShowXY() const;
    void setShowXY(bool value);

    static double calcPlotStep(double min, double max, size_t n);

private slots:
    void on_selected(const QPointF& pos);
    void on_controlSelected(const QPointF& pos);

signals:
    void clicked(const QPointF& pt);
    void controlClicked(const QPointF& pt);
//    void clicked(size_t ctr_num, size_t row_num, size_t col_num);
};

#endif // PLOT_VIEW_H
