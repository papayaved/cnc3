#include "qwt_plot_view.h"

#include <qwt_legend.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_picker_machine.h>
#include <qwt_scale_engine.h>
#include <qwt_plot_rescaler.h>
#include <qwt_plot_layout.h>

using namespace std;

QwtPlotView::QwtPlotView(QObject *parent) :
    QObject(parent),
    m_margin(40),
    m_owner(true),
    m_qwtPlot(new QwtPlot),
    m_grid(nullptr),
    m_picker(nullptr),
    m_range(ContourRange(0, 100, 0, 100))
{
    if (m_qwtPlot) {
        m_qwtPlot->setMinimumSize(200, 200);

        emptyPlot();

        m_picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                                                  QwtPicker::CrossRubberBand, QwtPicker::ActiveOnly,
                                                  m_qwtPlot->canvas());

        m_picker->setStateMachine(new QwtPickerClickPointMachine());

        connect(m_picker, &QwtPlotPicker::appended, this, &QwtPlotView::on_pointSelected);

//        m_pts_bot.resize(2);
//        m_pts_top.resize(2);
    }
}

QwtPlotView::QwtPlotView(const QwtPlotView& other) :
    QObject(other.parent()),
    m_margin(other.m_margin),
    m_owner(true),
    m_qwtPlot(new QwtPlot(other.m_qwtPlot)),
    m_range(other.m_range)
{
    if (m_qwtPlot)
        m_qwtPlot->setMinimumSize(QSize(200, 200));

//    m_pts_bot = other.m_pts_bot;
//    m_pts_top = other.m_pts_top;
}

QwtPlotView::QwtPlotView(QwtPlotView&& other) :
    QObject(other.parent()),
    m_margin(other.m_margin),
    m_owner(other.m_owner),
    m_qwtPlot(other.m_qwtPlot),
    m_range(other.m_range)
{
    if (m_qwtPlot)
        m_qwtPlot->setMinimumSize(200, 200);

    other.m_owner = false;
    other.m_qwtPlot = nullptr;

//    m_pts_bot.swap(other.m_pts_bot);
//    m_pts_top.swap(other.m_pts_top);
}

QwtPlotView::~QwtPlotView() {
    if (m_owner && m_qwtPlot) {
        delete m_qwtPlot;
        m_qwtPlot = nullptr;
    }
}

QwtPlot* QwtPlotView::widget() {
    m_owner = false;
    return m_qwtPlot;
}

void QwtPlotView::emptyPlot(const QSize& frameSize) {
    if (!m_qwtPlot) return;

//    QSizePolicy sp(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Preferred);
//    sp.setHeightForWidth(true);
    m_qwtPlot->setSizePolicy(sp);

    m_qwtPlot->detachItems();

#ifdef DARK_PLOT_THEME
    m_qwtPlot->setCanvasBackground(Qt::black);
//    m_qwtPlot->canvas()->palette().;
#else
    m_qwtPlot->setCanvasBackground(Qt::white);
#endif

    m_range = ContourRange(0, 100, 0, 100);
    setAxis(frameSize);

    m_qwtPlot->replot();
}

void QwtPlotView::emptyPlot() {
    if (m_qwtPlot)
        emptyPlot(m_qwtPlot->size());
}

//void QwtPlotView::clearPoints() {
//    for (QVector<QPointF>& ctr: m_pts_bot)
//        ctr.clear();

//    for (QVector<QPointF>& ctr: m_pts_top)
//        ctr.clear();
//}

