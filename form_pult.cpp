#include "form_pult.h"
#include <QDebug>
#include <QTimer>

using namespace std;

FormPult::FormPult(ProgramParam& par, QWidget *parent) :
    QWidget(parent), par(par), m_cncReaderEna(false), m_adcEnable(false), cutStateAbortReq(false), wireSpeedMode(WireSpeed::Mode::MMM)
{
    this->setObjectName(tr("CNC Pult"));
    pultWidget = new PultWidget;
    createButtonLayout();

//    setFontPointSize(14);

    QLabel* labelTitle = new QLabel(R"(<h1>)" + tr("Pult") + R"(</h1>)");

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(labelTitle, 0, Qt::AlignCenter);
    mainLayout->addWidget(pultWidget);
    mainLayout->addLayout(gridButtons);

    this->setLayout(mainLayout);

    connect(pultWidget->btnFwd, &QPushButton::clicked, this, [&](bool) {
        this->on_btnMove_clicked();
        m_running_reg = false;
    });

    connect(pultWidget->btnRev, &QPushButton::clicked, this, [&](bool) {
        this->on_btnMove_clicked(true);
        m_running_reg = false;
    });

    connect(pultWidget->btnSet, &QPushButton::clicked, this, &FormPult::on_btnSet_clicked);
    connect(pultWidget->btnCancel, &QPushButton::clicked, this, &FormPult::on_btnCancel_clicked);

//    bindNumberButtons(numWidth, btnWidthInc, btnWidthDec);

//    connect(sender, &Sender::valueChanged,
//        [=]( const QString &newValue ) { receiver->updateValue( "senderValue", newValue ); }
//    );
}

FormPult::~FormPult() {
    stopCncReader();
    stopAdc();
}

void FormPult::setFontPointSize(int pointSize) {
    pultWidget->setFontPointSize(pointSize);

    for (QPushButton* b: buttons) {
        QFont font = b->font();
        font.setPointSize(pointSize);
        b->setFont(font);
    }

    for (QSpinBox* const numBox: nums) {
        QFont font = numBox->font();
        font.setPointSize(pointSize);
        numBox->setFont(font);
    }

    for (QLabel* const label: labels) {
        QFont font = label->font();
        font.setPointSize(pointSize);
        label->setFont(font);
    }
}

