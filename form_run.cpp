#include "form_run.h"
#include "aux_items.h"
#include <QDebug>
#include <QTimer>
#include <QMessageBox>

using namespace std;

FormRun::FormRun(ProgramParam& par, QWidget *parent) :
    QWidget(parent), par(par), m_cncReaderEna(false), m_adcEnable(false), m_currentCursorPosition(0), m_cutStateAbortReq(false), m_speed(WireSpeed()), m_remain_tmr(0), m_full_length(9)
{
    this->setObjectName(tr("Work Panel"));
    m_runWidget = new RunWidget(this);
    createSpinBoxes(); // row 0
    addButtons(); // row 1

//    setFontPointSize(14);

    m_mainLayout = new QVBoxLayout;
    m_mainLayout->addWidget(m_runWidget);
    m_mainLayout->addLayout(m_gridButtons);

    this->setLayout(m_mainLayout);

    m_timer = new StartStopElapsedTimer;

    connect(m_runWidget->txtCode, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    highlightCurrentLine();

    m_report.bind(m_runWidget->txtMsg);

    connect(m_runWidget->btnStart, &QPushButton::clicked, this, &FormRun::on_btnStart_clicked);
    connect(m_runWidget->btnReverse, &QPushButton::clicked, this, &FormRun::on_btnReverse_clicked);
    connect(m_runWidget->btnCancel, &QPushButton::clicked, this, &FormRun::on_btnCancel_clicked);
    connect(m_runWidget->checkIdle, &QCheckBox::clicked, this, [&](bool checked) {
        par.appState.setIdleRun(checked);
    });

//    bindNumberButtons(numWidth, btnWidthInc, btnWidthDec);

//    connect(sender, &Sender::valueChanged,
//        [=]( const QString &newValue ) { receiver->updateValue( "senderValue", newValue ); }
//    );
}

FormRun::~FormRun() {
    stopCncReader();
    if (m_timer) {
        delete m_timer;
        m_timer = nullptr;
    }
}

void FormRun::setFontPointSize(int pointSize) {
    m_runWidget->setFontPointSize(pointSize);

    for (QPushButton* b: m_buttons) {
        if (!b) continue;

        QFont font = b->font();
        font.setPointSize(pointSize);
        b->setFont(font);
//        b->setStyleSheet("font: bold");
    }

    for (QSpinBox* const numBox: m_numBoxes) {
        if (!numBox) continue;

//        numBox->setSizePolicy( QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum) );
        QFont font = numBox->font();
        font.setPointSize(pointSize);
//        font.setBold(true);
        numBox->setFont(font);
    }

    vector<QLabel*> labels = {m_labelRoll, m_labelWidth, m_labelRatio, m_labelCurrent};
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
    m_btnHome = new QPushButton(tr("Home"));
    m_btnHome->setStatusTip(tr("Go to the Home panel") + "   Alt+H");
    m_btnHome->setShortcut(QKeySequence("Alt+H"));

    m_btnHelp = new QPushButton(tr("Help"));
    m_btnHelp->setStatusTip(tr("Open Help") + "   F1");
    m_btnHelp->setShortcut(QKeySequence::HelpContents);

    m_btnBreak = new QPushButton(tr("Break"));
    m_btnBreak->setCheckable(true);
    m_btnBreak->setStatusTip(tr("Wire break control on/off"));
    QPalette palButton = m_btnBreak->palette();
    palButton.setColor(QPalette::Disabled, QPalette::Button, Qt::darkGreen); //...
    m_btnBreak->setPalette(palButton);

    m_btnPump = new QPushButton(tr("Pump"));
    m_btnPump->setCheckable(true);
    m_btnPump->setStatusTip(tr("Pump on/off"));

    m_btnRoll = new QPushButton(tr("Roll"));
    m_btnRoll->setCheckable(true);
    m_btnRoll->setStatusTip(tr("Wire roll on/off"));

    m_btnRollVelDec = new QPushButton(tr("DEC"));
    m_btnRollVelDec->setStatusTip(tr("Roll velocity decrement"));

    m_btnRollVelInc = new QPushButton(tr("INC"));
    m_btnRollVelInc->setStatusTip(tr("Roll velocity increment"));

    m_btnHighVolt = new QPushButton;
#ifndef STONE
    m_btnHighVolt->setCheckable(true);
    m_btnHighVolt->setText(tr("High Voltage"));
    m_btnHighVolt->setStatusTip(tr("Enable High Voltage"));
#else
    m_btnHighVolt->setEnabled(false);
#endif

#ifndef STONE
    m_btnLowHighVolt = new QPushButton;
    m_btnLowHighVolt->setCheckable(true);
    m_btnLowHighVolt->setText(tr("Low High Volt."));
    m_btnLowHighVolt->setStatusTip(tr("Enable Low High Voltage"));
#endif

    m_btnCurrentDec = new QPushButton;
#ifndef STONE
    m_btnCurrentDec->setText(tr("DEC"));
#else
    m_btnCurrentDec->setEnabled(false);
#endif

    m_btnCurrentInc = new QPushButton;
#ifndef STONE
    m_btnCurrentInc->setText(tr("INC"));
#else
    m_btnCurrentInc->setEnabled(false);
#endif

    m_btnWidthDec = new QPushButton;
#ifndef STONE
    m_btnWidthDec->setText(tr("DEC"));
#else
    m_btnWidthDec->setEnabled(false);
#endif

    m_btnWidthInc = new QPushButton;
#ifndef STONE
    m_btnWidthInc->setText(tr("INC"));
#else
    m_btnWidthInc->setEnabled(false);
#endif

    m_btnRatioDec = new QPushButton;
#ifndef STONE
    m_btnRatioDec->setText(tr("DEC"));
#else
    m_btnRatioDec->setEnabled(false);
#endif

    m_btnRatioInc = new QPushButton;
#ifndef STONE
    m_btnRatioInc->setText(tr("INC"));
#else
    m_btnRatioInc->setEnabled(false);
#endif

    m_buttons = {
        m_btnBreak, m_btnPump, m_btnRoll, m_btnRollVelDec, m_btnRollVelInc,\
        m_btnHighVolt, m_btnCurrentDec, m_btnCurrentInc, m_btnWidthDec, m_btnWidthInc, m_btnRatioInc, m_btnRatioDec,\
        m_btnHome, m_btnHelp
#ifndef STONE
        , m_btnLowHighVolt
#endif
    };

#ifdef STONE
    m_gridButtons->addWidget(m_btnHome, 1, 0);
    m_gridButtons->addWidget(m_btnBreak, 1, 1);
    m_gridButtons->addWidget(m_btnPump, 1, 2);
    m_gridButtons->addWidget(m_btnRoll, 1, 3);
    m_gridButtons->addWidget(m_btnRollVelDec, 1, 4);
    m_gridButtons->addWidget(m_btnRollVelInc, 1, 5);
    m_gridButtons->addWidget(m_btnHighVolt, 1, 6);
    m_gridButtons->addWidget(m_btnCurrentDec, 1, 7);
    m_gridButtons->addWidget(m_btnCurrentInc, 1, 8);
    m_gridButtons->addWidget(m_btnWidthDec, 1, 9);
    m_gridButtons->addWidget(m_btnWidthInc, 1, 10);
    m_gridButtons->addWidget(m_btnRatioDec, 1, 11);
    m_gridButtons->addWidget(m_btnRatioInc, 1, 12);
#else
    m_gridButtons->addWidget(m_btnHome, 0, 0);

    m_gridButtons->addWidget(m_btnBreak, 1, 0);
    m_gridButtons->addWidget(m_btnPump, 1, 1);
    m_gridButtons->addWidget(m_btnRoll, 1, 2);
    m_gridButtons->addWidget(m_btnRollVelDec, 1, 3);
    m_gridButtons->addWidget(m_btnRollVelInc, 1, 4);
    m_gridButtons->addWidget(m_btnHighVolt, 1, 5);
    m_gridButtons->addWidget(m_btnCurrentDec, 1, 6);
    m_gridButtons->addWidget(m_btnCurrentInc, 1, 7);
    m_gridButtons->addWidget(m_btnWidthDec, 1, 8);
    m_gridButtons->addWidget(m_btnWidthInc, 1, 9);
    m_gridButtons->addWidget(m_btnRatioDec, 1, 10);
    m_gridButtons->addWidget(m_btnRatioInc, 1, 11);
    m_gridButtons->addWidget(m_btnLowHighVolt, 1, 12);
#endif
    m_gridButtons->addWidget(m_btnHelp, 1, 13);

    connect(m_btnHome, &QPushButton::clicked, this, &FormRun::on_btnHome_clicked);
    connect(m_btnHelp, &QPushButton::clicked, this, [&]() { emit helpPageClicked(m_help_file); });

    connect(m_btnRoll, &QPushButton::clicked, this, [&]() {
        blockSignals(true);
        try {            
            par.cnc.writeRollEnable( m_btnRoll->isChecked() );
        }
        catch (...) {}
        blockSignals(false);
    });

    connect(m_btnPump, &QPushButton::clicked, this, [&]() {
        blockSignals(true);
        try {
            par.cnc.writePumpEnable( m_btnPump->isChecked() );
        }
        catch (...) {}
        blockSignals(false);
    });

    connect(m_btnBreak, &QPushButton::clicked, this, [&]() {
        blockSignals(true);
        try {            
            par.cnc.writeWireEnable( m_btnBreak->isChecked() );
        }
        catch (...) {}
        blockSignals(false);
    });

    connect(m_btnHighVolt, &QPushButton::clicked, this, [&]() {
        blockSignals(true);
        try {
            par.cnc.writeEnableHighVoltage( m_btnHighVolt->isChecked() );
        }
        catch (...) {}
        blockSignals(false);
    });

#ifndef STONE
    connect(m_btnLowHighVolt, &QPushButton::clicked, this, [&]() {
        blockSignals(true);
        try {            
            par.cnc.writeEnableLowHighVolt( m_btnLowHighVolt->isChecked() );
        }
        catch (...) {}
        blockSignals(false);
    });
#endif

    connect(m_runWidget->btnHold, &QPushButton::clicked, this, [&]() {
        blockSignals(true);
        try {
            par.cnc.writeHoldEnable( m_runWidget->btnHold->isChecked() );
        }
        catch (...) {}
        blockSignals(false);
    });

    connect(m_btnRollVelInc, &QPushButton::clicked, this, [&]() { m_numRollVel->stepUp(); });
    connect(m_btnRollVelDec, &QPushButton::clicked, this, [&]() { m_numRollVel->stepDown(); });
    connect(m_btnWidthInc, &QPushButton::clicked, this, [&]() { m_numWidth->stepUp(); });
    connect(m_btnWidthDec, &QPushButton::clicked, this, [&]() { m_numWidth->stepDown(); });
    connect(m_btnRatioInc, &QPushButton::clicked, this, [&]() { m_numRatio->stepUp(); });
    connect(m_btnRatioDec, &QPushButton::clicked, this, [&]() { m_numRatio->stepDown(); });
    connect(m_btnCurrentInc, &QPushButton::clicked, this, [&]() { m_numCurrent->stepUp(); });
    connect(m_btnCurrentDec, &QPushButton::clicked, this, [&]() { m_numCurrent->stepDown(); });
}

void FormRun::createSpinBoxes() {
    m_numRollVel = new QSpinBox;
    m_numRollVel->setRange(cnc_param::ROLL_MIN, cnc_param::ROLL_MAX);

    m_numCurrent = new QSpinBox;
    m_numCurrent->setRange(cnc_param::CURRENT_MIN, cnc_param::CURRENT_MAX);

    m_numWidth = new QSpinBox;
    m_numWidth->setRange(cnc_param::PULSE_WIDTH_MIN, cnc_param::PULSE_WIDTH_MAX);

    m_numRatio = new QSpinBox;
    m_numRatio->setRange(cnc_param::PULSE_RATIO_MIN, cnc_param::PULSE_RATIO_MAX);

    m_numBoxes = {m_numRollVel, m_numWidth, m_numRatio, m_numCurrent};

    m_labelRoll = new QLabel(tr("Roll Velocity") + ": ");
    m_labelRoll->setBuddy(m_numRollVel);
    m_labelWidth = new QLabel(tr("Width") + ": ");
    m_labelWidth->setBuddy(m_numWidth);
    m_labelRatio = new QLabel(tr("Ratio") + ": ");
    m_labelRatio->setBuddy(m_numRatio);
    m_labelCurrent = new QLabel(tr("Current") + ": ");
    m_labelCurrent->setBuddy(m_numCurrent);

    m_groupRoll = new QGroupBox;
    m_groupWidth = new QGroupBox;
    m_groupRatio = new QGroupBox;
    m_groupCurrent = new QGroupBox;

    m_groupRoll->setLayout(new QHBoxLayout);
    m_groupRoll->layout()->addWidget(m_labelRoll);
    m_groupRoll->layout()->addWidget(m_numRollVel);

    m_groupWidth->setLayout(new QHBoxLayout);
    m_groupWidth->layout()->addWidget(m_labelWidth);
    m_groupWidth->layout()->addWidget(m_numWidth);

    m_groupRatio->setLayout(new QHBoxLayout);
    m_groupRatio->layout()->addWidget(m_labelRatio);
    m_groupRatio->layout()->addWidget(m_numRatio);

    m_groupCurrent->setLayout(new QHBoxLayout);
    m_groupCurrent->layout()->addWidget(m_labelCurrent);
    m_groupCurrent->layout()->addWidget(m_numCurrent);

    m_gridButtons = new QGridLayout;

#ifdef STONE
    m_gridButtons->addWidget(m_groupRoll, 0, 4, 1, 2);
#else
    m_gridButtons->addWidget(m_groupRoll, 0, 3, 1, 2);

    m_gridButtons->addWidget(m_groupCurrent, 0, 6, 1, 2);
    m_gridButtons->addWidget(m_groupWidth, 0, 8, 1, 2);
    m_gridButtons->addWidget(m_groupRatio, 0, 10, 1, 2);
#endif

    connect(m_numRollVel, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int x) {
        blockSignals(true);
        try {
            par.cnc.writeRollVel(static_cast<unsigned>(x));
        } catch (...) {}
        blockSignals(false);
    });

    connect(m_numWidth, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int x) {
        blockSignals(true);
        try {
            par.cnc.writePulseWidth(static_cast<unsigned>(x));
        } catch (...) {}
        blockSignals(false);
    });

    connect(m_numRatio, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int x) {
        blockSignals(true);
        try {
            par.cnc.writePulseRatio(static_cast<unsigned>(x));
        } catch (...) {}
        blockSignals(false);
    });

    connect(m_numCurrent, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int x) {
        blockSignals(true);
        try {
            par.cnc.writeCurrentIndex(static_cast<unsigned>(x));
        } catch (...) {}
        blockSignals(false);
    });

    //
    m_runWidget->numSpeed->setRange(m_speed.min(), m_speed.max());
    m_runWidget->numSpeed->setValue(m_speed.get());
    m_runWidget->numSpeed->setDecimals(2);
    m_runWidget->numSpeed->setSingleStep(0.01);

    connect(m_runWidget->numSpeed, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=](double x) {
        blockSignals(true);

        m_speed.set(x);
        par.cnc.writeSpeed(m_speed);

        blockSignals(false);
    });

