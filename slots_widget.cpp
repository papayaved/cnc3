#include "slots_widget.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QMessageBox>

#include "contour_pass.h"
#include "wire_cut.h"
#include "aux_items.h"

SlotsWidget::SlotsWidget(ProgramParam& par, QWidget *parent) : QWidget(parent), par(par) {
    labelTitle = new QLabel(R"(<h1>)" + tr("Slots") + R"(<br></h1>)");
    labelWireD = new QLabel(tr("Wire Diameter"));
    labelDepth = new QLabel(tr("Slots Depth") + ", " + tr("signed value"));
    labelStep = new QLabel(tr("Step") + ", " + tr("signed value"));
    labelSlotsNum = new QLabel(tr("Slots Number"));
    labelSpeed = new QLabel(tr("Cutting Speed"));
    labelRollVel = new QLabel(tr("Roll Velocity"));
    labelIdleSpeed = new QLabel(tr("Idle Speed"));
    labelIdleRollVel = new QLabel(tr("Idle Roll Velocity"));

    fnumWireD = new QDoubleSpinBox;
    labelWireD->setBuddy(fnumWireD);
    fnumWireD->setSuffix(" " + tr("mm"));
    fnumWireD->setDecimals(3);
    fnumWireD->setSingleStep(0.001);
    fnumWireD->setRange(0, 3);    
    fnumWireD->setAccelerated(true);
    fnumWireD->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    fnumDepth = new QDoubleSpinBox;
    labelDepth->setBuddy(fnumDepth);
    fnumDepth->setSuffix(" " + tr("mm"));
    fnumDepth->setDecimals(3);
    fnumDepth->setSingleStep(0.001);
    fnumDepth->setRange(-1000, 1000);
    fnumDepth->setAccelerated(true);
    fnumDepth->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    fnumStep = new QDoubleSpinBox;
    labelStep->setBuddy(fnumStep);
    fnumStep->setSuffix(" " + tr("mm"));
    fnumStep->setDecimals(3);
    fnumStep->setSingleStep(0.001);
    fnumStep->setRange(-1000, 1000);
    fnumStep->setAccelerated(true);
    fnumStep->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    numSlotsNum = new QSpinBox;
    labelSlotsNum->setBuddy(numSlotsNum);
    numSlotsNum->setRange(1, 1000);
    numSlotsNum->setAccelerated(true);
    numSlotsNum->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    fnumSpeed = new QDoubleSpinBox;
    labelSpeed->setBuddy(fnumSpeed);
    fnumSpeed->setSuffix(" " + tr("mm/min"));
    fnumSpeed->setDecimals(1);
    fnumSpeed->setSingleStep(0.1);
    fnumSpeed->setRange(0.1, cnc_param::SPEED_MAX);
    fnumSpeed->setAccelerated(true);
    fnumSpeed->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    fnumIdleSpeed = new QDoubleSpinBox;
    labelIdleSpeed->setBuddy(fnumIdleSpeed);
    fnumIdleSpeed->setSuffix(" " + tr("mm/min"));
    fnumIdleSpeed->setDecimals(1);
    fnumIdleSpeed->setSingleStep(0.1);
    fnumIdleSpeed->setRange(0.1, cnc_param::SPEED_MAX);
    fnumIdleSpeed->setAccelerated(true);
    fnumIdleSpeed->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    numRollVel = new QSpinBox;
    labelRollVel->setBuddy(numRollVel);
    numRollVel->setRange(cnc_param::ROLL_MIN, cnc_param::ROLL_MAX);

    numIdleRollVel = new QSpinBox;
    labelIdleRollVel->setBuddy(numIdleRollVel);
    numIdleRollVel->setRange(cnc_param::ROLL_MIN, cnc_param::ROLL_MAX);

    checkPause = new QCheckBox(tr("Pause"));
    numPause = new QSpinBox;
    numPause->setSuffix(" " + tr("sec"));
    numPause->setRange(1, 600);
    numPause->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    groupAxis = new QGroupBox(tr("Axis"));
    radioX = new QRadioButton("X");
    radioY = new QRadioButton("Y");
    {
        QHBoxLayout* hbox = new QHBoxLayout;
        hbox->addWidget(radioX);
        hbox->addWidget(radioY);
        groupAxis->setLayout(hbox);
    }

    //
    btnUndo = new QPushButton(tr("Undo"));
    btnUndo->setStatusTip(tr("Return last working values"));

    btnReset = new QPushButton(tr("Reset"));
    btnReset->setStatusTip(tr("Set default values"));

    btnGen = new QPushButton(tr("Generate"));
    btnGen->setStatusTip(tr("Generate contour"));

    //
    QGridLayout* grid = new QGridLayout;    
    grid->addWidget(labelTitle, 0, 0, 1, 2, Qt::AlignHCenter | Qt::AlignBottom);

    grid->addWidget(labelWireD, 1, 0);
    grid->addWidget(fnumWireD, 1, 1);

    grid->addWidget(labelDepth, 2, 0);
    grid->addWidget(fnumDepth, 2, 1);

    grid->addWidget(labelStep, 3, 0);
    grid->addWidget(fnumStep, 3, 1);

    grid->addWidget(labelSlotsNum, 4, 0);
    grid->addWidget(numSlotsNum, 4, 1);

    grid->addWidget(labelSpeed, 5, 0);
    grid->addWidget(fnumSpeed, 5, 1);

    grid->addWidget(labelRollVel, 6, 0);
    grid->addWidget(numRollVel, 6, 1);

    grid->addWidget(labelIdleSpeed, 7, 0);
    grid->addWidget(fnumIdleSpeed, 7, 1);

    grid->addWidget(labelIdleRollVel, 8, 0);
    grid->addWidget(numIdleRollVel, 8, 1);

    grid->addWidget(checkPause, 9, 0);
    grid->addWidget(numPause, 9, 1);

    grid->addWidget(groupAxis, 10, 1);

    grid->addWidget(btnReset, 11, 0, Qt::AlignLeft | Qt::AlignBottom);
    grid->addWidget(btnGen, 11, 1, Qt::AlignRight | Qt::AlignBottom);

    grid->addWidget(btnUndo, 12, 0, Qt::AlignLeft | Qt::AlignBottom);

    QGridLayout* gridCentral = new QGridLayout;
    gridCentral->addLayout(grid, 0, 0, Qt::AlignTop | Qt::AlignLeft);
    gridCentral->addWidget(new QFrame, 0, 1);
    gridCentral->addWidget(new QFrame, 1, 0, 1, 2);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addLayout(gridCentral);
    this->setLayout(mainLayout);

    connect(btnUndo, &QPushButton::clicked, this, &SlotsWidget::onUndo);
    connect(btnReset, &QPushButton::clicked, this, &SlotsWidget::onReset);
    connect(btnGen, &QPushButton::clicked, this, &SlotsWidget::onGenerate);

    connect(checkPause, &QCheckBox::clicked, this, [&](bool checked) {
        numPause->setEnabled(checked);
    });

    onUndo();

    widgets = {
        labelTitle, labelWireD, labelDepth, labelStep, labelSlotsNum, labelSpeed, labelIdleSpeed, labelRollVel, labelIdleRollVel,
        fnumWireD, fnumStep, fnumDepth, fnumSpeed, fnumIdleSpeed,
        numSlotsNum, numRollVel, numIdleRollVel, numPause,
        checkPause,
        groupAxis,
        radioX, radioY,
        btnReset, btnUndo, btnGen
    };
}

