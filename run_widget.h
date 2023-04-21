#ifndef RUN_PLOT_WIDGET_H
#define RUN_PLOT_WIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QCheckBox>

#include "gcode.h"
#include "plot_view.h"
#include "code_editor.h"
#include "qwt_plot_view.h"
#include "cnc_context.h"

class RunWidget : public QWidget {
    Q_OBJECT

public:
    CodeEditor* txtCode;
    QTextEdit* txtMsg;
    QPushButton *btnStart, *btnReverse, *btnCancel, *btnHold;

    std::vector<QPushButton*> buttons;
    std::vector<QLabel*> labels;
    std::vector<QDoubleSpinBox*> nums;
    std::vector<QRadioButton*> radio;

    QLabel* labelSpeed;
    QDoubleSpinBox* numSpeed;
    QPushButton* btnSpeed;

    QRadioButton *speedMMM, *speedUMS;
    QGroupBox* groupSpeed = nullptr;

    QCheckBox* checkIdle;

private:
#ifdef DARK_GUI_THEME
    const QString posLabelColor = R"(yellow)";
#else
    const QString posLabelColor = R"(red)";
#endif

    QwtPlotView plotView;

    QLabel *labelX, *labelY, *labelU, *labelV, *labelTime, *labelETA, *labelEncHeader, *labelEncX, *labelEncY;
    std::vector<QLabel*> posLabels;

    QLabel *labelTO, *labelFB, *labelPWR, *labelALM, *labelWB, *labelREV, *labelFWD;
    std::vector<QLabel*> checkLabels;
    QCheckBox *checkTO, *checkFB, *checkPWR, *checkALM, *checkWB, *checkREV, *checkFWD;
    QGridLayout *gridLS;
    QGridLayout *gridText, *gridState, *gridButton, *gridMain;

    QLabel *labelGap, *labelRev, *labelWorkpiece, *labelWire, *labelHV, *labelShunt;
    std::vector<QLabel*> adcLabels;
    std::vector<QLabel*> adcValueLabels;

    void createButtons();
    void createTextBoxes();
    void createState();

    QSize plotViewSize() const;
    QSize plotViewSize(const QSize& formSize) const;

protected:
    virtual void resizeEvent(QResizeEvent *event);

public:
    explicit RunWidget(QWidget *parent = nullptr);
    ~RunWidget();

    void plot(const ContourList& contourList, bool swapXY, bool inverseX, bool inverseY, bool showXY);
    void setButtonsEnabled(bool run_ena, bool reverse_ena, bool cancel_ena, bool hold_ena);
    void setButtonsText(const QString (&text)[3], const QString (&status)[3] = {"", "", ""});
    void setPositionLabels(size_t axis, int value);
    void setEncoderLabels(size_t axis, int value);
    void setElapsedTime(int h, int m, int s);
    void setElapsedTime(qint64 ms);
    void setRemainTime(int h, int m, int s);
    void setRemainTime(qint64 ms);

    void setLimitSwitches(CncLimitSwitches ls);
    void setLimitSwitches(bool fwd, bool rev, bool alarm, bool wire_break, bool pwr, bool feedback, bool fb_timeout);

    void setAdc(size_t adc_num, double value, bool mV_ena = false);
    void setAdc(size_t adc_num, double_valid_t value, bool mV_ena = false);
    void setAdc(const cnc_adc_volt_t& adc);

    void setFontPointSize(QWidget* widget, int pointSize);
    void setFontPointSize(int pointSize);
};

#endif // RUN_PLOT_WIDGET_H
