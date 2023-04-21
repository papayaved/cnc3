#include "center_widget.h"

#include <qwt_legend.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_picker_machine.h>
#include <qwt_scale_engine.h>
#include <qwt_plot_rescaler.h>
#include <qwt_plot_layout.h>
#include <QResizeEvent>

#include "main.h"
#include "cnc.h"

using namespace std;

CenterWidget::CenterWidget(QWidget *parent): QWidget(parent), plotView(QwtPlotView())
//    gridPlot(nullptr),
//    pickerPlot(nullptr)
{
//    qwtPlot = new QwtPlot;
//    qwtPlot->setMinimumSize(200, 200);
//    qwtPlot->setCanvasBackground( Qt::white );

    groupMode = new QGroupBox(tr("Mode"));
    radioMode1R = new QRadioButton(tr("Edge search"));
    radioMode4R = new QRadioButton(tr("4-point search (right angles)"));
    radioMode3Adj = new QRadioButton(tr("3-point search (adjustable angles)"));

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(radioMode1R);
    vbox->addWidget(radioMode4R);
    vbox->addWidget(radioMode3Adj);
    groupMode->setLayout(vbox);

    labelRadius = new QLabel(tr("Search radius") + ": ");
    fnumRadius = new QDoubleSpinBox;    
    fnumRadius->setRange(0.3, 200);
    fnumRadius->setSingleStep(0.1);
    fnumRadius->setDecimals(1);
    fnumRadius->setAccelerated(true);
    fnumRadius->setSuffix(" " + tr("mm"));
    fnumRadius->setValue(1);
    labelRadius->setBuddy(fnumRadius);

    labelFinePct = new QLabel(tr("Fine zone") + ": ");
    numFinePct = new QSpinBox;
    numFinePct->setRange(0, 100);
    numFinePct->setSingleStep(1);
    numFinePct->setValue(50);
    numFinePct->setSuffix(" %");
    labelFinePct->setBuddy(numFinePct);

    labelRollback = new QLabel(tr("Rollback") + ": ");
    fnumRollback = new QDoubleSpinBox;
    fnumRollback->setRange(0, 200);
    fnumRollback->setSingleStep(0.01);
    fnumRollback->setDecimals(2);
    fnumRollback->setAccelerated(true);
    fnumRollback->setSuffix(" " + tr("mm"));
    fnumRollback->setValue(0.100);
    labelRollback->setBuddy(fnumRollback);

    // Speed
    WireSpeed speed(0, WireSpeed::Mode::UMS);

    labelCoarseSpeed = new QLabel(tr("Coarse speed") + ": ");
    fnumCoarseSpeed = new QDoubleSpinBox;
    fnumCoarseSpeed->setAccelerated(true);
    fnumCoarseSpeed->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
    fnumCoarseSpeed->setDecimals(1);
    fnumCoarseSpeed->setSingleStep(0.1);
    fnumCoarseSpeed->setRange(speed.min(), speed.max());
    fnumCoarseSpeed->setValue(speed.max());
    fnumCoarseSpeed->setSuffix(" " + tr("um/sec"));
    labelCoarseSpeed->setBuddy(fnumCoarseSpeed);

    //
    labelFineSpeed = new QLabel(tr("Fine speed") + ": ");
    fnumFineSpeed = new QDoubleSpinBox;
    fnumFineSpeed->setAccelerated(true);
    fnumFineSpeed->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
    fnumFineSpeed->setDecimals(1);
    fnumFineSpeed->setSingleStep(0.1);
    fnumFineSpeed->setRange(speed.min(), speed.max());
    fnumFineSpeed->setValue(speed.max() / 10);
    fnumFineSpeed->setSuffix(" " + tr("um/sec"));
    labelFineSpeed->setBuddy(fnumFineSpeed);

    labelRollVel = new QLabel(tr("Roll velocity") + ": ");
    numRollVel = new QSpinBox;
    numRollVel->setRange(0, 7);
    numRollVel->setSingleStep(1);
    numRollVel->setValue(0);
    labelRollVel->setBuddy(numRollVel);

    labelThreshold = new QLabel(tr("Threshold") + ": ");
    numThreshold = new QSpinBox;
    numThreshold->setRange(7, 100);
    numThreshold->setSingleStep(1);
    numThreshold->setAccelerated(true);
    numThreshold->setSuffix(" " + tr("V"));
    numThreshold->setValue(10);
    labelThreshold->setBuddy(numThreshold);

    labelTouches = new QLabel(tr("Touches") + ": ");
    numTouches = new QSpinBox;
    numTouches->setRange(1, 100);
    numTouches->setSingleStep(1);
    numTouches->setValue(1);
    labelTouches->setBuddy(numTouches);

    labelAttempts = new QLabel(tr("Attempts") + ": ");
    numAttempts = new QSpinBox;
    numAttempts->setRange(1, 10);
    numAttempts->setSingleStep(1);
    numAttempts->setValue(1);
    labelAttempts->setBuddy(numAttempts);

    labelAngle[0] = new QLabel(tr("Angle") + " 1: ");
    numAngle[0] = new QSpinBox;
    numAngle[0]->setRange(0, 359);
    numAngle[0]->setSingleStep(1);
    numAngle[0]->setAccelerated(true);
    numAngle[0]->setValue(45);
    labelAngle[0]->setBuddy(numAngle[0]);

    labelAngle[1] = new QLabel(tr("Angle") + " 2: ");
    numAngle[1] = new QSpinBox;
    numAngle[1]->setRange(0, 359);
    numAngle[1]->setSingleStep(1);
    numAngle[1]->setAccelerated(true);
    numAngle[1]->setValue(90);
    labelAngle[1]->setBuddy(numAngle[1]);

    labelAngle[2] = new QLabel(tr("Angle") + " 3: ");
    numAngle[2] = new QSpinBox;
    numAngle[2]->setRange(0, 359);
    numAngle[2]->setSingleStep(1);
    numAngle[2]->setAccelerated(true);
    numAngle[2]->setValue(135);
    labelAngle[2]->setBuddy(numAngle[2]);

    //
    speedMMM = new QRadioButton(tr("mm/min"));
    speedUMS = new QRadioButton(tr("um/sec"));

    QGroupBox* groupSpeed = new QGroupBox;
    groupSpeed->setLayout(new QHBoxLayout);
    groupSpeed->layout()->addWidget(speedMMM);
    groupSpeed->layout()->addWidget(speedUMS);
    groupSpeed->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    speedUMS->click();

    btnResetPos = new QPushButton(tr("Reset position"));
    btnResetPos->setStatusTip("G92 X0 Y0");

    gridControl = new QGridLayout;
    gridControl->addWidget(groupMode, 0, 0, 1, 2);
    gridControl->addWidget(labelRadius, 1, 0, Qt::AlignRight);
    gridControl->addWidget(fnumRadius, 1, 1, Qt::AlignLeft);
    gridControl->addWidget(labelFinePct, 2, 0, Qt::AlignRight);
    gridControl->addWidget(numFinePct, 2, 1, Qt::AlignLeft);
    gridControl->addWidget(labelRollback, 3, 0, Qt::AlignRight);
    gridControl->addWidget(fnumRollback, 3, 1, Qt::AlignLeft);
    gridControl->addWidget(labelCoarseSpeed, 4, 0, Qt::AlignRight);
    gridControl->addWidget(fnumCoarseSpeed, 4, 1, Qt::AlignLeft);
    gridControl->addWidget(labelFineSpeed, 5, 0, Qt::AlignRight);
    gridControl->addWidget(fnumFineSpeed, 5, 1, Qt::AlignLeft);
    gridControl->addWidget(labelRollVel, 6, 0, Qt::AlignRight);
    gridControl->addWidget(numRollVel, 6, 1, Qt::AlignLeft);
    gridControl->addWidget(labelThreshold, 7, 0, Qt::AlignRight);
    gridControl->addWidget(numThreshold, 7, 1, Qt::AlignLeft);
    gridControl->addWidget(labelTouches, 8, 0, Qt::AlignRight);
    gridControl->addWidget(numTouches, 8, 1, Qt::AlignLeft);
    gridControl->addWidget(labelAttempts, 9, 0, Qt::AlignRight);
    gridControl->addWidget(numAttempts, 9, 1, Qt::AlignLeft);
    gridControl->addWidget(labelAngle[0], 10, 0, Qt::AlignRight);
    gridControl->addWidget(numAngle[0], 10, 1, Qt::AlignLeft);
    gridControl->addWidget(labelAngle[1], 11, 0, Qt::AlignRight);
    gridControl->addWidget(numAngle[1], 11, 1, Qt::AlignLeft);
    gridControl->addWidget(labelAngle[2], 12, 0, Qt::AlignRight);
    gridControl->addWidget(numAngle[2], 12, 1, Qt::AlignLeft);
    gridControl->addWidget(groupSpeed, 13, 0, 1, 2, Qt::AlignLeft);
    gridControl->addWidget(new QFrame, 14, 0, 1, 2);
    gridControl->addWidget(btnResetPos, 15, 0, 1, 2, Qt::AlignBottom);

    // Limit Switches
    labelFWD = new QLabel("FWD");
    labelREV = new QLabel("REV");
    labelWB = new QLabel("WB");
    labelALM = new QLabel("ALM");
    labelPWR = new QLabel("PWR");

    labelFB = new QLabel("FB");
    labelTO = new QLabel("TO");

    checkLabels = {labelTO, labelFB, labelPWR, labelALM, labelWB, labelREV, labelFWD};

    checkPWR = new QCheckBox;
    checkPWR->setEnabled(false);
    checkFB = new QCheckBox;
    checkFB->setEnabled(false);
    checkTO = new QCheckBox;
    checkTO->setEnabled(false);
    checkWB = new QCheckBox;
    checkWB->setEnabled(false);
    checkALM = new QCheckBox;
    checkALM->setEnabled(false);
    checkFWD = new QCheckBox;
    checkFWD->setEnabled(false);
    checkREV = new QCheckBox;
    checkREV->setEnabled(false);

    txtMsg = new QTextEdit;
    txtMsg->setReadOnly(true);
//    txtMsg->setEnabled(false);

    int h = txtMsg->fontMetrics().lineSpacing();
    h *= 5;
//    h += txtCode->fontMetrics().leading();

    QTextDocument* pdoc = txtMsg->document();
    QMargins margins = txtMsg->contentsMargins();
    h += static_cast<int>(pdoc->documentMargin()) + margins.top() + margins.bottom();

    txtMsg->setFixedHeight(h);

    //
    QGridLayout *gridLS = new QGridLayout;

    gridLS->addWidget(labelTO, 0, 0, Qt::AlignCenter);
    gridLS->addWidget(checkTO, 1, 0, Qt::AlignCenter);

    gridLS->addWidget(labelFB, 0, 1, Qt::AlignCenter);
    gridLS->addWidget(checkFB, 1, 1, Qt::AlignCenter);

    gridLS->addWidget(labelPWR, 0, 2, Qt::AlignCenter);
    gridLS->addWidget(checkPWR, 1, 2, Qt::AlignCenter);

    gridLS->addWidget(labelWB, 0, 3, Qt::AlignCenter);
    gridLS->addWidget(checkWB, 1, 3, Qt::AlignCenter);

    gridLS->addWidget(labelALM, 0, 4, Qt::AlignCenter);
    gridLS->addWidget(checkALM, 1, 4, Qt::AlignCenter);

    gridLS->addWidget(labelREV, 0, 5, Qt::AlignCenter);
    gridLS->addWidget(checkREV, 1, 5, Qt::AlignCenter);

    gridLS->addWidget(labelFWD, 0, 6, Qt::AlignCenter);
    gridLS->addWidget(checkFWD, 1, 6, Qt::AlignCenter);

    // State
    labelX = new QLabel("X: ");
    fnumX = new QDoubleSpinBox;
    fnumX->setRange(-9999, 9999);
    fnumX->setDecimals(3);
    fnumX->setSuffix(" " + tr("mm"));
    fnumX->setReadOnly(true);
    fnumX->setEnabled(false);

    labelY = new QLabel("Y: ");
    fnumY = new QDoubleSpinBox;
    fnumY->setRange(-9999, 9999);
    fnumY->setDecimals(3);
    fnumY->setSuffix(" " + tr("mm"));
    fnumY->setReadOnly(true);
    fnumY->setEnabled(false);

    labelVolt = new QLabel(tr("Voltage") + ": ");
    fnumVolt = new QDoubleSpinBox;
    fnumVolt->setRange(0, 999);
    fnumVolt->setDecimals(0);
    fnumVolt->setSuffix(" " + tr("V"));
    fnumVolt->setReadOnly(true);
    fnumVolt->setEnabled(false);

    labelSpeed = new QLabel(tr("Speed") + ": ");
    fnumSpeed = new QDoubleSpinBox;
    fnumSpeed->setRange(0, 999);
    fnumSpeed->setDecimals(0);
    fnumSpeed->setSuffix(" " + tr("um/sec"));
    fnumSpeed->setReadOnly(true);
    fnumSpeed->setEnabled(false);

    gridState = new QGridLayout;
    gridState->addWidget(new QFrame, 0, 0, 2, 1);
    gridState->addWidget(new QFrame, 0, 1, 1, 2);
    gridState->addLayout(gridLS, 0, 3, 1, 2, Qt::AlignBottom);
    gridState->addLayout(hboxLabelNum(labelSpeed, fnumSpeed), 1, 1, Qt::AlignBottom);
    gridState->addLayout(hboxLabelNum(labelVolt, fnumVolt), 1, 2, Qt::AlignBottom);
    gridState->addLayout(hboxLabelNum(labelX, fnumX), 1, 3, Qt::AlignBottom);
    gridState->addLayout(hboxLabelNum(labelY, fnumY), 1, 4, Qt::AlignBottom);

    //
    plotView.emptyPlot();

    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->addLayout(gridControl, 0, 0, Qt::AlignLeft);
    mainLayout->addWidget(plotView.widget(), 0, 1);
    mainLayout->addWidget(txtMsg, 1, 0);
    mainLayout->addLayout(gridState, 1, 1, Qt::AlignRight | Qt::AlignBottom);
    mainLayout->setColumnStretch(0, 4);
    mainLayout->setColumnStretch(1, 10);

    this->setLayout(mainLayout);

//    connect(fnumRadius, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=](double) {
//        emptyPlot();
//    });

    connect(speedMMM, &QRadioButton::clicked, this, [&]() {
        if (wireSpeedMode != WireSpeed::Mode::MMM) {
            changeSpinBoxSpeed(fnumSpeed, wireSpeedMode, WireSpeed::Mode::MMM);
            changeSpinBoxSpeed(fnumCoarseSpeed, wireSpeedMode, WireSpeed::Mode::MMM);
            changeSpinBoxSpeed(fnumFineSpeed, wireSpeedMode, WireSpeed::Mode::MMM);
            wireSpeedMode = WireSpeed::Mode::MMM;
        }
    });
    connect(speedUMS, &QRadioButton::clicked, this, [&]() {
        if (wireSpeedMode != WireSpeed::Mode::UMS) {
            changeSpinBoxSpeed(fnumSpeed, wireSpeedMode, WireSpeed::Mode::UMS);
            changeSpinBoxSpeed(fnumCoarseSpeed, wireSpeedMode, WireSpeed::Mode::UMS);
            changeSpinBoxSpeed(fnumFineSpeed, wireSpeedMode, WireSpeed::Mode::UMS);
            wireSpeedMode = WireSpeed::Mode::UMS;
        }
    });

    connect(radioMode1R, &QRadioButton::clicked, this, [&]() {
        for (QSpinBox* o: numAngle)
            o->setEnabled(false);

        fnumRollback->setEnabled(true);
        numAttempts->setEnabled(false);
        emit modeChanged(CenterMode::Mode1R);
    });

    connect(radioMode3Adj, &QRadioButton::clicked, this, [&]() {
        for (QSpinBox* o: numAngle)
            o->setEnabled(true);

        fnumRollback->setEnabled(false);
        numAttempts->setEnabled(true);
        emit modeChanged(CenterMode::Mode3Adj);
    });

    connect(radioMode4R, &QRadioButton::clicked, this, [&]() {
        for (QSpinBox* o: numAngle)
            o->setEnabled(false);

        fnumRollback->setEnabled(false);
        numAttempts->setEnabled(true);
        emit modeChanged(CenterMode::Mode4R);
    });

//    radioMode1R->click();
}