//    connect(runWidget->btnSpeed, &QPushButton::clicked, this, [&]() {
//        m_speed.set(runWidget->numSpeed->value());
//        par.cnc.writeSpeed(m_speed);
//    });

    connect(m_runWidget->speedMMM, &QRadioButton::clicked, this, [&]() {
        blockSignals(true);

        if (m_speed.mode() != WireSpeed::Mode::MMM) {
            m_speed.changeMode(WireSpeed::Mode::MMM);            

            unsigned dec = 2;
//            runWidget->numSpeed->setRange(DBL_MIN, DBL_MAX);
            m_runWidget->numSpeed->setDecimals(dec);
            m_runWidget->numSpeed->setSingleStep( pow(10, -dec) );
            m_runWidget->numSpeed->setRange(m_speed.min(), m_speed.max());

            double p = pow(10, dec);
            m_runWidget->numSpeed->setValue( round(m_speed.get() * p) / p );
        }

        blockSignals(false);
    });

    connect(m_runWidget->speedUMS, &QRadioButton::clicked, this, [&]() {
        blockSignals(true);

        if (m_speed.mode() != WireSpeed::Mode::UMS) {
            m_speed.changeMode(WireSpeed::Mode::UMS);            

            unsigned dec = 1;
//            runWidget->numSpeed->setRange(DBL_MIN, DBL_MAX);
            m_runWidget->numSpeed->setDecimals(dec);
            m_runWidget->numSpeed->setSingleStep( pow(10, -dec) );
            m_runWidget->numSpeed->setRange(m_speed.min(), m_speed.max());

            double p = pow(10, dec);
            m_runWidget->numSpeed->setValue( round(m_speed.get() * p) / p );
        }

        blockSignals(false);
    });
}

