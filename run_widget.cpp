#include "run_widget.h"
#include <QDebug>
#include <QList>
#include "program_param.h"
#include <cmath>
#include "aux_items.h"

using namespace std;

RunWidget::RunWidget(QWidget *parent) : QWidget(parent), plotView(QwtPlotView()) {
    createButtons();
    createTextBoxes();
    createState();

//    setFontPointSize(14);

    QGridLayout *grid = new QGridLayout;
    grid->addLayout(gridState, 0, 0, Qt::AlignTop);
    grid->addLayout(gridButton, 1, 0, Qt::AlignBottom);

    gridMain = new QGridLayout;
//    gridMain->addWidget(plotView.give(), 0, 0, Qt::AlignLeft | Qt::AlignTop);

//    gridMain->addWidget(plotView.give(), 0, 0);
//    gridMain->addLayout(gridState, 0, 1, Qt::AlignRight | Qt::AlignTop);
//    gridMain->addLayout(gridText, 1, 0);
//    gridMain->addLayout(gridButton, 1, 1, Qt::AlignRight | Qt::AlignTop);

    gridMain->addWidget(plotView.widget(), 0, 0);
    gridMain->addLayout(grid, 0, 1, 2, 1, Qt::AlignRight);
    gridMain->addLayout(gridText, 1, 0);

    this->setLayout(gridMain);

    qDebug() << "RunWidget constractor: " << size();
}

void RunWidget::createButtons() {
    btnStart = new QPushButton();
//    btnStart->setShortcut(QKeySequence("Space"));

    btnReverse = new QPushButton();
    btnCancel = new QPushButton();

    btnHold = new QPushButton(tr("Hold"));
    btnHold->setCheckable(true);
    btnHold->setStatusTip(tr("Holding of stepper motors on/off"));

    setButtonsEnabled(1, 0, 0, 1);
    setButtonsText({tr("Start"), tr("Back"), tr("Cancel")}, {tr("Start cutting"), tr("Go to back"), tr("Cancel program")});

    labelSpeed = new QLabel(tr("Speed") + ": ");

    numSpeed = new QDoubleSpinBox;
    numSpeed->setAccelerated(true);
    numSpeed->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    btnSpeed = new QPushButton(tr("Enter"));
    btnSpeed->setStatusTip(tr("Enter speed"));

    QHBoxLayout* hSpeed = new QHBoxLayout();
    hSpeed->addWidget(labelSpeed);
    hSpeed->addWidget(numSpeed);
//    hSpeed->addWidget(btnSpeed);
    hSpeed->addWidget(new QFrame);

    speedMMM = new QRadioButton(tr("mm/min"));
    speedUMS = new QRadioButton(tr("um/sec"));

    checkIdle = new QCheckBox(tr("Idle run"));
//    checkIdle->setCheckState(Qt::CheckState::PartiallyChecked); // ?

    groupSpeed = new QGroupBox;
    groupSpeed->setLayout(new QHBoxLayout);
    groupSpeed->layout()->addWidget(speedMMM);
    groupSpeed->layout()->addWidget(speedUMS);

    groupSpeed->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    speedMMM->click();

    gridButton = new QGridLayout;
    gridButton->addWidget(btnStart, 0, 0, 1, 2);
    gridButton->addWidget(btnReverse, 1, 0);
    gridButton->addWidget(btnCancel, 1, 1);
    gridButton->addWidget(btnHold, 2, 0, 1, 2);
    gridButton->addLayout(hSpeed, 3, 0, 1, 2);
    gridButton->addWidget(groupSpeed, 4, 0, 1, 2);
    gridButton->addWidget(checkIdle, 5, 0, 1, 2);

    //
    buttons = {btnStart, btnReverse, btnCancel, btnHold, btnSpeed};
    labels = {labelSpeed};
    nums = {numSpeed};
    radio = {speedMMM, speedUMS};
}