void CenterWidget::changeSpinBoxSpeed(QDoubleSpinBox*& fnumSpeed, WireSpeed::Mode cur_mode, WireSpeed::Mode new_mode) {
    if (new_mode == cur_mode)
        return;

    switch (new_mode) {
    case WireSpeed::Mode::MMM: {
        WireSpeed speed(fnumSpeed->value(), WireSpeed::Mode::UMS);
        speed.changeMode(WireSpeed::Mode::MMM);
        fnumSpeed->setDecimals(2);
        fnumSpeed->setSingleStep(0.01);
        fnumSpeed->setRange(speed.min(), speed.max());
        fnumSpeed->setValue(speed.get());        
        fnumSpeed->setSuffix(" " + tr("mm/min"));
    }
        break;

    case WireSpeed::Mode::UMS: {
        WireSpeed speed(fnumSpeed->value(), WireSpeed::Mode::MMM);
        speed.changeMode(WireSpeed::Mode::UMS);
        fnumSpeed->setDecimals(1);
        fnumSpeed->setSingleStep(0.1);
        fnumSpeed->setRange(speed.min(), speed.max());
        fnumSpeed->setValue(speed.get());        
        fnumSpeed->setSuffix(" " + tr("um/sec"));
    }
        break;
    }
}

QLayout* CenterWidget::hboxLabelNum(QLabel* label, QAbstractSpinBox* num) {
//    label->setBuddy(num);
    QGridLayout* grid = new QGridLayout;
    grid->addWidget(label, 0, 0, Qt::AlignRight);
    grid->addWidget(num, 0, 1, Qt::AlignLeft);
//    hbox->setSizeConstraint(QLayout::SetMinimumSize);
    return grid;
}