void FormRun::blockSignals(bool value) {
    m_btnRoll->blockSignals(value);
    m_btnPump->blockSignals(value);
    m_btnBreak->blockSignals(value);
    m_btnHighVolt->blockSignals(value);
#ifndef STONE
    m_btnLowHighVolt->blockSignals(value);
#endif

    m_numRollVel->blockSignals(value);
    m_numWidth->blockSignals(value);
    m_numRatio->blockSignals(value);
    m_numCurrent->blockSignals(value);

    m_runWidget->btnHold->blockSignals(value);
    m_runWidget->btnStart->blockSignals(value);
    m_runWidget->btnReverse->blockSignals(value);
    m_runWidget->btnCancel->blockSignals(value);

    m_runWidget->numSpeed->blockSignals(value);
}

using namespace auxItems;

bool FormRun::loadGcode() {    
    size_t wrsize, pa_size;
    m_info.clear();

    qDebug("Write frames");
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

    qDebug("Read frames");
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
        par.cnc.stateReset();

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

                stopCncReader();
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

                    if (m_timer) {
//                        timer->clear();                        
                        m_timer->start();
                    }

                    par.cncContext.setValid(false);
                } else {
                    qDebug("form_run::FormRun::init(true)>>Load G-code error");
                    m_report.writeLine(tr("Load G-code error"));
                }
            }
        }

        if (!m_cncReaderEna)
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
    m_runWidget->txtCode->clear();
    par.workContours.clear();
    par.mapGcodeToContours.clear();
    m_runWidget->txtMsg->clear();
    m_info.clear();
    m_runWidget->setElapsedTime(0);
    m_runWidget->setRemainTime(0);
    m_timer->clear();

    if (par.gcode.empty())
        m_info = tr("Error") + ": " + tr("No G-code") + "\n";
    else {
        par.gcode.normalize();

        if (!par.gcode.empty()) {
            par.gcodeCnc = par.gcode.toText().c_str();
//            text = addLineNum(text);
            m_runWidget->txtCode->setPlainText(par.gcodeCnc);
            setCursorToBegin();
            par.workContours = ContourList( par.gcode.getContours(&par.mapGcodeToContours) );
            m_full_length = par.workContours.botLengthFull();
        }
        else
            m_info = tr("Error") + ": " + tr("G-code normalization error") + "\n";
    }

    m_runWidget->checkIdle->setCheckState( par.appState.idleRun() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );

    updateButtons();
    m_runWidget->setButtonsEnabled(0, 0, 0, 1);

    m_runWidget->plot(par.workContours, par.swapXY, par.reverseX, par.reverseY, par.showXY);

    if (!par.cnc.isOpen())
        m_info += tr("Error") + ": " + tr("No CNC connection") + "\n";

    m_report.write(m_info);
}