void FormPult::addButtons() {
    btnHome = new QPushButton(tr("Home"));
    btnHome->setStatusTip(tr("Go to the Home panel") + "   Alt+H");
    btnHome->setShortcut(QKeySequence("Alt+H"));

    btnHelp = new QPushButton(tr("Help"));
    btnHelp->setStatusTip(tr("Open Help") + "   F1");
    btnHelp->setShortcut(QKeySequence::HelpContents);

    btnBreak = new QPushButton(tr("Break"));
    btnBreak->setCheckable(true);
    btnBreak->setStatusTip(tr("Wire break control on/off"));

    btnPump = new QPushButton(tr("Pump"));
    btnPump->setCheckable(true);
    btnPump->setStatusTip(tr("Pump on/off"));

    btnRoll = new QPushButton(tr("Roll"));
    btnRoll->setCheckable(true);
    btnRoll->setStatusTip(tr("Wire roll on/off"));

    btnRollVelDec = new QPushButton(tr("DEC"));
    btnRollVelDec->setStatusTip(tr("Roll velocity decrement"));

    btnRollVelInc = new QPushButton(tr("INC"));
    btnRollVelInc->setStatusTip(tr("Roll velocity increment"));

    btnHighVolt = new QPushButton;
    btnHighVolt->setCheckable(true);
#ifndef STONE    
    btnHighVolt->setText(tr("High Voltage"));
    btnHighVolt->setStatusTip(tr("Enable High Voltage"));
#else
    btnHighVolt->setEnabled(false);
#endif

#ifndef STONE
    btnLowHighVolt = new QPushButton;
    btnLowHighVolt->setText(tr("Low High Volt."));
    btnLowHighVolt->setCheckable(true);
    btnLowHighVolt->setStatusTip(tr("Enable Low High Voltage"));
#endif

    btnCurrentDec = new QPushButton;
#ifndef STONE
    btnCurrentDec->setText(tr("DEC"));
#else
    btnCurrentDec->setEnabled(false);
#endif

    btnCurrentInc = new QPushButton;
#ifndef STONE
    btnCurrentInc->setText(tr("INC"));
#else
    btnCurrentInc->setEnabled(false);
#endif

    btnWidthDec = new QPushButton;
#ifndef STONE
    btnWidthDec->setText(tr("DEC"));
#else
    btnWidthDec->setEnabled(false);
#endif

    btnWidthInc = new QPushButton;
#ifndef STONE
    btnWidthInc->setText(tr("INC"));
#else
    btnWidthInc->setEnabled(false);
#endif

    btnRatioDec = new QPushButton;
#ifndef STONE
    btnRatioDec->setText(tr("DEC"));
#else
    btnRatioDec->setEnabled(false);
#endif

    btnRatioInc = new QPushButton;
#ifndef STONE
    btnRatioInc->setText(tr("INC"));
#else
    btnRatioInc->setEnabled(false);
#endif

    buttons = {
        btnHome, btnHelp, btnBreak, btnPump, btnRoll, btnRollVelDec, btnRollVelInc,
        btnHighVolt, btnCurrentDec, btnCurrentInc, btnWidthDec, btnWidthInc, btnRatioInc, btnRatioDec
#ifndef STONE
        , btnLowHighVolt
#endif
    };

#ifdef STONE
    gridButtons->addWidget(btnHome, 1, 0);
    gridButtons->addWidget(btnBreak, 1, 1);
    gridButtons->addWidget(btnPump, 1, 2);
    gridButtons->addWidget(btnRoll, 1, 3);
    gridButtons->addWidget(btnRollVelDec, 1, 4);
    gridButtons->addWidget(btnRollVelInc, 1, 5);
    gridButtons->addWidget(btnHighVolt, 1, 6);
    gridButtons->addWidget(btnCurrentDec, 1, 7);
    gridButtons->addWidget(btnCurrentInc, 1, 8);
    gridButtons->addWidget(btnWidthDec, 1, 9);
    gridButtons->addWidget(btnWidthInc, 1, 10);
    gridButtons->addWidget(btnRatioDec, 1, 11);
    gridButtons->addWidget(btnRatioInc, 1, 12);
#else
    gridButtons->addWidget(btnHome, 0, 0);

    gridButtons->addWidget(btnBreak, 1, 0);
    gridButtons->addWidget(btnPump, 1, 1);
    gridButtons->addWidget(btnRoll, 1, 2);
    gridButtons->addWidget(btnRollVelDec, 1, 3);
    gridButtons->addWidget(btnRollVelInc, 1, 4);
    gridButtons->addWidget(btnHighVolt, 1, 5);
    gridButtons->addWidget(btnCurrentDec, 1, 6);
    gridButtons->addWidget(btnCurrentInc, 1, 7);
    gridButtons->addWidget(btnWidthDec, 1, 8);
    gridButtons->addWidget(btnWidthInc, 1, 9);
    gridButtons->addWidget(btnRatioDec, 1, 10);
    gridButtons->addWidget(btnRatioInc, 1, 11);
    gridButtons->addWidget(btnLowHighVolt, 1, 12);
#endif
    gridButtons->addWidget(btnHelp, 1, 13);

    connect(btnHome, &QPushButton::clicked, this, &FormPult::on_btnHome_clicked);
    connect(btnHelp, &QPushButton::clicked, this, [&]() { emit helpPageClicked(help_file); });

    connect(btnRoll, &QPushButton::clicked, this, [&]() {
        try {
            par.cnc.writeRollEnable( btnRoll->isChecked() );
        } catch (...) {}
    });

    connect(btnPump, &QPushButton::clicked, this, [&]() {
        try {
            par.cnc.writePumpEnable( btnPump->isChecked() );
        } catch (...) {}
    });

    connect(btnBreak, &QPushButton::clicked, this, [&]() {
        try {
            par.cnc.writeWireEnable( btnBreak->isChecked() );
        } catch (...) {}
    });

    connect(btnHighVolt, &QPushButton::clicked, this, [&]() {
        try {
            par.cnc.writeEnableHighVoltage( btnHighVolt->isChecked() );
        } catch (...) {}
    });

    connect(pultWidget->btnHold, &QPushButton::clicked, this, [&]() {
        try {
            par.cnc.writeHoldEnable( pultWidget->btnHold->isChecked() );
        } catch (...) {}
    });

    connect(btnRollVelInc, &QPushButton::clicked, [&]() { numRollVel->stepUp(); });
    connect(btnRollVelDec, &QPushButton::clicked, [&]() { numRollVel->stepDown(); });
    connect(btnCurrentInc, &QPushButton::clicked, [&]() { numCurrent->stepUp(); });
    connect(btnCurrentDec, &QPushButton::clicked, [&]() { numCurrent->stepDown(); });
    connect(btnWidthInc, &QPushButton::clicked, [&]() { numWidth->stepUp(); });
    connect(btnWidthDec, &QPushButton::clicked, [&]() { numWidth->stepDown(); });
    connect(btnRatioInc, &QPushButton::clicked, [&]() { numRatio->stepUp(); });
    connect(btnRatioDec, &QPushButton::clicked, [&]() { numRatio->stepDown(); });

#ifndef STONE
    connect(btnLowHighVolt, &QPushButton::clicked, [&]() {
        par.cnc.writeEnableLowHighVolt( btnLowHighVolt->isChecked() );
    });
#endif
}