void QwtPlotView::plot(const ContourList& contourList, const QSize& frameSize) {
    ContourRange range;
    QVector<QPointF> pts;

    if (!m_qwtPlot)
        return;

//    clearPoints();
//    m_pts_bot.resize( contourList.contours().size() );
//    m_pts_top.resize( contourList.contours().size() );

    m_qwtPlot->detachItems(QwtPlotItem::Rtti_PlotItem, true); // These are default parameters

    bool passed_bot_reg = true, passed_top_reg = true;

//    for (const ContourPair& pair: contourList.contours()) {
//        for (const DxfEntity* ent: pair.bot()->entities()) {
//            bool passed = i < contourList.currentContourNumber() || (i == contourList.currentContourNumber() && j <= contourList.currentSegmentNumber());

//            if (passed ^ passed_bot_reg) {
//                if (!pts_bot.empty()) {
//#ifdef DARK_PLOT_THEME
//                    addPlot(m_qwtPlot, range, pts_bot, m_swapXY, passed_bot_reg ? Qt::GlobalColor::red : Qt::GlobalColor::green);
//#else
//                    addPlot(m_qwtPlot, range, pts_bot, m_swapXY, passed_bot_reg ? Qt::GlobalColor::red : Qt::GlobalColor::blue);
//#endif
//                    pts_bot.clear();
//                }
//                passed_bot_reg = passed;
//            }

//            if (ent)
//                copy_back(pts_bot, ent->getPoints());

//            j++;
//        }

//        for (const DxfEntity* ent: pair.top()->entities()) {
//            bool passed = i < contourList.currentContourNumber() || (i == contourList.currentContourNumber() && k <= contourList.currentSegmentNumber());

//            if (passed ^ passed_top_reg) {
//                if (!pts_top.empty()) {
//#ifdef DARK_PLOT_THEME
//                    addPlot(m_qwtPlot, range, pts_top, m_swapXY, passed_top_reg ? Qt::GlobalColor::darkRed : Qt::GlobalColor::darkGreen);
//#else
//                    addPlot(m_qwtPlot, range, pts_top, m_swapXY, passed_top_reg ? Qt::GlobalColor::darkRed : Qt::GlobalColor::darkBlue);
//#endif
//                    pts_top.clear();
//                }
//                passed_top_reg = passed;
//            }

//            if (ent)
//                copy_back(pts_top, ent->getPoints());

//            k++;
//        }

//        i++;
//    }

    const DxfEntity *selBotEnt {nullptr}, *selTopEnt {nullptr};
    const ContourPair *selPair {nullptr};

    size_t ctr {0}, row {0};

    for (const ContourPair& pair: contourList.contours()) {
        if (contourList.isContourSelected() && ctr == contourList.selectedContour())
            selPair = &pair;

        // BOT
        row = 0;
        for (const DxfEntity* ent: pair.bot()->entities()) {
            bool a = contourList.isSegmentSelected();
            bool b = ctr == contourList.selectedContour();
            bool c = row == contourList.selectedRow();

//            if (contourList.isSegmentSelected() && ctr == contourList.selectedContour() && row == contourList.selectedRow())
            if (a && b && c)
                selBotEnt = ent;

            bool passed = (int)ctr < contourList.currentContourNumber() || ((int)ctr == contourList.currentContourNumber() && (int)row <= contourList.currentSegmentNumber());

            if (!pts.empty()) {
#ifdef DARK_PLOT_THEME
                addPlot(m_qwtPlot, range, pts, m_swapXY, passed_bot_reg ? Qt::GlobalColor::red : Qt::GlobalColor::green);
#else
                addPlot(m_qwtPlot, range, pts_bot, m_swapXY, passed_bot_reg ? Qt::GlobalColor::red : Qt::GlobalColor::blue);
#endif
//                m_pts_bot[ctr].append(pts);
                pts.clear();
            }

            passed_bot_reg = passed;

            if (ent)
                copy_back(pts, ent->getPoints());

            row++;
        }

        if (!pts.empty()) {
#ifdef DARK_PLOT_THEME
            addPlot(m_qwtPlot, range, pts, m_swapXY, passed_bot_reg ? Qt::GlobalColor::red : Qt::GlobalColor::green);
#else
            addPlot(m_qwtPlot, range, pts_bot, m_swapXY, passed_bot_reg ? Qt::GlobalColor::red : Qt::GlobalColor::blue);
#endif
//            m_pts_bot[ctr].append(pts);
            pts.clear();
        }

        // TOP
        row = 0;
        for (const DxfEntity* ent: pair.top()->entities()) {
            if (contourList.isSegmentSelected() && ctr == contourList.selectedContour() && row == contourList.selectedRow())
                selTopEnt = ent;

            bool passed = (int)ctr < contourList.currentContourNumber() || ((int)ctr == contourList.currentContourNumber() && (int)row <= contourList.currentSegmentNumber());

            if (!pts.empty()) {
#ifdef DARK_PLOT_THEME
                addPlot(m_qwtPlot, range, pts, m_swapXY, passed_top_reg ? Qt::GlobalColor::darkRed : Qt::GlobalColor::darkGreen);
#else
                addPlot(m_qwtPlot, range, pts_top, m_swapXY, passed_top_reg ? Qt::GlobalColor::darkRed : Qt::GlobalColor::darkBlue);
#endif
//                m_pts_top[ctr].append(pts);
                pts.clear();
            }

            passed_top_reg = passed;

            if (ent)
                copy_back(pts, ent->getPoints());

            row++;
        }

        if (!pts.empty()) {
            addPlot(m_qwtPlot, range, pts, m_swapXY, passed_top_reg ? Qt::GlobalColor::darkRed : Qt::GlobalColor::darkBlue, 2);

//            m_pts_top[ctr].append(pts);
            pts.clear();
        }

        ctr++;
    }

    addPlot(m_qwtPlot, range, selPair, m_swapXY, Qt::GlobalColor::yellow, Qt::GlobalColor::yellow);

    addPlot(m_qwtPlot, range, selBotEnt, m_swapXY, Qt::GlobalColor::yellow);
    addPlot(m_qwtPlot, range, selTopEnt, m_swapXY, Qt::GlobalColor::yellow);

//    fpoint_t pt(0,0);
//    range.append({pt});
//    addPoint(newChart, &pt, nullptr);

#ifdef DARK_PLOT_THEME
    addPoints(m_qwtPlot, contourList.xyPos(), contourList.uvPos(), m_swapXY, Qt::GlobalColor::green, Qt::GlobalColor::cyan);
#else
    addPoints(m_qwtPlot, contourList.xyPos(), contourList.uvPos(), m_swapXY, Qt::GlobalColor::blue, Qt::GlobalColor::cyan);
#endif

    if (contourList.hasUVPos())
        addSymbolCircle(m_qwtPlot, contourList.botPos(), m_swapXY, Qt::GlobalColor::red);
    else {
#ifdef DARK_PLOT_THEME
        addPoint(m_qwtPlot, contourList.botPos(), m_swapXY, Qt::GlobalColor::green);
#else
        addPoint(m_qwtPlot, contourList.botPos(), m_swapXY, Qt::GlobalColor::blue);
#endif
    }

    addLine(m_qwtPlot, contourList.xyPos(), contourList.uvPos(), m_swapXY, Qt::GlobalColor::blue);

    tweakPlot(m_qwtPlot, range, frameSize);
}