void RunWidget::createTextBoxes() {
    txtCode = new CodeEditor;
    txtCode->setReadOnly(true);

    int h = txtCode->fontMetrics().lineSpacing();
    h *= 5;
//    h += txtCode->fontMetrics().leading();

    QTextDocument* pdoc = txtCode->document();
    QMargins margins = txtCode->contentsMargins();
    h += static_cast<int>(pdoc->documentMargin()) + margins.top() + margins.bottom();

    txtCode->setFixedHeight(h);

    txtMsg = new QTextEdit;
    txtMsg->setReadOnly(true);

    txtMsg->setFixedHeight(h);

    gridText = new QGridLayout;
    gridText->addWidget(txtCode, 0, 0);
    gridText->addWidget(txtMsg, 0, 1);
}

void RunWidget::createState() {
#ifdef DARK_GUI_THEME
    const QString color = R"(orange)";
#else
    const QString color = R"(blue)";
#endif

    gridState = new QGridLayout;
    labelX = new QLabel(R"(<font color=)" + color + R"(>X</font>)");
    labelY = new QLabel(R"(<font color=)" + color + R"(>Y</font>)");
    labelU = new QLabel(R"(<font color=)" + color + R"(>U</font>)");
    labelV = new QLabel(R"(<font color=)" + color + R"(>V</font>)");

    labelTime = new QLabel(R"(<font color=)" + color + R"(>)" + tr("Time") + R"(</font>)");
    labelETA = new QLabel(R"(<font color=)" + color + R"(>)" + tr("Remain") + R"(</font>)");

    labelEncX = new QLabel(R"(<font color=)" + color + R"(>X</font>)");
    labelEncY = new QLabel(R"(<font color=)" + color + R"(>Y</font>)");

    posLabels = {new QLabel, new QLabel, new QLabel, new QLabel, new QLabel, new QLabel, new QLabel, new QLabel, labelX, labelY, labelU, labelV, labelTime, labelETA, labelEncX, labelEncY};

    for (size_t i = 0; i < posLabels.size(); i++) {
        if (i < CncParam::AXES_NUM)
            setPositionLabels(i, 0);
    }

    setElapsedTime(0, 0, 0);
    setRemainTime(0, 0, 0);

    setEncoderLabels(0, 0);
    setEncoderLabels(1, 0);

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

    //
    gridLS = new QGridLayout;

    gridLS->addWidget(labelPWR, 0, 0, Qt::AlignCenter);
    gridLS->addWidget(checkPWR, 1, 0, Qt::AlignCenter);

    gridLS->addWidget(labelWB, 0, 1, Qt::AlignCenter);
    gridLS->addWidget(checkWB, 1, 1, Qt::AlignCenter);

#ifndef STONE
    gridLS->addWidget(labelTO, 0, 2, Qt::AlignCenter);
    gridLS->addWidget(checkTO, 1, 2, Qt::AlignCenter);
#endif

    gridLS->addWidget(labelFB, 0, 3, Qt::AlignCenter);
    gridLS->addWidget(checkFB, 1, 3, Qt::AlignCenter);

    gridLS->addWidget(labelALM, 0, 4, Qt::AlignCenter);
    gridLS->addWidget(checkALM, 1, 4, Qt::AlignCenter);

    gridLS->addWidget(labelREV, 0, 5, Qt::AlignCenter);
    gridLS->addWidget(checkREV, 1, 5, Qt::AlignCenter);

    gridLS->addWidget(labelFWD, 0, 6, Qt::AlignCenter);
    gridLS->addWidget(checkFWD, 1, 6, Qt::AlignCenter);

    // ADC
    labelGap = new QLabel(tr("Discharge Gap") + ": ");
    labelRev = new QLabel(tr("Reverse Voltage") + ": ");
    labelWorkpiece = new QLabel(tr("Workpiece") + ": ");
    labelWire = new QLabel(tr("Wire") + ": ");
    labelHV = new QLabel(tr("High Voltage") + ": ");
    labelShunt = new QLabel(tr("Shunt") + ": ");
    adcLabels = {labelGap, labelRev, labelWorkpiece, labelWire, labelHV, labelShunt};
    adcValueLabels = {new QLabel, new QLabel, new QLabel, new QLabel, new QLabel, new QLabel};

    QGridLayout* gridAdc = new QGridLayout;

    gridAdc->addWidget(labelGap, 0, 0, Qt::AlignLeft);
    gridAdc->addWidget(adcValueLabels[0], 0, 1, Qt::AlignLeft);

#ifdef TEST_ADC
    gridAdc->addWidget(labelInv, 1, 0, Qt::AlignLeft);
    gridAdc->addWidget(adcValueLabels[1], 1, 1, Qt::AlignLeft);
#endif

    gridAdc->addWidget(labelWorkpiece, 2, 0, Qt::AlignLeft);
    gridAdc->addWidget(adcValueLabels[2], 2, 1, Qt::AlignLeft);

#ifdef TEST_ADC
    gridAdc->addWidget(labelWire, 3, 0, Qt::AlignLeft);
    gridAdc->addWidget(adcValueLabels[3], 3, 1, Qt::AlignLeft);

    gridAdc->addWidget(labelHV, 4, 0, Qt::AlignLeft);
    gridAdc->addWidget(adcValueLabels[4], 4, 1, Qt::AlignLeft);

    gridAdc->addWidget(labelShunt, 5, 0, Qt::AlignLeft);
    gridAdc->addWidget(adcValueLabels[5], 5, 1, Qt::AlignLeft);
#endif

    for (size_t i = 0; i < adcValueLabels.size(); i++)
        setAdc(i, 0);

    // Status Grid
    gridState->addWidget(labelX, 0, 0);
    gridState->addWidget(posLabels[0], 0, 1, Qt::AlignRight);

    gridState->addWidget(labelY, 1, 0);
    gridState->addWidget(posLabels[1], 1, 1, Qt::AlignRight);

#ifndef STONE
    gridState->addWidget(labelU, 2, 0);
    gridState->addWidget(posLabels[2], 2, 1, Qt::AlignRight);

    gridState->addWidget(labelV, 3, 0);
    gridState->addWidget(posLabels[3], 3, 1, Qt::AlignRight);
#endif

    gridState->addWidget(labelTime, 4, 0);
    gridState->addWidget(posLabels[4], 4, 1, Qt::AlignRight);

    gridState->addWidget(labelETA, 5, 0);
    gridState->addWidget(posLabels[5], 5, 1, Qt::AlignRight);

    gridState->addLayout(gridLS, 6, 0, 1, 2);

#ifndef STONE
    gridState->addLayout(gridAdc, 7, 0, 1, 2);
#endif

#ifdef SHOW_ENCODERS
    gridState->addWidget(new QFrame, 8, 0, 1, 2);

    labelEncHeader = new QLabel(tr("Encoders"));

    gridState->addWidget(labelEncHeader, 9, 0, 1, 2);

    gridState->addWidget(labelEncX, 10, 0);
    gridState->addWidget(posLabels[6], 10, 1, Qt::AlignRight);

    gridState->addWidget(labelEncY, 11, 0);
    gridState->addWidget(posLabels[7], 11, 1, Qt::AlignRight);
#endif

    gridState->setSizeConstraint(QLayout::SetFixedSize);
}