void FormPult::createButtonLayout() {
    numRollVel = new QSpinBox;
    numRollVel->setRange(1, 7);
    numWidth = new QSpinBox;
    numWidth->setRange(2, 99);
    numRatio = new QSpinBox;
    numRatio->setRange(2, 32);
    numCurrent = new QSpinBox;
    numCurrent->setRange(0, 11);

    nums = {numRollVel, numCurrent, numWidth, numRatio};

    labelRoll = new QLabel(tr("Roll Velocity") + ": ");
    labelRoll->setBuddy(numRollVel);
    labelWidth = new QLabel(tr("Width") + ": ");
    labelWidth->setBuddy(numWidth);
    labelRatio = new QLabel(tr("Ratio") + ": ");
    labelRatio->setBuddy(numRatio);
    labelCurrent = new QLabel(tr("Current") + ": ");
    labelCurrent->setBuddy(numCurrent);

    labels = {labelRoll, labelWidth, labelRatio, labelCurrent};

    groupRoll = new QGroupBox;
    groupWidth = new QGroupBox;
    groupRatio = new QGroupBox;
    groupCurrent = new QGroupBox;

    groupRoll->setLayout(new QHBoxLayout);
    groupRoll->layout()->addWidget(labelRoll);
    groupRoll->layout()->addWidget(numRollVel);

    groupWidth->setLayout(new QHBoxLayout);
    groupWidth->layout()->addWidget(labelWidth);
    groupWidth->layout()->addWidget(numWidth);

    groupRatio->setLayout(new QHBoxLayout);
    groupRatio->layout()->addWidget(labelRatio);
    groupRatio->layout()->addWidget(numRatio);

    groupCurrent->setLayout(new QHBoxLayout);
    groupCurrent->layout()->addWidget(labelCurrent);
    groupCurrent->layout()->addWidget(numCurrent);

    gridButtons = new QGridLayout;

#ifdef STONE
    gridButtons->addWidget(groupRoll, 0, 4, 1, 2);
#else
    gridButtons->addWidget(groupRoll, 0, 3, 1, 2);

    gridButtons->addWidget(groupCurrent, 0, 6, 1, 2);
    gridButtons->addWidget(groupWidth, 0, 8, 1, 2);
    gridButtons->addWidget(groupRatio, 0, 10, 1, 2);
#endif

    connect(numRollVel, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int x) { par.cnc.writeRollVel(static_cast<unsigned>(x)); });
    connect(numWidth,   QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int x) { par.cnc.writePulseWidth(static_cast<unsigned>(x)); });
    connect(numRatio,   QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int x) { par.cnc.writePulseRatio(static_cast<unsigned>(x)); });
    connect(numCurrent, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int x) { par.cnc.writeCurrentIndex(static_cast<unsigned>(x)); });

    //
    WireSpeed speed;
    pultWidget->numSpeed->setDecimals(2);
    pultWidget->numSpeed->setSingleStep(0.01);
    pultWidget->numSpeed->setRange(speed.min(), speed.max());
    pultWidget->numSpeed->setValue(speed.get());

    // don't work, because speed was written into FPGA
//    connect(pultWidget->numSpeed, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double x) {
//        WireSpeed speed(x, wireSpeedMode);
//        par.cnc.writeSpeed(speed);
//    });

    connect(pultWidget->speedMMM, &QRadioButton::clicked, this, [&]() {
        if (wireSpeedMode != WireSpeed::Mode::MMM) {
            WireSpeed speed(pultWidget->numSpeed->value(), wireSpeedMode);
            wireSpeedMode = WireSpeed::Mode::MMM;
            speed.changeMode(wireSpeedMode);
            pultWidget->numSpeed->setDecimals(2);
            pultWidget->numSpeed->setSingleStep(0.01);
            pultWidget->numSpeed->setRange(speed.min(), speed.max());
            pultWidget->numSpeed->setValue(speed.get());
        }
    });
    connect(pultWidget->speedUMS, &QRadioButton::clicked, this, [&]() {
        if (wireSpeedMode != WireSpeed::Mode::UMS) {
            WireSpeed speed(pultWidget->numSpeed->value(), wireSpeedMode);
            wireSpeedMode = WireSpeed::Mode::UMS;
            speed.changeMode(wireSpeedMode);
            pultWidget->numSpeed->setDecimals(1);
            pultWidget->numSpeed->setSingleStep(0.1);
            pultWidget->numSpeed->setRange(speed.min(), speed.max());
            pultWidget->numSpeed->setValue(speed.get());            
        }
    });

    addButtons();
}