void QwtPlotView::plot(const ContourList& contourList) {
    if (m_qwtPlot)
        plot(contourList, m_qwtPlot->size());
}

void QwtPlotView::plot(const Dxf& ctr, const QSize& frameSize) {
    ContourRange range;
    QVector<QPointF> pts;

    if (!m_qwtPlot)
        return;

    m_qwtPlot->detachItems();

    if (ctr.empty())
        return;

    copy_back(pts, ctr.getPoints());

#ifdef DARK_PLOT_THEME
    addPlot(m_qwtPlot, range, pts, m_swapXY, Qt::GlobalColor::green);
#else
    addPlot(m_qwtPlot, range, pts, m_swapXY, Qt::GlobalColor::blue);
#endif

    tweakPlot(m_qwtPlot, range, frameSize);
}

void QwtPlotView::plot(const Dxf& ctr) {
    if (m_qwtPlot)
        plot(ctr, m_qwtPlot->size());
}

void QwtPlotView::plot(QVector<QPointF> &pts, ContourRange& range, const QSize &frameSize, const QColor& color, const QColor& tipColor, int width, Qt::PenStyle style) {
    m_qwtPlot->detachItems();

    if (pts.empty())
        return;

    addPlot(m_qwtPlot, range, pts, m_swapXY, color, width, style);

    const QPointF& pt = pts.at(pts.count() - 1);
    fpoint_t last_pt(pt.x(), pt.y());

    addPoint(m_qwtPlot, &last_pt, m_swapXY, tipColor, QwtSymbol::Style::Ellipse);

    tweakPlot(m_qwtPlot, range, frameSize);
}

void QwtPlotView::plot(QVector<QPointF> &pts, ContourRange& range, const QColor& color, const QColor& tipColor, int width, Qt::PenStyle style) {
    if (m_qwtPlot)
        plot(pts, range, m_qwtPlot->size(), color, tipColor, width, style);
}