void FormRun::highlightCurrentLine() {
    setCursor(m_currentCursorPosition);

    QList<QTextEdit::ExtraSelection> extraSelections;

    QTextEdit::ExtraSelection selection;

    QColor lineColor = QColor(Qt::yellow).lighter(160);

    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = m_runWidget->txtCode->textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);

    m_runWidget->txtCode->setExtraSelections(extraSelections);
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
        m_runWidget->setButtonsText({tr("Start"), tr("Back"), tr("Cancel")},
                                  {statusStart, tr("Go to back"), tr("Cancel program")});
        m_btnHome->setEnabled(true);

        if (par.appState.idleRun() || (m_btnBreak->isChecked() && m_btnPump->isChecked() && m_btnRoll->isChecked()))
            m_runWidget->setButtonsEnabled(1,0,0,1);
        else
            m_runWidget->setButtonsEnabled(0,0,0,1);

        break;
    case AppState::STATES::ST_IDLE:
        m_runWidget->setButtonsText({tr("Start"), tr("Back"), tr("Cancel")},
                                  {statusStart, tr("Go to back"), tr("Cancel program")});
        m_btnHome->setEnabled(true);

        if (par.appState.idleRun() || (m_btnBreak->isChecked() && m_btnPump->isChecked() && m_btnRoll->isChecked()))
            m_runWidget->setButtonsEnabled(1,0,0,1);
        else
            m_runWidget->setButtonsEnabled(0,0,0,1);

        break;
    case AppState::STATES::ST_RUN:
        m_runWidget->setButtonsText({tr("Stop"), "", ""},
                                  {statusStop, "", ""});
        m_runWidget->setButtonsEnabled(1,0,0,0);
        m_btnHome->setEnabled(false);
        break;
    case AppState::STATES::ST_PAUSE:
        m_runWidget->setButtonsText({tr("Start"), tr("Back"), tr("Cancel")},
                                  {statusStart, tr("Go to back"), tr("Cancel program")});
        m_btnHome->setEnabled(false);

        if (par.appState.idleRun() || (m_btnBreak->isChecked() && m_btnPump->isChecked() && m_btnRoll->isChecked()))
            m_runWidget->setButtonsEnabled(1,1,1,1);
        else
            m_runWidget->setButtonsEnabled(0,1,1,1);

        break;
    case AppState::STATES::ST_REV:
        m_runWidget->setButtonsText({tr("Stop"), "", ""},
                                  {statusStop, "", ""});
        m_runWidget->setButtonsEnabled(1,0,0,0);
        m_btnHome->setEnabled(false);
        break;
    case AppState::STATES::ST_CANCEL:
        m_runWidget->setButtonsText({"", tr("Shortcut"), "Reset"},
                                  {statusStartStop, tr("Fast return to start position"), tr("Reset CNC")});
        m_runWidget->setButtonsEnabled(0,1,1,1);
        m_btnHome->setEnabled(false);
        break;
    case AppState::STATES::ST_SHORT_REV:
        m_runWidget->setButtonsText({tr("Stop"), "", ""},
                                  {statusStop, tr(""), tr("")});
        m_runWidget->setButtonsEnabled(1,0,0,0);
        m_btnHome->setEnabled(false);
        break;
    default:
        m_runWidget->setButtonsText({"", "", tr("Reset")},
                                  {statusStartStop, "", tr("Reset CNC")});
        m_runWidget->setButtonsEnabled(0,0,1,1);
        m_btnHome->setEnabled(false);
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
            m_runWidget->btnStart->setEnabled(false);
            QString tmp = m_runWidget->btnStart->text();
            m_runWidget->btnStart->setText(tr("Loading"));
            m_runWidget->btnStart->update();
            m_runWidget->btnStart->repaint();

            par.saveGcode();
