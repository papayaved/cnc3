#include "form_run.h"
#include "aux_items.h"
#include <QDebug>
#include <QTimer>
#include <QMessageBox>

using namespace std;

FormRun::FormRun(ProgramParam& par, QWidget *parent) :
    QWidget(parent), par(par), cncReaderEna(false), adcEnable(false), currentCursorPosition(0), cutStateAbortReq(false), m_speed(WireSpeed()), remain_tmr(0)
{
    this->setObjectName(tr("Work Panel"));
    runWidget = new RunWidget(this);
    createSpinBoxes(); // row 0
    addButtons(); // row 1

//    setFontPointSize(14);

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(runWidget);
    mainLayout->addLayout(gridButtons);

    this->setLayout(mainLayout);

    timer = new StartStopElapsedTimer;

    connect(runWidget->txtCode, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    highlightCurrentLine();

    m_report.bind(runWidget->txtMsg);

    connect(runWidget->btnStart, &QPushButton::clicked, this, &FormRun::on_btnStart_clicked);
    connect(runWidget->btnReverse, &QPushButton::clicked, this, &FormRun::on_btnReverse_clicked);
    connect(runWidget->btnCancel, &QPushButton::clicked, this, &FormRun::on_btnCancel_clicked);
    connect(runWidget->checkIdle, &QCheckBox::clicked, this, [&](bool checked) {
        par.appState.setIdleRun(checked);
    });

//    bindNumberButtons(numWidth, btnWidthInc, btnWidthDec);

//    connect(sender, &Sender::valueChanged,
//        [=]( const QString &newValue ) { receiver->updateValue( "senderValue", newValue ); }
//    );
}

FormRun::~FormRun() {
    stopCncReader();
    if (timer) {
        delete timer;
        timer = nullptr;
    }
}

void FormRun::setFontPointSize(int pointSize) {
    runWidget->setFontPointSize(pointSize);

    for (QPushButton* b: buttons) {
        if (!b) continue;

        QFont font = b->font();
        font.setPointSize(pointSize);
        b->setFont(font);
//        b->setStyleSheet("font: bold");
    }

    for (QSpinBox* const numBox: numBoxes) {
        if (!numBox) continue;

//        numBox->setSizePolicy( QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum) );
        QFont font = numBox->font();
        font.setPointSize(pointSize);
//        font.setBold(true);
        numBox->setFont(font);
    }

    vector<QLabel*> labels = {labelRoll, labelWidth, labelRatio, labelCurrent};
    for (QLabel* const label: labels) {
        if (!label) continue;

//        numBox->setSizePolicy( QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum) );
        QFont font = label->font();
        font.setPointSize(pointSize);
//        font.setBold(true);
        label->setFont(font);
    }
}

void FormRun::addButtons() {
    btnHome = new QPushButton(tr("Home"));
    btnHome->setStatusTip(tr("Go to the Home panel") + "   Alt+H");
    btnHome->setShortcut(QKeySequence("Alt+H"));

    btnHelp = new QPushButton(tr("Help"));
    btnHelp->setStatusTip(tr("Open Help") + "   F1");
    btnHelp->setShortcut(QKeySequence::HelpContents);

    btnBreak = new QPushButton(tr("Break"));
    btnBreak->setCheckable(true);
    btnBreak->setStatusTip(tr("Wire break control on/off"));
    QPalette palButton = btnBreak->palette();
    palButton.setColor(QPalette::Disabled, QPalette::Button, Qt::darkGreen); //...
    btnBreak->setPalette(palButton);

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
#ifndef STONE
    btnHighVolt->setCheckable(true);
    btnHighVolt->setText(tr("High Voltage"));
    btnHighVolt->setStatusTip(tr("Enable High Voltage"));
#else
    btnHighVolt->setEnabled(false);
#endif

    btnLowHighVolt = new QPushButton;
#ifndef STONE
    btnLowHighVolt->setCheckable(true);
    btnLowHighVolt->setText(tr("Low High Volt."));
    btnLowHighVolt->setStatusTip(tr("Enable Low High Voltage"));
#else
    btnLowHighVolt->setEnabled(false);
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
        btnBreak, btnPump, btnRoll, btnRollVelDec, btnRollVelInc,\
        btnHighVolt, btnCurrentDec, btnCurrentInc, btnWidthDec, btnWidthInc, btnRatioInc, btnRatioDec,\
        btnHome, btnHelp,
        btnLowHighVolt
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

    connect(btnHome, &QPushButton::clicked, this, &FormRun::on_btnHome_clicked);
    connect(btnHelp, &QPushButton::clicked, this, [&]() { emit helpPageClicked(help_file); });

    connect(btnRoll, &QPushButton::clicked, this, [&]() {
        blockSignals(true);
        try {            
            par.cnc.writeRollEnable( btnRoll->isChecked() );
        }
        catch (...) {}
        blockSignals(false);
    });

    connect(btnPump, &QPushButton::clicked, this, [&]() {
        blockSignals(true);
        try {
            par.cnc.writePumpEnable( btnPump->isChecked() );            
        }
        catch (...) {}
        blockSignals(false);
    });

    connect(btnBreak, &QPushButton::clicked, this, [&]() {
        blockSignals(true);
        try {            
            par.cnc.writeWireEnable( btnBreak->isChecked() );            
        }
        catch (...) {}
        blockSignals(false);
    });

    connect(btnHighVolt, &QPushButton::clicked, this, [&]() {
        blockSignals(true);
        try {
            par.cnc.writeEnableHighVoltage( btnHighVolt->isChecked() );
        }
        catch (...) {}
        blockSignals(false);
    });

#ifndef STONE
    connect(btnLowHighVolt, &QPushButton::clicked, this, [&]() {
        blockSignals(true);
        try {            
            par.cnc.writeEnableLowHighVolt( btnLowHighVolt->isChecked() );
        }
        catch (...) {}
        blockSignals(false);
    });
#endif

    connect(runWidget->btnHold, &QPushButton::clicked, this, [&]() {
        blockSignals(true);
        try {
            par.cnc.writeHoldEnable( runWidget->btnHold->isChecked() );
        }
        catch (...) {}
        blockSignals(false);
    });

    connect(btnRollVelInc, &QPushButton::clicked, this, [&]() { numRollVel->stepUp(); });
    connect(btnRollVelDec, &QPushButton::clicked, this, [&]() { numRollVel->stepDown(); });
    connect(btnWidthInc, &QPushButton::clicked, this, [&]() { numWidth->stepUp(); });
    connect(btnWidthDec, &QPushButton::clicked, this, [&]() { numWidth->stepDown(); });
    connect(btnRatioInc, &QPushButton::clicked, this, [&]() { numRatio->stepUp(); });
    connect(btnRatioDec, &QPushButton::clicked, this, [&]() { numRatio->stepDown(); });
    connect(btnCurrentInc, &QPushButton::clicked, this, [&]() { numCurrent->stepUp(); });
    connect(btnCurrentDec, &QPushButton::clicked, this, [&]() { numCurrent->stepDown(); });
}

void FormRun::createSpinBoxes() {
    numRollVel = new QSpinBox;
    numRollVel->setRange(cnc_param::ROLL_MIN, cnc_param::ROLL_MAX);

    numCurrent = new QSpinBox;
    numCurrent->setRange(cnc_param::CURRENT_MIN, cnc_param::CURRENT_MAX);

    numWidth = new QSpinBox;
    numWidth->setRange(cnc_param::PULSE_WIDTH_MIN, cnc_param::PULSE_WIDTH_MAX);

    numRatio = new QSpinBox;
    numRatio->setRange(cnc_param::PULSE_RATIO_MIN, cnc_param::PULSE_RATIO_MAX);

    numBoxes = {numRollVel, numWidth, numRatio, numCurrent};

    labelRoll = new QLabel(tr("Roll Velocity") + ": ");
    labelRoll->setBuddy(numRollVel);
    labelWidth = new QLabel(tr("Width") + ": ");
    labelWidth->setBuddy(numWidth);
    labelRatio = new QLabel(tr("Ratio") + ": ");
    labelRatio->setBuddy(numRatio);
    labelCurrent = new QLabel(tr("Current") + ": ");
    labelCurrent->setBuddy(numCurrent);

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

    connect(numRollVel, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int x) {
        blockSignals(true);
        try {
            par.cnc.writeRollVel(static_cast<unsigned>(x));            
        } catch (...) {}
        blockSignals(false);
    });

    connect(numWidth, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int x) {
        blockSignals(true);
        try {
            par.cnc.writePulseWidth(static_cast<unsigned>(x));
        } catch (...) {}
        blockSignals(false);
    });

    connect(numRatio, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int x) {
        blockSignals(true);
        try {
            par.cnc.writePulseRatio(static_cast<unsigned>(x));
        } catch (...) {}
        blockSignals(false);
    });

    connect(numCurrent, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int x) {
        blockSignals(true);
        try {
            par.cnc.writeCurrentIndex(static_cast<unsigned>(x));
        } catch (...) {}
        blockSignals(false);
    });

    //
    runWidget->numSpeed->setRange(m_speed.min(), m_speed.max());
    runWidget->numSpeed->setValue(m_speed.get());
    runWidget->numSpeed->setDecimals(2);
    runWidget->numSpeed->setSingleStep(0.01);

    connect(runWidget->numSpeed, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=](double x) {
        blockSignals(true);

        m_speed.set(x);
        par.cnc.writeSpeed(m_speed);

        blockSignals(false);
    });