void QwtPlotView::plot(QVector<QPointF> &pts, const QColor& color, const QColor& tipColor, int width, Qt::PenStyle style) {
    if (m_qwtPlot) {
        ContourRange range;
        plot(pts, range, m_qwtPlot->size(), color, tipColor, width, style);
    }
}

//void QwtPlotView::plot(const QVector<QPointF> &pts, const QSize &frameSize, const QColor& color, const QColor& tipColor, int width, Qt::PenStyle style) {
//    ContourRange range;
//    plot(pts, range, frameSize, color, tipColor, width, style);
//}

void QwtPlotView::tweakPlot(QwtPlot* const plot, const ContourRange& range, const QSize& frameSize) {
    static QSize frameSize_old = {0, 0};

    if (!m_qwtPlot) return;

    bool print_new = frameSize != frameSize_old || !range.equal(m_range);
    frameSize_old = frameSize;

    if (plot && !plot->itemList().empty()) {
        m_grid = new QwtPlotGrid();

#ifdef DARK_PLOT_THEME
        m_qwtPlot->setCanvasBackground(Qt::black);
        m_grid->setMajorPen( QPen(Qt::darkGray, 1) );
        m_grid->setMinorPen(Qt::gray, 1, Qt::PenStyle::DotLine);
#else
        m_qwtPlot->setCanvasBackground(Qt::white);
        m_grid->setMajorPen( QPen(Qt::gray, 1) );
        m_grid->setMinorPen(Qt::black, 1, Qt::PenStyle::DotLine);
#endif

        m_grid->enableXMin(true);
        m_grid->enableYMin(true);
        m_grid->attach(m_qwtPlot);

        // todo: delete picker
//        QwtPlotPicker* picker = new QwtPlotPicker(
//            QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::CrossRubberBand, QwtPicker::ActiveOnly, qwtPlot->canvas()
//        );
//        picker->setRubberBandPen( QColor(Qt::red) );
//        picker->setTrackerPen( QColor(Qt::black) );
//        picker->setStateMachine( new QwtPickerDragPointMachine );

        m_range = range;

        if (print_new) qDebug() << "Plot items: " << plot->itemList().count();

        setAxis(frameSize);

        plot->updateGeometry();
        plot->update();
        plot->show();

        if (print_new) qDebug() << "Plot size: " << plot->size();
    }
    else
        emptyPlot(frameSize);

    QwtScaleEngine* engine = plot->axisScaleEngine(QwtPlot::yLeft);
    if (engine)
        engine->setAttribute(QwtScaleEngine::Attribute::Inverted, true);
//    qwtPlot->setAxisScaleEngine(QwtPlot::yLeft, );

    if (plot)
        plot->replot();
}