//            par.cnc.stateClear();

            stopCncReader();
            qDebug("Load G-code. Start");
            bool OK = loadGcode();
            qDebug("Load G-code. End");
            startCncReader();

            m_runWidget->btnStart->setEnabled(true);
            m_runWidget->btnStart->setText(tmp);
            m_runWidget->btnStart->update();

            if (OK) {
//                m_report.append(tr("G-code is loaded"));
                qDebug("form_run::on_btnStart_clicked>>G-code is loaded");

                m_info.chop(1); // clear \n
                m_info += ". " + tr("Loaded") + "\n";
                m_runWidget->txtMsg->setText(m_info);

                par.cnc.imitEna(false);

                if (m_runWidget->checkIdle->isChecked() || (m_btnBreak->isChecked() && m_btnPump->isChecked() && m_btnRoll->isChecked())) {
                    if (!m_runWidget->checkIdle->isChecked())
                        par.cnc.writeHoldEnable(true);

                    par.cnc.runReq();
                    if (m_timer) {
                        m_timer->clear();
                        m_timer->start();
                    }
                }
            } else {
                qDebug("form_run::on_btnStart_clicked>>Load G-code error");
                m_info.chop(1); // clear \n
                m_info += " " + tr("Load G-code error") + "\n";
                m_runWidget->txtMsg->setText(m_info);
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
        if (m_timer)
            m_timer->stop();
        break;

    case AppState::STATES::ST_PAUSE:
        if (m_runWidget->checkIdle->isChecked() || (m_btnBreak->isChecked() && m_btnPump->isChecked() && m_btnRoll->isChecked())) {
            if (!m_runWidget->checkIdle->isChecked())
                par.cnc.writeHoldEnable(true);

            par.cnc.runReq();
            if (m_timer)
                m_timer->start();
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
        if (!m_runWidget->checkIdle->isChecked())
            par.cnc.writeHoldEnable(true);

        par.cnc.revReq();
        if (m_timer)
            m_timer->start();

        par.appState.next(AppState::BUTTON::SIG_REVERSE);
        updateButtons();
    } catch (...) {}

    blockSignals(false);
}