//    connect(runWidget->btnSpeed, &QPushButton::clicked, this, [&]() {
//        m_speed.set(runWidget->numSpeed->value());
//        par.cnc.writeSpeed(m_speed);
//    });

    connect(runWidget->speedMMM, &QRadioButton::clicked, this, [&]() {
        blockSignals(true);

        if (m_speed.mode() != WireSpeed::Mode::MMM) {
            m_speed.changeMode(WireSpeed::Mode::MMM);            

            unsigned dec = 2;
//            runWidget->numSpeed->setRange(DBL_MIN, DBL_MAX);
            runWidget->numSpeed->setDecimals(dec);
            runWidget->numSpeed->setSingleStep( pow(10, -dec) );
            runWidget->numSpeed->setRange(m_speed.min(), m_speed.max());

            double p = pow(10, dec);
            runWidget->numSpeed->setValue( round(m_speed.get() * p) / p );
        }

        blockSignals(false);
    });

    connect(runWidget->speedUMS, &QRadioButton::clicked, this, [&]() {
        blockSignals(true);

        if (m_speed.mode() != WireSpeed::Mode::UMS) {
            m_speed.changeMode(WireSpeed::Mode::UMS);            

            unsigned dec = 1;
//            runWidget->numSpeed->setRange(DBL_MIN, DBL_MAX);
            runWidget->numSpeed->setDecimals(dec);
            runWidget->numSpeed->setSingleStep( pow(10, -dec) );
            runWidget->numSpeed->setRange(m_speed.min(), m_speed.max());

            double p = pow(10, dec);
            runWidget->numSpeed->setValue( round(m_speed.get() * p) / p );
        }

        blockSignals(false);
    });
}