void QwtPlotView::setAxis(const QSize& frameSize) {
    static ContourRange range_old;
    static QSize frameSize_old = {0, 0};

    if (!m_qwtPlot) return;

    if (!m_range.valid || frameSize.width() == 0 || frameSize.height() == 0) return;

    bool print_new = !m_range.equal(range_old) || frameSize != frameSize_old;
    range_old = m_range;
    frameSize_old = frameSize;

    double width = m_range.width();
    double height = m_range.height();

    if (print_new) qDebug() << "Aspect ratio " << frameSize.width() << ":" << frameSize.height();
    if (print_new) qDebug() << "Plot ratio " << width << ":" << height;

    double aspect_ratio = static_cast<double>(frameSize.width() - D_WIRE_PX) / static_cast<double>(frameSize.height() - D_WIRE_PX);

    double scale; // mm / pixel

    if (width < 0.001 && height < 0.001)
        scale = 1;
    else {
        double plot_ratio = width / height;

        if (plot_ratio > aspect_ratio)
            scale = width / (frameSize.width() - D_WIRE_PX);
        else
            scale = height / (frameSize.height() - D_WIRE_PX);
    }

    if (print_new) qDebug() << "Scale " << scale << " mm / pixel";

    width = frameSize.width() * scale; // mm
    height = frameSize.height() * scale; // mm
    double wire_mm = D_WIRE_PX * scale;

    if (print_new) qDebug() << "New plot ratio " << width << ":" << height;

//    double x_min = m_range.x_min - wire_mm / 2;
//    double x_max = m_range.x_min + width + wire_mm / 2;
//    double y_min = m_range.y_min - wire_mm / 2;
//    double y_max = m_range.y_min + height + wire_mm / 2;

    double x_mean = m_range.meanX();
    double x_delta = (width + wire_mm) / 2;
    double x_min = x_mean - x_delta;
    double x_max = x_mean + x_delta;

    double y_mean = m_range.meanY();
    double y_delta = (height + wire_mm) / 2;
    double y_min = y_mean - y_delta;
    double y_max = y_mean + y_delta;

    double x_step = calcPlotStep(x_min, x_max, 10);
    double y_step = calcPlotStep(y_min, y_max, 10);
    double step = x_step > y_step ? x_step : y_step;

    if (m_inverseX)
        m_qwtPlot->setAxisScale(QwtPlot::xBottom, x_max, x_min, -step);
    else
        m_qwtPlot->setAxisScale(QwtPlot::xBottom, x_min, x_max, step);

    if (m_inverseY)
        m_qwtPlot->setAxisScale(QwtPlot::yLeft, y_max, y_min, -step);
    else
        m_qwtPlot->setAxisScale(QwtPlot::yLeft, y_min, y_max, step);

    if (m_showXY) {
        m_qwtPlot->setAxisTitle(QwtPlot::Axis::xBottom, m_swapXY ? "Y" : "X");
        m_qwtPlot->setAxisTitle(QwtPlot::Axis::yLeft, m_swapXY ? "X" : "Y");
    }

    m_qwtPlot->updateAxes();

//    QwtPlotRescaler* rescaler = new QwtPlotRescaler( qwtPlot->canvas() );
//    rescaler->setReferenceAxis(QwtPlot::xBottom);
//    rescaler->setAspectRatio(QwtPlot::yLeft, 1.0);
//    rescaler->setRescalePolicy(QwtPlotRescaler::Expanding);
//    rescaler->setEnabled(true);
//    rescaler->rescale();
//    qwtPlot->plotLayout()->setAlignCanvasToScales(true);

//    qwtPlot->replot();
}

void QwtPlotView::swapXY(QVector<QVector<QPointF>>& pts_set) {
    for (QVector<QPointF>& pts: pts_set)
        for (QPointF& pt: pts)
            pt = QPointF(pt.y(), pt.x());
}

void QwtPlotView::swapXY(QVector<QPointF>& pts) {
    for (QPointF& pt: pts)
        pt = QPointF(pt.y(), pt.x());
}

void QwtPlotView::addPlot(QwtPlot* const newPlot, ContourRange& newRange, QVector<QVector<QPointF>>& pts_set, bool _swapXY, const QColor& color, int width, Qt::PenStyle style) {
    if (!newPlot || pts_set.empty())
        return;

    if (_swapXY)
        swapXY(pts_set);

    newRange.append(pts_set);

    for (QVector<QPointF>& pts : pts_set) {
        QwtPlotCurve* curve = new QwtPlotCurve;

        if (curve) {
            curve->setPen(color, width, style);
            curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

            curve->setSamples(pts);

            curve->attach(newPlot);
        }
    }
}

void QwtPlotView::addPlot(QwtPlot* const newPlot, ContourRange& newRange, QVector<QPointF>& pts, bool _swapXY, const QColor& color, int width, Qt::PenStyle style) {
    if (!newPlot || pts.empty())
        return;

    if (_swapXY)
        swapXY(pts);

    newRange.append(pts);

    QwtPlotCurve* curve = new QwtPlotCurve;
    if (curve) {
        curve->setPen(color, width, style);
        curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

        curve->setSamples(pts);

        curve->attach(newPlot);
    }
}

void QwtPlotView::addPlot(
    QwtPlot* const newPlot, ContourRange& newRange,
    const ContourPair* const pair,
    bool swapXY,
    const QColor& colorBot,
    const QColor& colorTop,
    int width,
    Qt::PenStyle style
) {
    if (newPlot && pair && !pair->empty()) {
        if (!pair->botEmpty())
            addPlot(newPlot, newRange, pair->bot(), swapXY, colorBot, width, style);

        if (!pair->topEmpty())
            addPlot(newPlot, newRange, pair->top(), swapXY, colorTop, width, style);
    }
}