CenterWidget::~CenterWidget() {
    //    delete qwtPlot;
}

// times
int CenterWidget::touches() const { return numTouches->value(); }
// times
int CenterWidget::attempts() const { return numAttempts->value(); }
// mm
double CenterWidget::radius() const { return fnumRadius->value(); }
// mm
double CenterWidget::rollback() const { return fnumRollback->value(); }
// %
int CenterWidget::fineSharePct() const { return numFinePct->value(); }
// grade
int CenterWidget::rollVel() const { return numRollVel->value(); }
// radian
double CenterWidget::angle(size_t i) const { return i < sizeof(numAngle) / sizeof(numAngle[0]) ? numAngle[i]->value() * (2.0 * M_PI / 360.0) : 0; }

// mm/min
double CenterWidget::speedCoarse() const {
    double value = fnumCoarseSpeed->value();
    if (speedUMS->isChecked())
        value = CncParam::ums_to_mmm(value);
    return value;
}
// mm/min
double CenterWidget::speedFine() const {
    double value = fnumFineSpeed->value();
    if (speedUMS->isChecked())
        value = CncParam::ums_to_mmm(value);
    return value;
}
// V
int CenterWidget::threshold() const { return numThreshold->value(); }

// Limit switches
void CenterWidget::setLimitSwitches(CncLimitSwitches ls) {
    checkFWD->setChecked(ls.limsw_fwd);
    checkREV->setChecked(ls.limsw_rev);
    checkALM->setChecked(ls.limsw_alm);
    checkWB->setChecked(ls.wire_break);
    checkPWR->setChecked(ls.pwr);
    checkFB->setChecked(ls.fb);
    checkTO->setChecked(ls.fb_to);
}