void FormRun::blockSignals(bool value) {
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

    runWidget->btnHold->blockSignals(value);
    runWidget->btnStart->blockSignals(value);
    runWidget->btnReverse->blockSignals(value);
    runWidget->btnCancel->blockSignals(value);

    runWidget->numSpeed->blockSignals(value);
}

using namespace auxItems;

bool FormRun::loadGcode() {
    size_t wrsize, pa_size;
    m_info.clear();

    QElapsedTimer t;
    t.start();    

    bool OK = par.cnc.write(m_gframes, &wrsize, &pa_size);

    if (!OK) {
        m_info +=   tr("G-code size is too big") + "\n";
        m_info +=   tr("G-code size") + ": " + QString::number(wrsize) + " " + tr("bytes") + ". " +
                    tr("CNC program array size") + ": " + QString::number(pa_size) + " " + tr("bytes");

        m_report.write(m_info);
        m_info += "\n";

        QMessageBox::critical(
            this,
            tr("G-code size is too big"),
            tr("G-code size") + ": " + QString::number(wrsize) + " " + tr("bytes") + ". " + tr("CNC program array size") + ": " + QString::number(pa_size) + " " + tr("bytes")
        );

        return false;
    }

    m_info +=   tr("G-code size") + ": " + QString::number(wrsize) + " " + tr("bytes") + ". " +
                tr("CNC program array size") + ": " + QString::number(pa_size) + " " + tr("bytes");

    m_report.write(m_info);
    m_info += "\n";

    qDebug("loadGcode elapsed 1: %d ms", (int)t.elapsed());

    t.restart();
    list<string> read_gframes = par.cnc.read();
    qDebug("loadGcode elapsed 2: %d ms", (int)t.elapsed());

    t.restart();
    string error_text;
    OK = GCode::compareFrames(m_gframes, read_gframes, error_text);
    qDebug("loadGcode elapsed 3: %d ms", (int)t.elapsed());

    if (OK)
        qDebug("form_run::compareFrames>>OK");
    else {
        qDebug( "form_run::compareFrames>>Error: %s", error_text.c_str());
//        runWidget->txtMsg->append(QString(error_text.c_str()));

        QMessageBox::critical(
            this,
            tr("Read G-code error"),
            error_text.c_str()
        );

        return false;
    }

    t.restart();
    string txt = GCode::toText(read_gframes, true);
    qDebug("loadGcode elapsed 4: %d ms", (int)t.elapsed());

    t.restart();
    qDebug() << txt.c_str();
    qDebug("loadGcode elapsed 5: %d ms", (int)t.elapsed());

    return true;
}