void FormPult::init() {
    _init();

    if (!m_cncReaderEna)
        startCncReader();

#ifndef STONE
    startAdc();
#endif
}

void FormPult::_init() {
    controlsEnable(true, true);
    m_running_reg = false;

//    if (!par.cnc.isOpen())
//        pultWidget->txtMsg->insertPlainText("Error: No CNC connection\n");
}

void FormPult::on_btnHome_clicked() {
    stopCncReader();
    stopAdc();
    emit homePageClicked();
}

void FormPult::on_btnMove_clicked(bool dir) {
    int32_t nx = pultWidget->getMoveN(0);
    int32_t ny = pultWidget->getMoveN(1);
    int32_t nu = pultWidget->getMoveN(2);
    int32_t nv = pultWidget->getMoveN(3);
    if (dir) {
        nx = -nx;
        ny = -ny;
        nu = -nu;
        nv = -nv;
    }

    try {
        par.cnc.writeHoldEnable( true );
        par.cnc.directMoveOn(nx, pultWidget->scale(0), ny, pultWidget->scale(1), nu, pultWidget->scale(2), nv, pultWidget->scale(3), pultWidget->speed());
    } catch (...) {}
}

void FormPult::on_btnSet_clicked() {
    int32_t nx = pultWidget->getSetN(0);
    int32_t ny = pultWidget->getSetN(1);
    int32_t nu = pultWidget->getSetN(2);
    int32_t nv = pultWidget->getSetN(3);
    int32_t enc_x = pultWidget->getSetEncN(0);
    int32_t enc_y = pultWidget->getSetEncN(1);
    m_running_reg = false;

    try {
        par.cnc.directSetPos(nx, ny, nu, nv, enc_x, enc_y);
    } catch (...) {}
}

void FormPult::on_btnCancel_clicked() {
    m_running_reg = false;

    try {
        par.cnc.cancelReq();
    } catch (...) {}
}

void FormPult::startCncReader() {
    if (par.cnc.isOpen()) {
        qDebug()<<"CNC reader: Start";
        m_cncReaderEna = true;
        readCutState();
    }
}

void FormPult::stopCncReader() {
    qDebug()<<"CNC reader: Stop";
    m_cncReaderEna = false;
}

void FormPult::blockSignals(bool value) {
    btnRoll->blockSignals(value);
    btnPump->blockSignals(value);
    btnBreak->blockSignals(value);
    btnHighVolt->blockSignals(value);
#ifndef STONE
    btnLowHighVolt->blockSignals(value);
#endif

    numRollVel->blockSignals(value);
    numWidth->blockSignals(value);
    numRatio->blockSignals(value);
    numCurrent->blockSignals(value);

    pultWidget->btnHold->blockSignals(value);
    pultWidget->btnCancel->blockSignals(value);
}