void FormRun::on_btnCancel_clicked() {
    blockSignals(true);

    try {
        par.saveCncContext();
        int roll_vel = m_numRollVel->value();

        par.cnc.cancelReq();
        par.cnc.writeRollVel(roll_vel);

        par.appState.next(AppState::BUTTON::SIG_CANCEL);
        updateButtons();
    } catch (...) {}

    blockSignals(false);    
}

void FormRun::setCursor(int row) {
    m_currentCursorPosition = row;

    QTextCursor textCursor = m_runWidget->txtCode->textCursor();

    textCursor.movePosition(QTextCursor::Start);
    textCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, m_currentCursorPosition);

    m_runWidget->txtCode->setTextCursor(textCursor);
    m_runWidget->txtCode->update();
}

void FormRun::setCursorToBegin() {
    QTextCursor textCursor = m_runWidget->txtCode->textCursor();
    textCursor.movePosition(QTextCursor::Start);

    m_currentCursorPosition = textCursor.position();

    m_runWidget->txtCode->setTextCursor(textCursor);
    m_runWidget->txtCode->update();
}

void FormRun::setCursorToEnd() {
    QTextCursor textCursor = m_runWidget->txtCode->textCursor();
    textCursor.movePosition(QTextCursor::End);

    m_currentCursorPosition = textCursor.position();

    m_runWidget->txtCode->setTextCursor(textCursor);
    m_runWidget->txtCode->update();
}