void FormRun::init(bool recovery) {
    m_report.clear();
    par.cnc.bindReporter(&m_report);

    _init();

    try {
        par.cnc.stateClear();

        // check G-code size
        m_gframes = par.gcode.toFrameList();
        size_t gsize = sizeOf(m_gframes);

        uint32_t pa_size = par.cnc.readProgArraySize();

        QString txt =   tr("G-code size") + ": " + QString::number(gsize) + " " + tr("bytes") + ". " +
                        tr("CNC program array size") + ": " + QString::number(pa_size) + " " + tr("bytes");

        m_report.write(txt);
        m_info += txt + "\n";

        if (gsize > pa_size) {
            QMessageBox::critical(
                this,
                tr("G-code size is too big"),
                tr("G-code size") + ": " + QString::number(gsize) + " " + tr("bytes") + ". " + tr("CNC program array size") + ": " + QString::number(pa_size) + " " + tr("bytes")
            );
        }

        if (recovery) {
            if (par.cncContext.valid()) {
                par.cnc.reset(); // includes Input Levels
#if defined(STONE)
                par.cnc.writeSemaphoreCncEnable(true);
#else
                par.cnc.writeCncEnable(true);
#endif
                m_gframes = par.gcode.toFrameList();

                m_report.clear();
                bool OK = loadGcode();

                if (OK) {
                    par.cnc.initialContext(par.cncContext.get());

                    if (par.gcodeSettings.isUV()) {
                        par.cnc.recoveryUV(par.gcodeSettings);
                    }

                    par.appState.gotoPauseState();
                    updateButtons();

                    m_report.writeLine(tr("G-code is loaded"));
                    par.cnc.imitEna(false);
                    m_full_length_valid = false;

                    if (timer) {
//                        timer->clear();                        
                        timer->start();
                    }

                    par.cncContext.setValid(false);
                } else {
                    qDebug("form_run::FormRun::init(true)>>Load G-code error");
                    m_report.writeLine(tr("Load G-code error"));
                }
            }
        }

        if (!cncReaderEna)
            startCncReader();

#ifndef STONE
        startAdc();
#endif
    } catch (const runtime_error& e) {
        m_report.writeLine( "Connection error.\n" + QString(e.what()) );
        par.cnc.close();
        return;
    } catch (const string& msg) {
        m_report.writeLine( "Error: " + msg );
        return;
    } catch (const exception& e) {
        m_report.writeLine("Error: " + QString(e.what()) + ")");
        return;
    } catch (...) {
        m_report.writeLine("Error: FormRun unknown exception");
        return;
    }
}