RunWidget::~RunWidget() {}

void RunWidget::setButtonsEnabled(bool run_ena, bool reverse_ena, bool cancel_ena, bool hold_ena) {
    btnStart->setEnabled(run_ena);
    btnReverse->setEnabled(reverse_ena);
    btnCancel->setEnabled(cancel_ena);
    btnHold->setEnabled(hold_ena);
}

void RunWidget::setButtonsText(const QString (&text)[3], const QString (&status)[3]) {
    if (text[0].length() != 0) {
        btnStart->setText(text[0]);
        btnStart->setStatusTip(status[0]);
    }
    if (text[1].length() != 0) {
        btnReverse->setText(text[1]);
        btnReverse->setStatusTip(status[1]);
    }
    if (text[2].length() != 0) {
        btnCancel->setText(text[2]);
        btnCancel->setStatusTip(status[2]);
    }
}

void RunWidget::setPositionLabels(size_t axis_num, int value) {
    if (axis_num < CncParam::AXES_NUM) {
        bool sign = value < 0;
        value = std::abs(value);

        double scale = 1;
        switch (axis_num) {
        case 0: scale = CncParam::scaleX; break;
        case 1: scale = CncParam::scaleY; break;
        case 2: scale = CncParam::scaleU; break;
        case 3: scale = CncParam::scaleV; break;
        }

        uint32_t value_um = static_cast<uint32_t>( std::round(double(value) / scale * 1e3) );
        QString s = QString::asprintf("%c%04d.%03d", sign ? '-' : ' ', value_um / 1000, value_um % 1000);

#if defined(STONE)
        if (axis_num < 2)
            posLabels[axis_num]->setText(R"(<font color=)" + posLabelColor + R"(>)" + s + R"(</font>)");
#else
        posLabels[axis_num]->setText(R"(<font color=)" + posLabelColor + R"(>)" + s + R"(</font>)");
#endif
    }
}

