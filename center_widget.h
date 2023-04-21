#ifndef CENTERINGWIDGET_H
#define CENTERINGWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QGridLayout>
#include <QPushButton>

//#include <qwt_plot.h>
//#include <qwt_plot_grid.h>
//#include <qwt_plot_picker.h>

#include "cnc_context.h"
#include "qwt_plot_view.h"

class CenterWidget : public QWidget {
    Q_OBJECT

//    QwtPlot* qwtPlot;
//    QwtPlotGrid* gridPlot;
//    QwtPlotPicker* pickerPlot;

//    QGridLayout* mainLayout;

    QVector<QPointF> m_pts;

    static void changeSpinBoxSpeed(QDoubleSpinBox*& fnumSpeed, WireSpeed::Mode cur_mode, WireSpeed::Mode new_mode);
    static QLayout* hboxLabelNum(QLabel* label, QAbstractSpinBox* num);
    void tweakPlot();
    double calcPlotStep(double min, double max, size_t n);

    QSize plotViewSize() const;
    QSize plotViewSize(const QSize& formSize) const;

protected:
    virtual void resizeEvent(QResizeEvent *event);

public:
    enum class CenterMode {Mode1R, Mode3Adj, Mode4R, Mode4a};

    explicit CenterWidget(QWidget *parent = nullptr);
    ~CenterWidget();

    QGridLayout *gridControl, *gridState;
    QwtPlotView plotView;

    QLabel *labelRadius, *labelFinePct, *labelRollback, *labelCoarseSpeed, *labelFineSpeed, *labelRollVel, *labelThreshold, *labelHighThld, *labelTouches, *labelAttempts,\
    *labelX, *labelY, *labelVolt, *labelSpeed;

    QDoubleSpinBox *fnumRadius, *fnumRollback, *fnumCoarseSpeed, *fnumFineSpeed, *fnumX, *fnumY, *fnumVolt, *fnumSpeed;
    QSpinBox *numFinePct, *numRollVel;
    QRadioButton *speedMMM, *speedUMS;
//    QGroupBox *groupSpeed;
//    QGridLayout *gridSpeed;

    QGroupBox *groupMode;
    QRadioButton *radioMode1R, *radioMode4R, *radioMode3Adj;
    QLabel *labelAngle[3];
    QSpinBox *numAngle[3];

    QSpinBox *numTouches, *numAttempts, *numThreshold;
    QPushButton *btnResetPos;

    QLabel *labelTO, *labelFB, *labelPWR, *labelALM, *labelWB, *labelREV, *labelFWD;
    std::vector<QLabel*> checkLabels;
    QCheckBox *checkTO, *checkFB, *checkPWR, *checkALM, *checkWB, *checkREV, *checkFWD;

    QTextEdit *txtMsg;

    WireSpeed::Mode wireSpeedMode;

    void setFontPointSize(QWidget* widget, int pointSize);
    void setFontPointSize(int pointSize);

    int touches() const;
    int attempts() const;
    double radius() const;
    double rollback() const;
    double speedCoarse() const;
    double speedFine() const;
    int threshold() const;
    int thresholdHigh() const;

    int fineSharePct() const;
    // grade
    int rollVel() const;
    // radian
    double angle(size_t i) const;

    void setLimitSwitches(CncLimitSwitches ls);
    void setLimitSwitches(bool fwd, bool rev, bool alarm, bool wire_break, bool pwr, bool feedback, bool fb_timeout);

    void setX(int32_t value);
    void setY(int32_t value);

    void setAdc(double value);
    void setAdc(const cnc_adc_volt_t& adc);
    void setSpeed(double F);

    void emptyPlot();
    void addToPlot(int x, int y, ContourRange& range, const QColor& color = Qt::GlobalColor::red, const QColor& tipColor = Qt::GlobalColor::blue, int width = 2, Qt::PenStyle = Qt::SolidLine);

    void plot(const ContourList& contourList, bool swapXY, bool inverseX, bool inverseY, bool showXY);

signals:
    void modeChanged(CenterMode mode);
};

#endif // CENTERINGWIDGET_H