void FormRun::_init() {
    par.gcodeCnc.clear();
    runWidget->txtCode->clear();
    par.workContours.clear();
    par.mapGcodeToContours.clear();
    runWidget->txtMsg->clear();
    m_info.clear();
    runWidget->setElapsedTime(0);
    runWidget->setRemainTime(0);
    timer->clear();
    m_full_length_valid = false;

    if (par.gcode.empty())
        m_info = tr("Error") + ": " + tr("No G-code") + "\n";
    else {
        par.gcode.normalize();

        if (!par.gcode.empty()) {
            par.gcodeCnc = par.gcode.toText().c_str();
//            text = addLineNum(text);
            runWidget->txtCode->setPlainText(par.gcodeCnc);
            setCursorToBegin();
            par.workContours = ContourList( par.gcode.getContours(&par.mapGcodeToContours) );
        }
        else
            m_info = tr("Error") + ": " + tr("G-code normalization error") + "\n";
    }

    runWidget->checkIdle->setCheckState( par.appState.idleRun() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );

    updateButtons();
    runWidget->setButtonsEnabled(0, 0, 0, 1);

    runWidget->plot(par.workContours, par.swapXY, par.reverseX, par.reverseY, par.showXY);

    if (!par.cnc.isOpen())
        m_info += tr("Error") + ": " + tr("No CNC connection") + "\n";

    m_report.write(m_info);
}

void FormRun::highlightCurrentLine() {
    setCursor(currentCursorPosition);

    QList<QTextEdit::ExtraSelection> extraSelections;

    QTextEdit::ExtraSelection selection;

    QColor lineColor = QColor(Qt::yellow).lighter(160);

    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = runWidget->txtCode->textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);

    runWidget->txtCode->setExtraSelections(extraSelections);
}

void FormRun::on_btnHome_clicked() {
    stopCncReader();
    stopAdc();
    emit homePageClicked();
}

QString FormRun::addLineNum(QString txt) {
    QStringList list = txt.split(QRegExp("(\\n|\\r\\n|\\n\\r)"));
    QString res;
    int i = 0;
    for (auto it = list.begin(); it != list.end(); ++it, i++)
        res += QString("N%1 %2\n").arg(QString::number(i), *it);

    return res;
}

void FormRun::updateButtons() {
    const QString statusStartStop   = tr("Start/Stop running");
    const QString statusStart       = tr("Start cutting");
    const QString statusStop        = tr("Stop cutting");

    switch (par.appState.state()) {
    case AppState::STATES::ST_NONE:
        runWidget->setButtonsText({tr("Start"), tr("Back"), tr("Cancel")},
                                  {statusStart, tr("Go to back"), tr("Cancel program")});
        btnHome->setEnabled(true);

        if (par.appState.idleRun() || (btnBreak->isChecked() && btnPump->isChecked() && btnRoll->isChecked()))
            runWidget->setButtonsEnabled(1,0,0,1);
        else
            runWidget->setButtonsEnabled(0,0,0,1);

        break;
    case AppState::STATES::ST_IDLE:
        runWidget->setButtonsText({tr("Start"), tr("Back"), tr("Cancel")},
                                  {statusStart, tr("Go to back"), tr("Cancel program")});
        btnHome->setEnabled(true);

        if (par.appState.idleRun() || (btnBreak->isChecked() && btnPump->isChecked() && btnRoll->isChecked()))
            runWidget->setButtonsEnabled(1,0,0,1);
        else
            runWidget->setButtonsEnabled(0,0,0,1);

        break;
    case AppState::STATES::ST_RUN:
        runWidget->setButtonsText({tr("Stop"), "", ""},
                                  {statusStop, "", ""});
        runWidget->setButtonsEnabled(1,0,0,0);
        btnHome->setEnabled(false);
        break;
    case AppState::STATES::ST_PAUSE:
        runWidget->setButtonsText({tr("Start"), tr("Back"), tr("Cancel")},
                                  {statusStart, tr("Go to back"), tr("Cancel program")});
        btnHome->setEnabled(false);

        if (par.appState.idleRun() || (btnBreak->isChecked() && btnPump->isChecked() && btnRoll->isChecked()))
            runWidget->setButtonsEnabled(1,1,1,1);
        else
            runWidget->setButtonsEnabled(0,1,1,1);

        break;
    case AppState::STATES::ST_REV:
        runWidget->setButtonsText({tr("Stop"), "", ""},
                                  {statusStop, "", ""});
        runWidget->setButtonsEnabled(1,0,0,0);
        btnHome->setEnabled(false);
        break;
    case AppState::STATES::ST_CANCEL:
        runWidget->setButtonsText({"", tr("Shortcut"), "Reset"},
                                  {statusStartStop, tr("Fast return to start position"), tr("Reset CNC")});
        runWidget->setButtonsEnabled(0,1,1,1);
        btnHome->setEnabled(false);
        break;
    case AppState::STATES::ST_SHORT_REV:
        runWidget->setButtonsText({tr("Stop"), "", ""},
                                  {statusStop, tr(""), tr("")});
        runWidget->setButtonsEnabled(1,0,0,0);
        btnHome->setEnabled(false);
        break;
    default:
        runWidget->setButtonsText({"", "", tr("Reset")},
                                  {statusStartStop, "", tr("Reset CNC")});
        runWidget->setButtonsEnabled(0,0,1,1);
        btnHome->setEnabled(false);
        break;
    }
}