void SlotsWidget::init() {
    fnumWireD->setValue(par.m_comb.wireD);
    fnumDepth->setValue(par.m_comb.depth);
    fnumStep->setValue(par.m_comb.step);
    numSlotsNum->setValue(par.m_comb.slots_num);

    fnumSpeed->setValue(par.m_comb.speed);
    fnumIdleSpeed->setValue(par.m_comb.speed_idle);

    numRollVel->setValue(par.m_comb.roll_vel);
    numIdleRollVel->setValue(par.m_comb.roll_vel_idle);

    checkPause->setChecked(par.m_comb.pause_ena);
    numPause->setEnabled(par.m_comb.pause_ena);
    numPause->setValue(par.m_comb.pause);

    if (par.m_comb.axis == AXIS::AXIS_Y)
        radioY->click();
    else
        radioX->click();
}

// slots is a reserved name
comb_t SlotsWidget::comb() const {
    comb_t res;
    res.slots_num       = numSlotsNum->value();

    res.roll_vel        = numRollVel->value();
    res.roll_vel_idle   = numIdleRollVel->value();

    res.wireD       = fnumWireD->value();
    res.depth       = fnumDepth->value();
    res.step        = fnumStep->value();

    res.speed       = fnumSpeed->value();
    res.speed_idle  = fnumIdleSpeed->value();

    res.pause_ena   = checkPause->isChecked();
    res.pause       = numPause->value();

    res.axis        = radioY->isChecked() ? AXIS::AXIS_Y : AXIS::AXIS_X;

    return res;
}

void SlotsWidget::onUndo() {
    comb_t comb;
    par.loadComb(comb);
    init();
}

void SlotsWidget::onReset() {
    par.m_comb = comb_t();
    init();
}

void SlotsWidget::onGenerate() {
    btnGen->blockSignals(true);
    btnGen->setEnabled(false);

    try {
        Dxf bot;
        par.saveComb( comb() );
        init();

        bot.generate(par.m_comb);

        par.contours.clear();
        par.contours.new_back();
        par.contours.back()->setBot(bot);

        ContourPair* pair = par.contours.front();
        std::deque<ContourPair> ar;
        ar.push_back(*pair);

        if (par.gcode.generate(par.cutParam, ar, CncParam()) ) {
            par.gcodeText = par.gcode.toText().c_str();
            par.gcodeSettings = par.gcode.getSettings();

            if (!par.gcode.empty()) {
                par.cutParam = cut_t();
                par.cutParam.offsets[0].offset = 0;
                par.cutParam.offset_ena = false;

                emit clicked();
            }
        }
    } catch(const std::exception& ex) {
        qDebug("CombWidget::onGenerate>>%s", ex.what());
        QMessageBox::critical(this, tr("Slots generate error"), ex.what());
    } catch(...) {
        qDebug("CombWidget::onGenerate>>%s", std::exception().what());
        QMessageBox::critical(this, tr("Slots generate error"), std::exception().what());
    }

    btnGen->setEnabled(true);
    btnGen->blockSignals(false);
}

void SlotsWidget::setFontPointSize(QWidget *w, int pointSize) {
    QFont font = w->font();
    font.setPointSize(pointSize);
    w->setFont(font);
}

void SlotsWidget::setFontPointSize(int pointSize) {
    for (QWidget* w: widgets)
        setFontPointSize(w, pointSize);
}