void CenterWidget::setLimitSwitches(bool fwd, bool rev, bool alarm, bool wire_break, bool pwr, bool feedback, bool fb_timeout) {
    CncLimitSwitches ls;

    ls.limsw_fwd = fwd;
    ls.limsw_rev = rev;
    ls.limsw_alm = alarm;
    ls.wire_break = wire_break;
    ls.pwr = pwr;
    ls.fb = feedback;
    ls.fb_to = fb_timeout;
    ls.res = 0;

    setLimitSwitches(ls);
}

void CenterWidget::setX(int32_t value) {
    fnumX->setValue(value * (1.0 / CncParam::scaleX));
}

void CenterWidget::setY(int32_t value) {
    fnumY->setValue(value * (1.0 / CncParam::scaleY));
}

// ADC
void CenterWidget::setAdc(double value) {
    fnumVolt->setValue(round(value));
}

void CenterWidget::setAdc(const cnc_adc_volt_t &adc) {
    setAdc(adc.gap.value);
}

// mm/min
void CenterWidget::setSpeed(double F) {
    WireSpeed speed(F);
    speed.changeMode(wireSpeedMode);

    if (speed.get() < speed.min()) {
        fnumSpeed->setValue(speed.min());
        qDebug("Error: MIN CNC Speed: %f", F);
    }
    else if (speed.get() > speed.max()) {
        fnumSpeed->setValue(speed.max());
        qDebug("Error: MAX CNC Speed: %f", F);
    }
    else
        fnumSpeed->setValue(speed.get());
}