void FormRun::on_btnStart_clicked() {
    blockSignals(true);

    switch (par.appState.state()) {
    case AppState::STATES::ST_NONE:
        _init();
        [[clang::fallthrough]];

    case AppState::STATES::ST_IDLE:
        try {
            runWidget->btnStart->setEnabled(false);
            QString tmp = runWidget->btnStart->text();
            runWidget->btnStart->setText(tr("Loading"));
            runWidget->btnStart->update();
            runWidget->btnStart->repaint();

            par.saveGcode();
//            par.cnc.stateClear();

            stopCncReader();
            bool OK = loadGcode();
            startCncReader();

            runWidget->btnStart->setEnabled(true);
            runWidget->btnStart->setText(tmp);
            runWidget->btnStart->update();

            if (OK) {
//                m_report.append(tr("G-code is loaded"));
                qDebug("form_run::on_btnStart_clicked>>G-code is loaded");

                m_info.chop(1); // clear \n
                m_info += ". " + tr("Loaded") + "\n";
                runWidget->txtMsg->setText(m_info);

                par.cnc.imitEna(false);

                if (runWidget->checkIdle->isChecked() || (btnBreak->isChecked() && btnPump->isChecked() && btnRoll->isChecked())) {
                    if (!runWidget->checkIdle->isChecked())
                        par.cnc.writeHoldEnable(true);

                    par.cnc.runReq();
                    if (timer) {
                        timer->clear();
                        m_full_length_valid = false;
                        timer->start();
                    }
                }
            } else {
                qDebug("form_run::on_btnStart_clicked>>Load G-code error");
                m_info.chop(1); // clear \n
                m_info += " " + tr("Load G-code error") + "\n";
                runWidget->txtMsg->setText(m_info);
            }
        } catch (string& s) {
            m_report.writeLine(s);
            return;
        } catch (exception& e) {
            m_report.writeLine("Error: exception (" + QString(e.what()) + ")");
            return;
        } catch (...) {
            m_report.writeLine("Error: Form run unknown exception");
            return;
        }
        break;

    case AppState::STATES::ST_RUN:
    case AppState::STATES::ST_REV:
        par.cnc.stopReq();
        if (timer)
            timer->stop();
        break;

    case AppState::STATES::ST_PAUSE:
        if (runWidget->checkIdle->isChecked() || (btnBreak->isChecked() && btnPump->isChecked() && btnRoll->isChecked())) {
            if (!runWidget->checkIdle->isChecked())
                par.cnc.writeHoldEnable(true);

            par.cnc.runReq();
            if (timer)
                timer->start();
        }
        break;

    default:
//        par.cnc.stopReq();
        break;
    }

    // todo: more reactions
    par.appState.next(AppState::BUTTON::SIG_START);
    updateButtons();

    blockSignals(false);
}