void FormPult::readCutState() {
    if (m_cncReaderEna) {
//        qDebug()<<"CNC reader: Reading...";

        blockSignals(true);
        try {
            CncContext ctx = par.cnc.readCncContext();
//            qDebug() << "CNC reader: " + QString(cut_state.toString().c_str());

            pultWidget->setMotorView(0, ctx.x());
            pultWidget->setMotorView(1, ctx.y());
            pultWidget->setMotorView(2, ctx.u());
            pultWidget->setMotorView(3, ctx.v());
            pultWidget->setEncoderView(0, ctx.encoderX());
            pultWidget->setEncoderView(1, ctx.encoderY());

            pultWidget->setLimitSwitches(ctx.limitSwitches());

            btnRoll->setChecked(ctx.isRollEnable());
            btnPump->setChecked(ctx.pumpEnabled());
            btnBreak->setChecked(ctx.wireControlEnabled());
            btnHighVolt->setChecked(ctx.highVoltageEnabled());

            pultWidget->btnHold->setChecked(ctx.hold());

            if (ctx.rollVelocity() < numRollVel->minimum()) {
                numRollVel->setValue(numRollVel->minimum());
//                qDebug("Error: MIN CNC Roll Vel: %x", static_cast<unsigned>(ctx.rollVelocity()));
            }
            else if (ctx.rollVelocity() > numRollVel->maximum()) {
                numRollVel->setValue(numRollVel->maximum());
                qDebug("Error: MAX CNC Roll Vel: %x", static_cast<unsigned>(ctx.rollVelocity()));
            }
            else
                numRollVel->setValue(ctx.rollVelocity());

            if (ctx.pulseWidth() < numWidth->minimum()) {
                numWidth->setValue(numWidth->minimum());
                qDebug("Error: MIN CNC Pulse Width: %x", static_cast<unsigned>(ctx.pulseWidth()));
            }
            else if (ctx.pulseWidth() > numWidth->maximum()) {
                numWidth->setValue(numWidth->maximum());
                qDebug("Error: MAX CNC Pulse Width: %x", static_cast<unsigned>(ctx.pulseWidth()));
            }
            else
                numWidth->setValue(ctx.pulseWidth());

            if (ctx.pulseRatio() < numRatio->minimum()) {
                numRatio->setValue(numRatio->minimum());
                qDebug("Error: MIN CNC Pulse Ratio: %x", static_cast<unsigned>(ctx.pulseRatio()));
            }
            else if (ctx.pulseRatio() > numRatio->maximum()) {
                numRatio->setValue(numRatio->maximum());
                qDebug("Error: MAX CNC Pulse Ratio: %x", static_cast<unsigned>(ctx.pulseRatio()));
            }
            else
                numRatio->setValue(ctx.pulseRatio());

#ifndef STONE
            btnLowHighVolt->setChecked( ctx.lowHighVoltageEnabled() );
#endif

            if (ctx.currentIndex() < numCurrent->minimum()) {
                numCurrent->setValue(numCurrent->minimum());
                qDebug("Error: MIN CNC Pulse Ratio: %x", static_cast<unsigned>(ctx.currentIndex()));
            }
            else if (ctx.currentIndex() > numCurrent->maximum()) {
                numCurrent->setValue(numCurrent->maximum());
                qDebug("Error: MAX CNC Pulse Ratio: %x", static_cast<unsigned>(ctx.currentIndex()));
            }
            else
                numCurrent->setValue(ctx.currentIndex());

            WireSpeed speed( ctx.speed() );
            speed.changeMode(wireSpeedMode);

//            if (speed.get() < speed.min()) {
//                pultWidget->numSpeed->setValue(speed.min());
//                qDebug("Error: MIN CNC Speed: %f", static_cast<double>(ctx.speed()));
//            }
//            else if (speed.get() > speed.max()) {
//                pultWidget->numSpeed->setValue(speed.max());
//                qDebug("Error: MAX CNC Speed: %f", static_cast<double>(ctx.speed()));
//            }
//            else
//                pultWidget->numSpeed->setValue(speed.get());

//            if (par.appState.isWork() && cut_state.isWork()) {
//                onFrameChanged(cut_state.frame_num,
//                               fpoint_t(cut_state.pos.x / X_SCALE, cut_state.pos.y / Y_SCALE),
//                               fpoint_t(cut_state.pos.u / U_SCALE, cut_state.pos.v /  V_SCALE)
//                               );
//            }

            bool m_running = ctx.isWork();

            if (m_running_reg && !m_running)
                pultWidget->btnCancel->click();

            m_running_reg = m_running;

            if (ctx.isWork()) {
                controlsEnable(false);
            } else if (ctx.isError()) {
                par.cnc.cancelReq();
                controlsEnable(true);
                m_running_reg = false;
            } else {
                controlsEnable(true);
                m_running_reg = false;
            }
        } catch (...) {
            controlsEnable(true, true);
            m_running_reg = false;
        }

        blockSignals(false);

        QTimer::singleShot(POLLING_TIME, this, &FormPult::readCutState);
    }
}

void FormPult::controlsEnable(bool ena, bool force) {
    if (force || btnHome->isEnabled() != ena) {
        pultWidget->controlsEnable(ena);
        btnHome->setEnabled(ena);
    }
}

void FormPult::startAdc() { // on init
    if (par.cnc.isOpen()) {
        qDebug()<<"ADC: Start";
        m_adcEnable = true;
        readAdc();
    }
    else
        m_adcEnable = false;
}

void FormPult::stopAdc() { // at to home
    qDebug()<<"ADC: Stop";
    m_adcEnable = false;
}

void FormPult::readAdc() {
    if (m_adcEnable) {
        cnc_adc_volt_t adc = par.cnc.readADCVolt();
        pultWidget->setAdc(adc);
        QTimer::singleShot(ADC_POLLING_TIME, this, &FormPult::readAdc);
    }
}