void RunWidget::setEncoderLabels(size_t axis_num, int value) {
    if (axis_num < CncParam::ENCODERS_NUM) {
        bool sign = value < 0;
        unsigned abs_value = std::abs(value);

        double scale = CncParam::DEFAULT_SCALE_ENC_XY;

        uint32_t value_um = static_cast<uint32_t>( std::round(double(abs_value) / scale * 1e3) );

        QString s = QString::asprintf("%c%04d.%03d", sign ? '-' : ' ', value_um / 1000, value_um % 1000);
//        QString s2 = QString::asprintf("%c%07d", sign ? '-' : ' ', abs_value);

        posLabels[6 + axis_num]->setText(R"(<font color=)" + posLabelColor + R"(>)" + s + R"(</font>)");
    }
}

void RunWidget::setElapsedTime(int h, int m, int s) {
    QString str = QString::asprintf("%02d:%02d:%02d", h, m, s);
    posLabels[4]->setText(R"(<font color=)" + posLabelColor + R"(>)" + str + R"(</font>)");
//    qDebug("%s\n", str.toStdString().c_str());
}

void RunWidget::setElapsedTime(qint64 ms) {
    using namespace auxItems;
    HMS hms(ms);
    setElapsedTime(int(hms.h), int(hms.m), int(hms.s));
//    qDebug("%d ms\n", int(ms));
}

void RunWidget::setRemainTime(int h, int m, int s) {
    QString str = QString::asprintf("%02d:%02d:%02d", h, m, s);
    posLabels[5]->setText(R"(<font color=)" + posLabelColor + R"(>)" + str + R"(</font>)");
}

void RunWidget::setRemainTime(qint64 ms) {
    using namespace auxItems;
    HMS hms(ms);
    setRemainTime(int(hms.h), int(hms.m), int(hms.s));
}

void RunWidget::setAdc(size_t adc_num, double value, bool mV_ena) {
    if (adc_num < adcValueLabels.size()) {
        if (value < 0)
            value = 0;

        if (mV_ena)
            value *= 1e3;

        int data = round(value);

        QString s;

        if (mV_ena)
            s = QString::asprintf("%d mV", data);
        else
            s = QString::asprintf("%d V", data);

        adcValueLabels[adc_num]->setText(s);
    }
}

void RunWidget::setAdc(size_t adc_num, double_valid_t value, bool mV_ena) {
//    if (!value.valid)
//        value.value = 0;

    setAdc(adc_num, value.value, mV_ena);
}

void RunWidget::setAdc(const cnc_adc_volt_t &adc) {
    setAdc(0, adc.gap);
    setAdc(1, adc.rev_gap);
    setAdc(2, adc.workpiece);
    setAdc(3, adc.wire);
    setAdc(4, adc.hv);
    setAdc(5, adc.shunt, true);
}

void RunWidget::setFontPointSize(QWidget* w, int pointSize) {
    QFont font = w->font();
    font.setPointSize(pointSize);
    w->setFont(font);
}