// Plot
void CenterWidget::emptyPlot() {
    m_pts.clear();
    plotView.emptyPlot();
}

void CenterWidget::addToPlot(int x, int y, ContourRange& range, const QColor& color, const QColor& tipColor, int width, Qt::PenStyle style) {
    if (!m_pts.empty()) {
        int x_last = round(m_pts.back().x() * CncParam::scaleX);
        int y_last = round(m_pts.back().y() * CncParam::scaleY);

        if (x == x_last && y == y_last)
            return;
    }

    QPointF pt = {x / CncParam::scaleX, y / CncParam::scaleY};

    m_pts.push_back(pt);

    plotView.plot(m_pts, range, color, tipColor, width, style);
}

//void CenterWidget::emptyPlot() {
//    if (!qwtPlot) return;

//    m_pts.clear();
//    qwtPlot->detachItems();
////    tweakPlot();
//    qwtPlot->replot();
//}

//void CenterWidget::addPlot(int x, int y, const QColor& color, const QColor& tipColor, int width, Qt::PenStyle style) {
//    if (!m_pts.empty()) {
//        int x_last = round(m_pts.back().x() * X_SCALE);
//        int y_last = round(m_pts.back().y() * Y_SCALE);

//        if (x == x_last && y == y_last)
//            return;
//    }