void FormRun::startCncReader() {
    if (par.cnc.isOpen()) {
        qDebug()<<"CNC reader: Start";
        m_cncReaderEna = true;
        m_remain_tmr = 0;
        readCncContext();
    }
}

void FormRun::stopCncReader() {
    qDebug()<<"CNC reader: Stop";
    m_cncReaderEna = false;
    m_remain_tmr = 0;
    m_timer->clear();
}

void FormRun::readCncContext() {
    if (m_cncReaderEna) {
        qDebug("Read context. Start");

        blockSignals(true);

        try {
            par.cncContext = par.cnc.readCncContext();
            const CncContext& ctx = par.cncContext;
#ifndef STONE
            m_runWidget->txtMsg->setText(m_info + QString( ctx.toStringRunDebug().c_str() ));
#else
            m_runWidget->txtMsg->setText(m_info + QString( ctx.toStringRunStoneDebug().c_str() ));
#endif
//            qDebug() << "CNC reader: " + QString(ctx.toString().c_str());

            m_runWidget->setPositionLabels(0, ctx.x());
            m_runWidget->setPositionLabels(1, ctx.y());
            m_runWidget->setPositionLabels(2, ctx.u());
            m_runWidget->setPositionLabels(3, ctx.v());

            m_runWidget->setEncoderLabels(0, ctx.encoderX());
            m_runWidget->setEncoderLabels(1, ctx.encoderY());

            qint64 ms = m_timer ? m_timer->elapsed() : 0;
            m_runWidget->setElapsedTime(ms);

            m_runWidget->setLimitSwitches(ctx.limitSwitches());

            m_btnRoll->setChecked(ctx.isRollEnable());
            m_btnPump->setChecked(ctx.pumpEnabled());
            m_btnBreak->setChecked(ctx.wireControlEnabled());
            m_btnHighVolt->setChecked(ctx.highVoltageEnabled());

            m_runWidget->btnHold->setChecked(ctx.hold());

            if (ctx.rollVelocity() < m_numRollVel->minimum()) {
                m_numRollVel->setValue(m_numRollVel->minimum());
//                qDebug("Error: MIN CNC Roll Vel: %x", static_cast<unsigned>(ctx.rollVelocity()));
            }
            else if (ctx.rollVelocity() > m_numRollVel->maximum()) {
                m_numRollVel->setValue(m_numRollVel->maximum());
                qDebug("Error: MAX CNC Roll Vel: %x", static_cast<unsigned>(ctx.rollVelocity()));
            }
            else
                m_numRollVel->setValue(ctx.rollVelocity());

            if (ctx.pulseWidth() < m_numWidth->minimum()) {
                m_numWidth->setValue(m_numWidth->minimum());
                qDebug("Error: MIN CNC Pulse Width: %x", static_cast<unsigned>(ctx.pulseWidth()));
            }
            else if (ctx.pulseWidth() > m_numWidth->maximum()) {
                m_numWidth->setValue(m_numWidth->maximum());
                qDebug("Error: MAX CNC Pulse Width: %x", static_cast<unsigned>(ctx.pulseWidth()));
            }
            else
                m_numWidth->setValue(ctx.pulseWidth());

            if (ctx.pulseRatio() < m_numRatio->minimum()) {
                m_numRatio->setValue(m_numRatio->minimum());
                qDebug("Error: MIN CNC Pulse Ratio: %x", static_cast<unsigned>(ctx.pulseRatio()));
            }
            else if (ctx.pulseRatio() > m_numRatio->maximum()) {
                m_numRatio->setValue(m_numRatio->maximum());
                qDebug("Error: MAX CNC Pulse Ratio: %x", static_cast<unsigned>(ctx.pulseRatio()));
            }
            else
                m_numRatio->setValue(ctx.pulseRatio());

#ifndef STONE
            m_btnLowHighVolt->setChecked(ctx.lowHighVoltageEnabled());
#endif

            if (ctx.currentIndex() < m_numCurrent->minimum()) {
                m_numCurrent->setValue(m_numCurrent->minimum());
                qDebug("Error: MIN CNC Pulse Ratio: %x", static_cast<unsigned>(ctx.currentIndex()));
            }
            else if (ctx.currentIndex() > m_numCurrent->maximum()) {
                m_numCurrent->setValue(m_numCurrent->maximum());
                qDebug("Error: MAX CNC Pulse Ratio: %x", static_cast<unsigned>(ctx.currentIndex()));
            }
            else
                m_numCurrent->setValue(ctx.currentIndex());

            m_speed.setMMM(ctx.speed());

            if (m_speed.get() < m_speed.min()) {
                m_runWidget->numSpeed->setValue(m_speed.min());
                qDebug("Error: MIN CNC Speed: %f", ctx.speed());
            }
            else if (m_speed.get() > m_speed.max()) {
                m_runWidget->numSpeed->setValue(m_speed.max());
                qDebug("Error: MAX CNC Speed: %f", ctx.speed());
            }
            else
                m_runWidget->numSpeed->setValue(m_speed.get());

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

                if (m_remain_tmr >= REMAIN_TIMER_MAX) {
                    m_remain_tmr = 0;

                    if (!ctx.reverse()) {
                        double length = par.workContours.botLength();
                        double speed = ms > 0 ? length / (double)ms : 0;
                        double remain_length = m_full_length > 0 ? m_full_length - length : 0;
                        double remain_time = remain_length / speed;

                        if (remain_time < 0)
                            remain_time = 0;

                        m_runWidget->setRemainTime(static_cast<qint64>(remain_time));

                        qDebug("Remain: %0.3f mm, %0.3f mm, %d ms, %d ms", m_full_length, length, (int)ms, (int)remain_time);
                    } else {
                        m_runWidget->setRemainTime(0);
                    }
                }
                else
                    m_remain_tmr++;
            }

            if (par.appState.isError() || ctx.isError() || m_cutStateAbortReq) {
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

                if (m_cutStateAbortReq) {
                    qDebug("CutState: Abort request");
                    m_cutStateAbortReq = false;
                }

                if (m_timer)
                    m_timer->stop();
            }
            else if (par.appState.isWork() && !ctx.isWork()) {
                par.appState.reset();
                updateButtons();                
                setCursorToEnd();

                if (m_timer)
                    m_timer->stop();

                m_runWidget->setRemainTime(0);
            }
            else {
                par.appState.update(ctx);
                updateButtons();
            }
        }
        catch (...) {}

        qDebug("Read context. End");

        blockSignals(false);
        QTimer::singleShot(POLLING_TIME, this, &FormRun::readCncContext);
    } else {
        qDebug("Read context. Skip");
    }
}

void FormRun::startAdc() { // on init
    if (par.cnc.isOpen()) {
        qDebug()<<"ADC: Start";
        m_adcEnable = true;
        readAdc();
    }
    else
        m_adcEnable = false;
}

void FormRun::stopAdc() { // at to home
    qDebug()<<"ADC: Stop";
    m_adcEnable = false;
}

void FormRun::readAdc() {
    if (m_adcEnable) {
        cnc_adc_volt_t adc = par.cnc.readADCVolt();
        m_runWidget->setAdc(adc);
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

                const double& k = par.gcodeSettings.bot_coe;
                fpoint_t dXY = fpoint_t(k * top_pos.x, k * top_pos.y);

                par.workContours.setBotPos(bot_pos + dXY);
            }
            else {
                par.workContours.setBotPos(bot_pos);
            }
        }
        else
            par.workContours.clearSelected();

        m_runWidget->plot(par.workContours, par.swapXY, par.reverseX, par.reverseY, par.showXY);
    }
}