void RunWidget::setFontPointSize(int pointSize) {
    for (QPushButton* o: buttons) {
        QFont font = o->font();
        font.setPointSize(pointSize);
        o->setFont(font);
    //        o->setStyleSheet("font: bold");
    }

    for (QLabel* o: labels)
        setFontPointSize(o, pointSize);

    for (QLabel* o: checkLabels)
        setFontPointSize(o, pointSize);

    for (QLabel* o: adcLabels)
        setFontPointSize(o, pointSize);

    for (QLabel* o: adcValueLabels)
        setFontPointSize(o, pointSize);

    for (QDoubleSpinBox* o: nums) {
        QFont font = o->font();
        font.setPointSize(pointSize);
        o->setFont(font);
    }

    for (QRadioButton* o: radio) {
        QFont font = o->font();
        font.setPointSize(static_cast<int>( round(pointSize * 0.8) ));
        o->setFont(font);
    }

    for (size_t i = 0; i < posLabels.size(); i++) {
        QLabel*& label = posLabels[i];

        label->setSizePolicy( QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum) );
        QFont font = label->font();

//        if (i == 6 || i == 7 || i == posLabels.size() - 1 || i == posLabels.size() - 2)
//            font.setPointSize(pointSize * 1.2);
//        else
            font.setPointSize(pointSize * 2);

        font.setBold(true);
        label->setFont(font);
    }

#ifndef STONE
    {
        QFont font = labelEncHeader->font();
        font.setPointSize(pointSize * 1.2);
        font.setBold(true);
        labelEncHeader->setFont(font);
    }
#endif

    {
        QFont font = checkIdle->font();
        font.setPointSize(pointSize);
        checkIdle->setFont(font);
    }
}

void RunWidget::setLimitSwitches(CncLimitSwitches ls) {
    checkFWD->setChecked(ls.limsw_fwd);
    checkREV->setChecked(ls.limsw_rev);
    checkALM->setChecked(ls.limsw_alm);
    checkWB->setChecked(ls.wire_break);
    checkPWR->setChecked(ls.pwr);
    checkFB->setChecked(ls.fb);
    checkTO->setChecked(ls.fb_to);
}

void RunWidget::setLimitSwitches(bool fwd, bool rev, bool alarm, bool wire_break, bool pwr, bool feedback, bool fb_timeout) {
    CncLimitSwitches ls;

    ls.limsw_fwd = fwd;
    ls.limsw_rev = rev;
    ls.limsw_alm = alarm;
    ls.wire_break = wire_break;
    ls.pwr = pwr;
    ls.fb = feedback;
    ls.fb_to = fb_timeout;

    setLimitSwitches(ls);
}

//void RunWidget::setETA(int value) {
////        bool sign = value < 0;
////        value = std::abs(value);
////        uint32_t value_um = static_cast<uint32_t>( std::round(double(value) / CNC_SCALE * 1e3) );
////        QString s = QString::asprintf("%c%04d.%03d", sign ? '-' : ' ', value_um / 1000, value_um % 1000);
////        posLabels[axis_num]->setText(R"(<font color=)" + posLabelColor + R"(>)" + s + R"(</font>)");
//}

void RunWidget::plot(const ContourList& contourList, bool swapXY, bool inverseX, bool inverseY, bool showXY) {
    plotView.setSwapXY(swapXY);
    plotView.setInverseX(inverseX);
    plotView.setInverseY(inverseY);
    plotView.setShowXY(showXY);
    plotView.plot(contourList);
}

QSize RunWidget::plotViewSize() const {
    return plotViewSize(size());
}

QSize RunWidget::plotViewSize(const QSize& formSize) const {
    int codeHeight = gridText->sizeHint().height();
    int stateWidth = gridState->sizeHint().width();

    int w = formSize.width() - stateWidth;
    int h = formSize.height() - codeHeight;

    QSize res(w, h);

    qDebug() << "Widget size: " << size() << ", plot size:" << res << ", msg height: " << codeHeight << ", state width: " << stateWidth;

    return res;
}

void RunWidget::resizeEvent(QResizeEvent* event) {
    if (!plotView.onResizeEvent( plotViewSize(event->size()) ))
        event->ignore();
}