void QwtPlotView::addPlot(QwtPlot* const qwtPlot, ContourRange& range, const Dxf* const dxf, bool swapXY, const QColor& color, int width, Qt::PenStyle style) {
    QVector<QPointF> pts;

    if (qwtPlot && dxf && !dxf->empty()) {
        for (const DxfEntity* const ent: dxf->entities()) {
            if (ent)
                copy_back(pts, ent->getPoints());

            if (!pts.empty()) {
                addPlot(qwtPlot, range, pts, swapXY, color, width, style);
                pts.clear();
            }
        }
    }
}

void QwtPlotView::addPlot(QwtPlot* const newPlot, ContourRange& newRange, const DxfEntity* const entity, bool swapXY, const QColor& color, int width, Qt::PenStyle style) {
    QVector<QPointF> pts;

    if (newPlot && entity) {
        copy_back(pts, entity->getPoints());
        addPlot(newPlot, newRange, pts, swapXY, color, width, style);
    }
}

void QwtPlotView::addPoint(QwtPlot* const plot, const fpoint_t* const pt, bool swapXY, const QColor& color, QwtSymbol::Style style, int size) {
    if (!plot) return;

    int width = size / 2;

    if (pt) {
        QwtPlotCurve* curve = new QwtPlotCurve;

        if (curve) {
            curve->setPen(color, width);
            curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

            QwtSymbol* symbol = new QwtSymbol( style, QBrush(color), QPen(QBrush(color), width, Qt::PenStyle::SolidLine), QSize(width, width) );
            curve->setSymbol(symbol);

            curve->setSamples( {swapXY ? QPointF(pt->y, pt->x) : QPointF(pt->x, pt->y)} );
            curve->attach(plot);
        }
    }
}

void QwtPlotView::addPoints(
        QwtPlot* const plot,
        const fpoint_t* const bot, const fpoint_t* const top,
        bool swapXY,
        const QColor& botColor, const QColor& topColor,
        QwtSymbol::Style style,
        int size
) {
    addPoint(plot, bot, swapXY, botColor, style, size);
    addPoint(plot, top, swapXY, topColor, style, size);
}

void QwtPlotView::addSymbolHollow(
    QwtPlot* const plot,
    const fpoint_t* const bot,
    bool swapXY,
    const QColor& botColor,
    const QwtSymbol::Style& style,
    int size
) {
    if (!plot) return;

    if (bot) {
        QwtPlotCurve* curve = new QwtPlotCurve;
        if (curve) {
//            curve->setPen(botColor, 1);
            QwtSymbol* symbol = new QwtSymbol( style, QBrush(botColor, Qt::BrushStyle::NoBrush), QPen(botColor, 1), QSize(size, size) );
            curve->setSymbol(symbol);
            curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

            curve->setSamples( {swapXY ? QPointF(bot->y, bot->x) : QPointF(bot->x, bot->y)} );
            curve->attach(plot);
        }
    }
}

void QwtPlotView::addSymbolCircle(QwtPlot* const plot, const fpoint_t* const bot, bool swapXY, const QColor& botColor, int size) {
    addSymbolHollow(plot, bot, swapXY, botColor, QwtSymbol::Style::Ellipse, size);
}

void QwtPlotView::addSymbolSquare(const fpoint_t* const bot, const QColor& botColor, int size) {
    addSymbolHollow(m_qwtPlot, bot, m_swapXY, botColor, QwtSymbol::Style::Rect, size);
}

bool QwtPlotView::isSwapXY() const { return m_swapXY; }
void QwtPlotView::setSwapXY(bool value) { m_swapXY = value; }

bool QwtPlotView::isInverseX() const { return m_inverseX; }
void QwtPlotView::setInverseX(bool value) { m_inverseX = value; }

bool QwtPlotView::isInverseY() const { return m_inverseY; }
void QwtPlotView::setInverseY(bool value) { m_inverseY = value; }

bool QwtPlotView::isShowXY() const { return m_showXY; }
void QwtPlotView::setShowXY(bool value) { m_showXY = value; }