//    QPointF pt = {x * (1.0 / X_SCALE), y * (1.0 / Y_SCALE)};

//    qwtPlot->detachItems();
//    m_pts.push_back(pt);

//    {
//        QwtPlotCurve* curve = new QwtPlotCurve;
//        curve->setPen(color, width, style);
//        curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

//        curve->setSamples(m_pts);

//        curve->attach(qwtPlot);
//    }

//    {
//        QwtPlotCurve* curve = new QwtPlotCurve;
//        curve->setPen(tipColor, width);
//        curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

//        QwtSymbol* symbol = new QwtSymbol( QwtSymbol::Style::Ellipse, QBrush(tipColor), QPen(tipColor, width), QSize(width, width) );
//        curve->setSymbol(symbol);

//        curve->setSamples( {pt} );
//        curve->attach(qwtPlot);
//    }

//    tweakPlot();

//    qwtPlot->replot();
//}

//void CenterWidget::tweakPlot() {
//    QwtPlotGrid* grid = new QwtPlotGrid;
//    grid->setMajorPen(QPen( Qt::gray, 1 ));
//    grid->setMinorPen(Qt::black, 1, Qt::PenStyle::DotLine);
//    grid->enableXMin(true);
//    grid->enableYMin(true);
//    grid->attach(qwtPlot);