void FormRun::on_btnReverse_clicked() {
    blockSignals(true);

    try {
        if (!runWidget->checkIdle->isChecked())
            par.cnc.writeHoldEnable(true);

        par.cnc.revReq();
        if (timer)
            timer->start();

        par.appState.next(AppState::BUTTON::SIG_REVERSE);
        updateButtons();
    } catch (...) {}

    blockSignals(false);
}

void FormRun::on_btnCancel_clicked() {
    blockSignals(true);

    try {
        par.saveCncContext();
        int roll_vel = numRollVel->value();

        par.cnc.cancelReq();
        par.cnc.writeRollVel(roll_vel);

        par.appState.next(AppState::BUTTON::SIG_CANCEL);
        updateButtons();
    } catch (...) {}

    blockSignals(false);
}

void FormRun::setCursor(int row) {
    currentCursorPosition = row;

    QTextCursor textCursor = runWidget->txtCode->textCursor();

    textCursor.movePosition(QTextCursor::Start);
    textCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, currentCursorPosition);

    runWidget->txtCode->setTextCursor(textCursor);
    runWidget->txtCode->update();
}

void FormRun::setCursorToBegin() {
    QTextCursor textCursor = runWidget->txtCode->textCursor();
    textCursor.movePosition(QTextCursor::Start);

    currentCursorPosition = textCursor.position();

    runWidget->txtCode->setTextCursor(textCursor);
    runWidget->txtCode->update();
}

void FormRun::setCursorToEnd() {
    QTextCursor textCursor = runWidget->txtCode->textCursor();
    textCursor.movePosition(QTextCursor::End);

    currentCursorPosition = textCursor.position();

    runWidget->txtCode->setTextCursor(textCursor);
    runWidget->txtCode->update();
}

void FormRun::startCncReader() {
    if (par.cnc.isOpen()) {
        qDebug()<<"CNC reader: Start";
        cncReaderEna = true;
        remain_tmr = 0;
        m_full_length_valid = false;
        readCncContext();
    }
}

void FormRun::stopCncReader() {
    qDebug()<<"CNC reader: Stop";
    cncReaderEna = false;
    remain_tmr = 0;
    timer->clear();
    m_full_length_valid = false;
}