void QwtPlotView::addLine(QwtPlot* const newPlot, const fpoint_t* const bot, const fpoint_t* const top, bool swapXY, const QColor& color, int width, Qt::PenStyle style) {
    if (top && bot && newPlot) {
        QVector<QPointF> pts = {
            swapXY ? QPointF(bot->y, bot->x) : QPointF(bot->x, bot->y),
            swapXY ? QPointF(top->y, top->x) : QPointF(top->x, top->y)
        };

        QwtPlotCurve* curve = new QwtPlotCurve;
        if (curve) {
            curve->setPen(color, width, style);
            curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

            curve->setSamples(pts);
            curve->attach(newPlot);
        }
    }
}

bool QwtPlotView::onResizeEvent(const QSize& newFrameSize) {
    if (!m_range.valid || !m_qwtPlot) return false;

    setAxis(newFrameSize);
    m_qwtPlot->updateGeometry();
    m_qwtPlot->update();
    return true;
}

void QwtPlotView::copy_back(vector<fpoint_t>& pts, const vector<fpoint_t>& ent_pts) { // to template with float, or uint16_t and scale
    fpoint_t cur;

    for (const fpoint_t& pt: ent_pts) {
        if (pts.empty()) {
            pts.push_back(pt);
            cur = pt;
        }
        else if (pt != cur) {
            pts.push_back(pt);
            cur = pt;
        }
    }
}

void QwtPlotView::copy_back(QVector<QPointF>& pts, const vector<fpoint_t>& ent_pts) {
    fpoint_t cur;

    for (const fpoint_t& pt: ent_pts) {
        if (pts.empty()) {
            pts.push_back(QPointF(pt.x, pt.y));
            cur = pt;
        }
        else if (pt != cur) {
            pts.push_back(QPointF(pt.x, pt.y));
            cur = pt;
        }
    }
}

double QwtPlotView::calcPlotStep(double min, double max, size_t n) {
    double w = (max - min) * 10;
    double step = w / n;

    double e = log10(step);
    e = trunc(e);
    double exp = pow(10, e);

    if (step / exp < 1)
        exp = e < 0 ? exp / 10 : exp * 10;

    step /= exp; // 1..9
    step = ceil(step);
    int step_int = static_cast<int>(step);

    if (step_int == 0)
        return 0; // error
    else if (step_int == 1)
        step = 1;
    else if (step_int == 2)
        step = 2;
    else if (step_int <= 5)
        step = 5;
    else
        step = 10;

    return step * exp / 10;
}

void QwtPlotView::on_pointSelected(const QPointF& pt) {
    qDebug("Clicked (%f, %f)", pt.x(), pt.y());

    emit clicked(pt);

//    bool valid = false;
//    size_t ctr_num = 0, row_num = 0, col_num = 0;

//    valid = find(pt, ctr_num, row_num, col_num);

//    if (valid) {
//        qDebug("Clicked (%f, %f). %s: [%d, %d]", pt.x(), pt.y(), col_num == 0 ? "BOT" : "TOP", (int)ctr_num, (int)row_num);

//        emit clicked(ctr_num, row_num, col_num);
//    }
//    else
//        qDebug("Clicked (%f, %f)", pt.x(), pt.y());
}

//bool QwtPlotView::find(const QPointF &pt, size_t &ctr_num, size_t &row_num, size_t &col_num) const {
//    double e = 1; // mm

//    if (m_range.valid)
//        e = max(m_range.width(), m_range.height()) / 100;

//    ctr_num = col_num = 0;

//    for (const QVector<QPointF>& ctr: m_pts_bot) {
//        for (row_num = 0; row_num + 1 < (size_t)ctr.size(); row_num++) {
//            const QPointF& A = ctr[row_num];
//            const QPointF& B = ctr[row_num + 1];

//            DxfLine line(A.x(), A.y(), B.x(), B.y());

//            double h = line.distance( fpoint_t(pt.x(), pt.y()) );

//            if (h <= e)
//                return true;
//        }

//        ctr_num++;
//    }

//    ctr_num = 0;
//    col_num = 1;

//    for (const QVector<QPointF>& ctr: m_pts_top) {
//        for (row_num = 0; row_num + 1 < (size_t)ctr.size(); row_num++) {
//            const QPointF& A = ctr[row_num];
//            const QPointF& B = ctr[row_num + 1];

//            DxfLine line(A.x(), A.y(), B.x(), B.y());

//            double h = line.distance( fpoint_t(pt.x(), pt.y()) );

//            if (h <= e)
//                return true;
//        }

//        ctr_num++;
//    }

//    return false;
//}