////    double R = fabs(fnumRadius->value());
////    double step = calcPlotStep(-R, R, 10);
////    QPointF pt0 = m_pts.empty() ? QPointF(0,0) : m_pts.front();

////    qwtPlot->setAxisScale(QwtPlot::xBottom, pt0.x() - R, pt0.x() + R, step);
////    qwtPlot->setAxisScale(QwtPlot::yLeft, pt0.y() - R, pt0.y() + R, step);

//    qwtPlot->setAxisTitle(QwtPlot::Axis::xBottom, "X");
//    qwtPlot->setAxisTitle(QwtPlot::Axis::yLeft, "Y");

//    QwtPlotRescaler* rescaler = new QwtPlotRescaler( qwtPlot->canvas() );
//    rescaler->setReferenceAxis(QwtPlot::xBottom);
////    rescaler->setAspectRatio(QwtPlot::yLeft, 1.0);
//    rescaler->setRescalePolicy(QwtPlotRescaler::Fixed);
//    rescaler->setEnabled(true);
//    rescaler->rescale();
//    qwtPlot->plotLayout()->setAlignCanvasToScales(true);

//    qwtPlot->updateAxes();
////    plot->updateGeometry();
////    plot->update();
////    plot->show();
//}

double CenterWidget::calcPlotStep(double min, double max, size_t n) {
    double w = max - min;
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

    return step * exp;
}

void CenterWidget::setFontPointSize(QWidget *w, int pointSize) {
    QFont font = w->font();
    font.setPointSize(pointSize);
    w->setFont(font);
}

void CenterWidget::setFontPointSize(int /*pointSize*/) {
//    for (QWidget* w: widgets)
//        setFontPointSize(w, pointSize);
}

void CenterWidget::plot(const ContourList& contourList, bool swapXY, bool inverseX, bool inverseY, bool showXY) {
    plotView.setSwapXY(swapXY);
    plotView.setInverseX(inverseX);
    plotView.setInverseY(inverseY);
    plotView.setShowXY(showXY);
    plotView.plot(contourList);
}

QSize CenterWidget::plotViewSize() const {
    return plotViewSize(size());
}

QSize CenterWidget::plotViewSize(const QSize& formSize) const {
    int stateHeight = gridState->sizeHint().height();
    int controlWidth = gridControl->sizeHint().width();

    int h = formSize.height() - stateHeight;
    int w = formSize.width() - controlWidth;

    QSize res(w, h);

    qDebug() << "Widget size: " << size() << ", plot size:" << res << ", msg height: " << stateHeight << ", state width: " << controlWidth;

    return res;
}

void CenterWidget::resizeEvent(QResizeEvent* event) {
    if (!plotView.onResizeEvent( plotViewSize(event->size()) ))
        event->ignore();
}