void FormRun::readCncContext() {
    if (cncReaderEna) {
//        qDebug()<<"CNC reader: Reading...";

        blockSignals(true);

        try {
            par.cncContext = par.cnc.readCncContext();
            const CncContext& ctx = par.cncContext;
#ifndef STONE
            runWidget->txtMsg->setText(m_info + QString( ctx.toStringRunDebug().c_str() ));
#else
            runWidget->txtMsg->setText(m_info + QString( ctx.toStringRunStoneDebug().c_str() ));
#endif
//            qDebug() << "CNC reader: " + QString(ctx.toString().c_str());

            runWidget->setPositionLabels(0, ctx.x());
            runWidget->setPositionLabels(1, ctx.y());
            runWidget->setPositionLabels(2, ctx.u());
            runWidget->setPositionLabels(3, ctx.v());

            runWidget->setEncoderLabels(0, ctx.encoderX());
            runWidget->setEncoderLabels(1, ctx.encoderY());

            qint64 ms = timer ? timer->elapsed() : 0;
            runWidget->setElapsedTime(ms);

            runWidget->setLimitSwitches(ctx.limitSwitches());

            btnRoll->setChecked(ctx.isRollEnable());
            btnPump->setChecked(ctx.pumpEnabled());
            btnBreak->setChecked(ctx.wireControlEnabled());
            btnHighVolt->setChecked(ctx.highVoltageEnabled());

            runWidget->btnHold->setChecked(ctx.hold());

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
            btnLowHighVolt->setChecked(ctx.lowHighVoltageEnabled());
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

            m_speed.setMMM(ctx.speed());

            if (m_speed.get() < m_speed.min()) {
                runWidget->numSpeed->setValue(m_speed.min());
                qDebug("Error: MIN CNC Speed: %f", ctx.speed());
            }
            else if (m_speed.get() > m_speed.max()) {
                runWidget->numSpeed->setValue(m_speed.max());
                qDebug("Error: MAX CNC Speed: %f", ctx.speed());
            }
            else
                runWidget->numSpeed->setValue(m_speed.get());

            if (par.appState.isWork() && ctx.isWork()) {
                setCursor(ctx.frameNum());

                if (ctx.isEncoderMode()) {
                    onFrameChanged(
                        ctx.frameNum(),
                        fpoint_t(ctx.encoderX() / CncParam::scaleEncX, ctx.encoderY() / CncParam::scaleEncY),
                        fpoint_t(ctx.u() / CncParam::scaleU, ctx.v() / CncParam::scaleV)
                    );
                } else {
                    onFrameChanged(
                        ctx.frameNum(),
                        fpoint_t(ctx.x() / CncParam::scaleX, ctx.y() / CncParam::scaleY),
                        fpoint_t(ctx.u() / CncParam::scaleU, ctx.v() / CncParam::scaleV)
                    );
                }

                if (remain_tmr >= REMAIN_TIMER_MAX) {
                    remain_tmr = 0;

                    if (!m_full_length_valid) {
                        m_full_length = par.workContours.botLengthFull();
                        m_full_length_valid = true;
                    }

                    double length = par.workContours.botLength();...

                    double speed = ms > 0 ? length / (double)ms : 0;

                    double remain_length = m_full_length - length;
                    double remain_time = remain_length / speed;

                    if (remain_time < 0)
                        remain_time = 0;

                    runWidget->setRemainTime(static_cast<qint64>(remain_time));

                    qDebug("Remain: %0.3f mm, %0.3f mm, %d ms, %d ms", m_full_length, length, (int)ms, (int)remain_time);
                }
                else
                    ++remain_tmr;
            }

            if (par.appState.isError() || ctx.isError() || cutStateAbortReq) {
                if (ctx.isError()) {
                    par.cnc.cancelReq();
                    par.appState.update(ctx);
                    updateButtons();
                    qDebug("CutState: Error");
                }
                else
                    par.cnc.stopReq();

                if (par.appState.isError())
                    qDebug("RunWidgetState: Error");

                if (cutStateAbortReq) {
                    qDebug("CutState: Abort request");
                    cutStateAbortReq = false;
                }

                if (timer)
                    timer->stop();
            }
            else if (par.appState.isWork() && !ctx.isWork()) {
                par.appState.reset();
                updateButtons();                
                setCursorToEnd();

                if (timer)
                    timer->stop();

                runWidget->setRemainTime(0);
            }
            else {
                par.appState.update(ctx);
                updateButtons();
            }
        }
        catch (...) {}

        blockSignals(false);

        QTimer::singleShot(POLLING_TIME, this, &FormRun::readCncContext);
    }
}

void FormRun::startAdc() { // on init
    if (par.cnc.isOpen()) {
        qDebug()<<"ADC: Start";
        adcEnable = true;
        readAdc();
    }
    else
        adcEnable = false;
}

void FormRun::stopAdc() { // at to home
    qDebug()<<"ADC: Stop";
    adcEnable = false;
}

void FormRun::readAdc() {
    if (adcEnable) {
        cnc_adc_volt_t adc = par.cnc.readADCVolt();
        runWidget->setAdc(adc);
        QTimer::singleShot(ADC_POLLING_TIME, this, &FormRun::readAdc);
    }
}

void FormRun::onFrameChanged(int frame_num, const fpoint_t& bot_pos, const fpoint_t& top_pos) {
    if (!par.workContours.empty() && !par.mapGcodeToContours.empty()) {
        if (frame_num >= 0) {
            par.workContours.select( par.getDxfEntityNum(size_t(frame_num)) );

            if (par.gcodeSettings.isUV()) {
                par.workContours.setXYPos(bot_pos);
                par.workContours.setUVPos(bot_pos + top_pos);
                fpoint_t xy = par.workContours.intersectUV();
                par.workContours.setBotPos(xy);
            }
            else {
                par.workContours.setBotPos(bot_pos);
            }
        }
        else
            par.workContours.clearSelected();

        runWidget->plot(par.workContours, par.swapXY, par.reverseX, par.reverseY, par.showXY);
    }
}
