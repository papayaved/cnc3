#include "form_settings.h"
#include <cstdint>
#include <QToolTip>
#include <QScrollArea>

#include "program_param.h"

// todo: message view
FormSettings::FormSettings(ProgramParam& par, QWidget *parent) : QWidget(parent), par(par) {
    this->setObjectName(tr("CNC Settings"));
    createSettingsWidget();
    createButtons();

    QScrollArea* scrollArea = new QScrollArea;
    scrollArea->setWidget(widgetSettings);
    scrollArea->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(scrollArea, Qt::AlignTop | Qt::AlignHCenter);
    mainLayout->addLayout(gridButtons);

    this->setLayout(mainLayout);
}

QGroupBox* FormSettings::groupLabelNum(QLabel* label, QDoubleSpinBox* num) {
    QGroupBox* group = new QGroupBox;
    label->setBuddy(num);
    group->setLayout(new QHBoxLayout);
    group->layout()->addWidget(label);
    group->layout()->addWidget(num);
    return group;
}

QGroupBox* FormSettings::groupLabelNum(QLabel* label, QSpinBox* num, QComboBox* combo) {
    QGroupBox* group = new QGroupBox;
    label->setBuddy(num);
    group->setLayout(new QHBoxLayout);
    group->layout()->addWidget(label);
    group->layout()->addWidget(num);
    group->layout()->addWidget(combo);
    return group;
}

QGroupBox* FormSettings::groupLabelCombo(QLabel* label, QComboBox* combo) {
    QGroupBox* group = new QGroupBox;
    label->setBuddy(combo);
    group->setLayout(new QHBoxLayout);
    group->layout()->addWidget(label);
    group->layout()->addWidget(combo);
    return group;
}

void FormSettings::createSettingsWidget() {
    widgetSettings = new QWidget;
    gridSettings = new QGridLayout;

    labelTitle = new QLabel("<h3>" + tr("Application settings") + "</h3>");
    labelCNC = new QLabel("<h3>" + tr("CNC parameters") + "</h3>");
//    labelTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    labelLanguage = new QLabel("Language: ");

    comboLanguage = new QComboBox;
    comboLanguage->addItem("English");
    comboLanguage->addItem("Русский");

//    checkReverseX = new QCheckBox(tr("Reverse plot axis X"));
//    checkReverseY = new QCheckBox(tr("Reverse plot axis Y"));
//    checkSwapXY = new QCheckBox(tr("Swap plot axes X, Y"));
//    checkShowXY = new QCheckBox(tr("Show plot axes names"));

    labelInputLevel = new QLabel(tr("Input Levels, bits") + ": ");
    numInputLevel = new QSpinBox;
    numInputLevel->setDisplayIntegerBase(16);
    numInputLevel->setPrefix("0x");
    numInputLevel->setRange(0, INT32_MAX);

    comboInputLevel = new QComboBox;
    comboInputLevel->addItem(tr(""));
    comboInputLevel->addItem(tr("Metal"));
    comboInputLevel->addItem(tr("Stone"));
    comboInputLevel->addItem(tr("Debug"));

    checkReverseMotorX = new QCheckBox(tr("Reverse motor X"));
    checkReverseMotorY = new QCheckBox(tr("Reverse motor Y"));
    checkSwapMotorXY = new QCheckBox(tr("Swap motors X and Y"));

#ifndef STONE
    checkStepDirEnable = new QCheckBox(tr("Use servomotors"));    

    checkReverseMotorU = new QCheckBox(tr("Reverse motor U"));
    checkReverseMotorV = new QCheckBox(tr("Reverse motor V"));    
    checkSwapMotorUV = new QCheckBox(tr("Swap motors U and V"));

    //
    QFrame* hLine[2];
    for (size_t i = 0; i < sizeof(hLine)/sizeof(hLine[0]); i++) {
        hLine[i] = new QFrame;
        hLine[i]->setFrameShape(QFrame::HLine);
        hLine[i]->setFrameShadow(QFrame::Sunken);
        hLine[i]->setLineWidth(3);
    }

    groupEnc = new QGroupBox(tr("Use linear encoders for X, Y axes"));
    groupEnc->setCheckable(true);

    checkReverseEncX = new QCheckBox(tr("Reverse Encoder X"));
    checkReverseEncY = new QCheckBox(tr("Reverse Encoder Y"));

    QGridLayout* gridEnc = new QGridLayout;
    gridEnc->addWidget(checkReverseEncX, 0, 0);
    gridEnc->addWidget(checkReverseEncY, 1, 0);

    groupEnc->setLayout(gridEnc);

    //
    labelAcc = new QLabel(tr("Acceleration") + ":");
    fnumAcc = new QDoubleSpinBox;
    fnumAcc->setRange(1, 1000);
    fnumAcc->setSuffix(" " + tr("um/sec2")); // per 100 V
    fnumAcc->setDecimals(0);
    fnumAcc->setSingleStep(10);

    labelDec = new QLabel(tr("Deceleration") + ":");
    fnumDec = new QDoubleSpinBox;
    fnumDec->setRange(1, 1000);
    fnumDec->setSuffix(" " + tr("um/sec2")); // per 100 V
    fnumDec->setDecimals(0);
    fnumDec->setSingleStep(10);

    // Feedback
    groupFeedback = new QGroupBox(tr("Feedback enable"));
    groupFeedback->setCheckable(true);

    int thld_max = static_cast<int>( round(cnc_adc_volt_t::maxVolt(0)) );

    labelHighThld = new QLabel(tr("High threshold (HV)"));
    numHighThld = new QSpinBox;
    labelHighThld->setBuddy(numHighThld);
    numHighThld->setRange(0, thld_max);
    numHighThld->setSuffix(" " + tr("V"));

    labelLowThld = new QLabel(tr("Low threshold (HV)"));
    numLowThld = new QSpinBox;
    labelLowThld->setBuddy(numLowThld);
    numLowThld->setRange(0, thld_max);
    numLowThld->setSuffix(" " + tr("V"));

    labelHighThld2 = new QLabel(tr("High threshold (Low HV)"));
    numHighThld2 = new QSpinBox;
    labelHighThld2->setBuddy(numHighThld2);
    numHighThld2->setRange(0, thld_max);
    numHighThld2->setSuffix(" " + tr("V"));

    labelLowThld2 = new QLabel(tr("Low threshold (Low HV)"));
    numLowThld2 = new QSpinBox;
    labelLowThld2->setBuddy(numLowThld2);
    numLowThld2->setRange(0, thld_max);
    numLowThld2->setSuffix(" " + tr("V"));

    labelRbTimeout = new QLabel(tr("Rollback timeout"));
    numRbTimeout = new QSpinBox;
    labelRbTimeout->setBuddy(numRbTimeout);
    numRbTimeout->setRange(10, 60);
    numRbTimeout->setSuffix(" " + tr("sec"));

    labelRbLength = new QLabel(tr("Rollback distance"));
    fnumRbLength = new QDoubleSpinBox;
    labelRbLength->setBuddy(fnumRbLength);
    fnumRbLength->setRange(0.100, 0.500);
    fnumRbLength->setSuffix(" " + tr("mm"));
    fnumRbLength->setDecimals(3);
    fnumRbLength->setSingleStep(0.01);

    labelRbSpeed = new QLabel(tr("Rollback speed"));
    fnumRbSpeed = new QDoubleSpinBox;
    labelRbSpeed->setBuddy(fnumRbSpeed);
    fnumRbSpeed->setRange(0.1, 2);
    fnumRbSpeed->setSuffix(" " + tr("mm/min"));
    fnumRbSpeed->setDecimals(1);
    fnumRbSpeed->setSingleStep(0.1);

    labelRbAttempts = new QLabel(tr("Rollback attempts"));
    numRbAttempts = new QSpinBox;
    labelRbAttempts->setBuddy(numRbAttempts);
    numRbAttempts->setRange(1, 5);

    QGridLayout* gridFeedback = new QGridLayout;
    gridFeedback->addWidget(labelHighThld, 0, 0);
    gridFeedback->addWidget(numHighThld, 0, 1);
    gridFeedback->addWidget(labelLowThld, 1, 0);
    gridFeedback->addWidget(numLowThld, 1, 1);

    gridFeedback->addWidget(labelHighThld2, 2, 0);
    gridFeedback->addWidget(numHighThld2, 2, 1);
    gridFeedback->addWidget(labelLowThld2, 3, 0);
    gridFeedback->addWidget(numLowThld2, 3, 1);

    gridFeedback->addWidget(labelRbTimeout, 4, 0);
    gridFeedback->addWidget(numRbTimeout, 4, 1);
    gridFeedback->addWidget(labelRbLength, 5, 0);
    gridFeedback->addWidget(fnumRbLength, 5, 1);
    gridFeedback->addWidget(labelRbSpeed, 6, 0);
    gridFeedback->addWidget(fnumRbSpeed, 6, 1);
    gridFeedback->addWidget(labelRbAttempts, 7, 0);
    gridFeedback->addWidget(numRbAttempts, 7, 1);

    groupFeedback->setLayout(gridFeedback);
#endif

    //    
    fnumStep = new QDoubleSpinBox;
    labelStep = new QLabel(tr("Calculation step") + ": ");
    labelStep->setBuddy(fnumStep);
    fnumStep->setRange(0.001, 1.0);
    // todo: bind range with scale
    fnumStep->setSingleStep(0.001);
    fnumStep->setDecimals(3);
    fnumStep->setSuffix(" " + tr("mm"));
#ifdef STONE
    fnumStep->setEnabled(false);
#endif

    labelX = new QLabel("X: ");
    labelY = new QLabel("Y: ");

    labelPrecision = new QLabel(tr("Precision (steps/mm)"));
    labelMotor = new QLabel(tr("Motor"));

    fnumScaleX = new QDoubleSpinBox;
    labelX->setBuddy(fnumScaleX);
    fnumScaleX->setRange(1, 1000000);
    fnumScaleX->setDecimals(0);
#ifdef STONE
    fnumScaleX->setEnabled(false);
#endif

    fnumScaleY = new QDoubleSpinBox;
    labelY->setBuddy(fnumScaleY);
    fnumScaleY->setRange(1, 1000000);
    fnumScaleY->setDecimals(0);
#ifdef STONE
    fnumScaleY->setEnabled(false);
#endif

#ifndef STONE
    labelU = new QLabel("U: ");
    labelV = new QLabel("V: ");
    labelEncX = new QLabel("X: ");
    labelEncY = new QLabel("Y: ");

    labelEncoder = new QLabel(tr("Encoder"));

    fnumScaleU = new QDoubleSpinBox;
    labelU->setBuddy(fnumScaleU);
    fnumScaleU->setRange(1, 1000000);
    fnumScaleU->setDecimals(0);

    fnumScaleV = new QDoubleSpinBox;
    labelV->setBuddy(fnumScaleV);
    fnumScaleV->setRange(1, 1000000);
    fnumScaleV->setDecimals(0);

    fnumScaleEncX = new QDoubleSpinBox;
    labelEncX->setBuddy(fnumScaleEncX);
    fnumScaleEncX->setRange(1, 1000);
    fnumScaleEncX->setDecimals(0);

    fnumScaleEncY = new QDoubleSpinBox;
    labelEncY->setBuddy(fnumScaleEncY);
    fnumScaleEncY->setRange(1, 1000);
    fnumScaleEncY->setDecimals(0);
#endif

    checks = {
//        checkShowXY,
        checkReverseMotorX, checkReverseMotorY, checkSwapMotorXY,
#ifndef STONE
        checkStepDirEnable,
        checkReverseMotorU, checkReverseMotorV, checkSwapMotorUV,
        checkReverseEncX, checkReverseEncY
#endif
    };

#ifndef STONE
    scaleNums = {fnumScaleX, fnumScaleY, fnumScaleU, fnumScaleV, fnumScaleEncX, fnumScaleEncY};
#else
    scaleNums = {fnumScaleX, fnumScaleY};
#endif

    init();
    setFontPointSize(16);

    gridSettings = new QGridLayout;

    gridSettings->addWidget(labelTitle, 0, 1, 1, 4, Qt::AlignHCenter | Qt::AlignBottom);
    gridSettings->addWidget(new QFrame, 1, 0, 1, 4);

    gridSettings->addWidget(groupLabelCombo(labelLanguage, comboLanguage), 2, 1, 1, 2);
//    gridSettings->addWidget(checkReverseX, 3, 1, 1, 4);
//    gridSettings->addWidget(checkReverseY, 4, 1, 1, 4);
//    gridSettings->addWidget(checkSwapXY, 5, 1, 1, 4);
//    gridSettings->addWidget(checkShowXY, 6, 1, 1, 4);

    gridSettings->addWidget(new QFrame, 7, 0, 1, 4);
    gridSettings->addWidget(labelCNC, 8, 1, 1, 4, Qt::AlignHCenter | Qt::AlignBottom);

    gridSettings->addWidget(groupLabelNum(labelInputLevel, numInputLevel, comboInputLevel), 9, 1, 1, 3);

    gridSettings->addWidget(groupLabelNum(labelStep, fnumStep), 10, 1, 1, 2);

#ifndef STONE
    gridSettings->addWidget(checkStepDirEnable, 11, 1, 1, 4);
#endif

    gridSettings->addWidget(checkReverseMotorX, 12, 1, 1, 4);
    gridSettings->addWidget(checkReverseMotorY, 13, 1, 1, 4);

#ifndef STONE
    gridSettings->addWidget(checkReverseMotorU, 14, 1, 1, 4);
    gridSettings->addWidget(checkReverseMotorV, 15, 1, 1, 4);
#endif

    gridSettings->addWidget(checkSwapMotorXY, 16, 1, 1, 4);

#ifndef STONE
    gridSettings->addWidget(checkSwapMotorUV, 17, 1, 1, 4);

    gridSettings->addWidget(hLine[0], 18, 1, 1, 4);
    gridSettings->addWidget(groupEnc, 19, 1, 1, 2);

    gridSettings->addWidget(hLine[1], 20, 1, 1, 4);
    gridSettings->addWidget(groupFeedback, 21, 1, 1, 2);

    gridSettings->addWidget(groupLabelNum(labelAcc, fnumAcc), 22, 1, 1, 2);
    gridSettings->addWidget(groupLabelNum(labelDec, fnumDec), 23, 1, 1, 2);

    gridSettings->addWidget(labelPrecision, 24, 1, 1, 4, Qt::AlignHCenter | Qt::AlignBottom);

    gridSettings->addWidget(labelMotor, 25, 0);
    gridSettings->addWidget(groupLabelNum(labelX, fnumScaleX), 25, 1, 1, 2);
    gridSettings->addWidget(groupLabelNum(labelY, fnumScaleY), 25, 3, 1, 2);

    gridSettings->addWidget(groupLabelNum(labelU, fnumScaleU), 26, 1, 1, 2);
    gridSettings->addWidget(groupLabelNum(labelV, fnumScaleV), 26, 3, 1, 2);

    gridSettings->addWidget(labelEncoder, 27, 0);
    gridSettings->addWidget(groupLabelNum(labelEncX, fnumScaleEncX), 27, 1, 1, 2);
    gridSettings->addWidget(groupLabelNum(labelEncY, fnumScaleEncY), 27, 3, 1, 2);
#endif

//    gridSettings->addWidget(new QFrame, 0, 3, 8, 1);
//    gridSettings->addWidget(new QFrame, 8, 0, 1, 4);

//    gridSettings->setSizeConstraint(QLayout::SetFixedSize);
    QWidget* widgetInside = new QWidget(this);
    widgetInside->setLayout(gridSettings);

    QGridLayout* grid = new QGridLayout;
    grid->addWidget(new QFrame, 0, 0);
    grid->addWidget(widgetInside, 0, 1);
    grid->addWidget(new QFrame, 0, 2);
    grid->addWidget(new QFrame, 1, 0, 1, 3);

    widgetSettings->setLayout(grid);

    //
    connect(comboLanguage, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int i) {
        switch (i) {
        case int(InterfaceLanguage::RUSSIAN):
            ProgramParam::saveInterfaceLanguage(InterfaceLanguage::RUSSIAN);
            break;
        default:
            ProgramParam::saveInterfaceLanguage(InterfaceLanguage::ENGLISH);
            break;
        }

        emit showWarning("Interface language will be changed after reboot");
    });

//    connect(checkSwapXY, &QCheckBox::stateChanged, [=](int state) {
//        ProgramParam::saveSwapXY(state == Qt::Checked);
//    });

//    connect(checkReverseX, &QCheckBox::stateChanged, [=](int state) {
//        ProgramParam::saveReverseX(state == Qt::Checked);
//    });

//    connect(checkReverseY, &QCheckBox::stateChanged, [=](int state) {
//        ProgramParam::saveReverseY(state == Qt::Checked);
//    });

//    connect(checkShowXY, &QCheckBox::stateChanged, [=](int state) {
//        ProgramParam::saveShowXY(state == Qt::Checked);
//    });

    connect(comboInputLevel, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int i) {
        numInputLevel->blockSignals(true);

        switch (i) {
        case 1: numInputLevel->setValue(CncParam::INPUT_LEVEL_METAL); break;
        case 2: numInputLevel->setValue(CncParam::INPUT_LEVEL_STONE); break;
        case 3: numInputLevel->setValue(CncParam::INPUT_LEVEL_DEBUG); break;
        }

        numInputLevel->blockSignals(false);
    });

    connect(numInputLevel, QOverload<int>::of(&QSpinBox::valueChanged), this, [&](int /*value*/) {
        selectComboInputLevel(numInputLevel->value());
    });

    connect(checkReverseMotorX, &QCheckBox::stateChanged, this, &FormSettings::onStateChangedMotor);
    connect(checkReverseMotorY, &QCheckBox::stateChanged, this, &FormSettings::onStateChangedMotor);
    connect(checkSwapMotorXY, &QCheckBox::stateChanged, this, &FormSettings::onStateChangedMotor);

#ifndef STONE
    connect(checkReverseMotorU, &QCheckBox::stateChanged, this, &FormSettings::onStateChangedMotor);
    connect(checkReverseMotorV, &QCheckBox::stateChanged, this, &FormSettings::onStateChangedMotor);
    connect(checkSwapMotorUV, &QCheckBox::stateChanged, this, &FormSettings::onStateChangedMotor);    
    connect(checkReverseEncX, &QCheckBox::stateChanged, this, &FormSettings::onStateChangedMotor);
    connect(checkReverseEncY, &QCheckBox::stateChanged, this, &FormSettings::onStateChangedMotor);
#endif

//    connect(numHighThld, QOverload<int>::of(&QSpinBox::valueChanged), this, [&](int value) {
//        if (value < numLowThld->value())
//            numLowThld->setValue(value);
//    });

//    connect(numLowThld, QOverload<int>::of(&QSpinBox::valueChanged), this, [&](int value) {
//        if (value > numHighThld->value())
//            numHighThld->setValue(value);
//    });

    connect(fnumScaleX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FormSettings::onNumScaleChanged);
    connect(fnumScaleY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FormSettings::onNumScaleChanged);
//    connect(numScaleU, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FormSettings::onNumScaleChanged);
//    connect(numScaleV, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FormSettings::onNumScaleChanged);

//    connect(numScaleEncX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FormSettings::onNumScaleChanged);
//    connect(numScaleEncY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FormSettings::onNumScaleChanged);
}

void FormSettings::init() {
    comboLanguage->setCurrentIndex(int(ProgramParam::lang));
//    checkShowXY->setCheckState(ProgramParam::showXY ? Qt::Checked : Qt::Unchecked);

//    checkReverseX->setCheckState(ProgramParam::reverseX ? Qt::Checked : Qt::Unchecked);
//    checkReverseY->setCheckState(ProgramParam::reverseY ? Qt::Checked : Qt::Unchecked);
//    checkSwapXY->setCheckState(ProgramParam::swapXY ? Qt::Checked : Qt::Unchecked);

    numInputLevel->setValue(int(CncParam::inputLevel));
    selectComboInputLevel(numInputLevel->value());

#ifndef STONE
    checkStepDirEnable->setCheckState(CncParam::sdEnable ? Qt::Checked : Qt::Unchecked);
    groupEnc->setChecked(CncParam::encXY);
#endif

    for (QCheckBox* const o: checks)
        o->blockSignals(true);

    checkReverseMotorX->setCheckState(CncParam::reverseX ? Qt::Checked : Qt::Unchecked);
    checkReverseMotorY->setCheckState(CncParam::reverseY ? Qt::Checked : Qt::Unchecked);
    checkSwapMotorXY->setCheckState(CncParam::swapXY ? Qt::Checked : Qt::Unchecked);

#ifndef STONE
    checkReverseMotorU->setCheckState(CncParam::reverseU ? Qt::Checked : Qt::Unchecked);
    checkReverseMotorV->setCheckState(CncParam::reverseV ? Qt::Checked : Qt::Unchecked);
    checkSwapMotorUV->setCheckState(CncParam::swapUV ? Qt::Checked : Qt::Unchecked);
    checkReverseEncX->setCheckState(CncParam::reverseEncX ? Qt::Checked : Qt::Unchecked);
    checkReverseEncY->setCheckState(CncParam::reverseEncY ? Qt::Checked : Qt::Unchecked);
#endif

    onStateChangedMotor(0);

    for (QCheckBox* const o: checks)
        o->blockSignals(false);

#ifndef STONE
    fnumAcc->setValue(CncParam::fb_acc);
    fnumDec->setValue(CncParam::fb_dec);
    groupFeedback->setChecked(CncParam::fb_ena);

    numHighThld->setValue(round(CncParam::high_thld[0]));
    numLowThld->setValue(round(CncParam::low_thld[0]));
    numHighThld2->setValue(round(CncParam::high_thld[1]));
    numLowThld2->setValue(round(CncParam::low_thld[1]));

    numRbTimeout->setValue(round(CncParam::rb_to));
    numRbAttempts->setValue(CncParam::rb_attempts);
    fnumRbLength->setValue(CncParam::rb_len);
    fnumRbSpeed->setValue(CncParam::rb_speed);
#endif

    fnumStep->setValue(CncParam::step);

    for (QDoubleSpinBox* const o: scaleNums)
        o->blockSignals(true);

    fnumScaleX->setValue(CncParam::scaleX);
    fnumScaleY->setValue(CncParam::scaleY);
#ifndef STONE
    fnumScaleU->setValue(CncParam::scaleU);
    fnumScaleV->setValue(CncParam::scaleV);

    fnumScaleEncX->setValue(CncParam::scaleEncX);
    fnumScaleEncY->setValue(CncParam::scaleEncY);
#endif

    onNumScaleChanged(0);

    for (QDoubleSpinBox* const o: scaleNums)
        o->blockSignals(false);
}

void FormSettings::reset() {
    ProgramParam::loadDefaultParam();
    init();
}

void FormSettings::selectComboInputLevel(int bits) {
    comboInputLevel->blockSignals(true);

    switch (bits) {
    case (CncParam::INPUT_LEVEL_METAL): comboInputLevel->setCurrentIndex(1); break;
    case (CncParam::INPUT_LEVEL_STONE): comboInputLevel->setCurrentIndex(2); break;
    case (CncParam::INPUT_LEVEL_DEBUG): comboInputLevel->setCurrentIndex(3); break;
    default: comboInputLevel->setCurrentIndex(0); break;
    }

    comboInputLevel->blockSignals(false);
}

void FormSettings::onNumScaleChanged(double) {
    double value = std::min(fnumScaleX->value(), fnumScaleY->value());
    double step = 1.0 / value;

    if (step < 0.001)
        step = 0.001;

    fnumStep->setMinimum(step);
}

void FormSettings::onStateChangedMotor(int /*state*/) {
#ifndef STONE
    ProgramParam::saveMotorDir(
        checkReverseMotorX->isChecked(), checkReverseMotorY->isChecked(), checkReverseMotorU->isChecked(), checkReverseMotorV->isChecked(),
        checkSwapMotorXY->isChecked(), checkSwapMotorUV->isChecked(),
        checkReverseEncX->isChecked(), checkReverseEncY->isChecked()
    );
#else
    ProgramParam::saveMotorDir(
        checkReverseMotorX->isChecked(), checkReverseMotorY->isChecked(), false, false,
        checkSwapMotorXY->isChecked(), false,
        false, false
    );
#endif
}

FormSettings::~FormSettings() {}

void FormSettings::setFontPointSize(int pointSize) {
    QFont font;

    for (QPushButton* b: buttons) {
        font = b->font();
        font.setPointSize(pointSize);
        b->setFont(font);
    }

    font = labelX->font();
    font.setPointSize(pointSize + 2);

    labelTitle->setFont(font);
    labelCNC->setFont(font);
    labelLanguage->setFont(font);
    labelInputLevel->setFont(font);
    labelStep->setFont(font);

    labelX->setFont(font);
    labelY->setFont(font);

    labelPrecision->setFont(font);
    labelMotor->setFont(font);

    //
    font = comboLanguage->font();
    font.setPointSize(pointSize + 2);

    comboLanguage->setFont(font);
    comboInputLevel->setFont(font);

    //
    font = fnumStep->font();
    font.setPointSize(pointSize + 2);

    numInputLevel->setFont(font);
    fnumStep->setFont(font);

    for (QDoubleSpinBox* const o: scaleNums)
        o->setFont(font);

    font = checkSwapMotorXY->font();
    font.setPointSize(pointSize + 2);

    for (QCheckBox* const o: checks)
        o->setFont(font);

#ifndef STONE
    labelEncoder->setFont(font);
    labelU->setFont(font);
    labelV->setFont(font);
    labelEncX->setFont(font);
    labelEncY->setFont(font);

    labelAcc->setFont(font);
    labelDec->setFont(font);
    fnumAcc->setFont(font);
    fnumDec->setFont(font);

    groupFeedback->setFont(font);
    groupEnc->setFont(font);

    labelHighThld->setFont(font);
    labelLowThld->setFont(font);
    labelHighThld2->setFont(font);
    labelLowThld2->setFont(font);

    numHighThld->setFont(font);
    numLowThld->setFont(font);
    numHighThld2->setFont(font);
    numLowThld2->setFont(font);
#endif
}

void FormSettings::createButtons() {
    btnHome = new QPushButton(tr("Home"));
    btnHome->setStatusTip(tr("Go to the Home panel") + "   Alt+H");
    btnHome->setShortcut(QKeySequence("Alt+H"));

    btnRead = new QPushButton(tr("Read"));
    btnRead->setStatusTip(tr("Read settings from CNC"));

    btnWrite = new QPushButton(tr("Write"));
    btnWrite->setStatusTip(tr("Write settings"));

    btnDefault = new QPushButton(tr("Default"));
    btnDefault->setStatusTip(tr("Set default settings"));

    btn4 = new QPushButton();
    btn4->setEnabled(false);

    btn5 = new QPushButton();
    btn5->setEnabled(false);

    btn6 = new QPushButton();
    btn6->setEnabled(false);

    btn7 = new QPushButton();
    btn7->setEnabled(false);

    btn8 = new QPushButton();
    btn8->setEnabled(false);

    btn9 = new QPushButton();
    btn9->setEnabled(false);

    btn10 = new QPushButton();
    btn10->setEnabled(false);

    btn11 = new QPushButton();
    btn11->setEnabled(false);

    btn12 = new QPushButton();
    btn12->setEnabled(false);

    btnHelp = new QPushButton(tr("Help"));
    btnHelp->setStatusTip(tr("Open Help") + "   F1");
    btnHelp->setShortcut(QKeySequence::HelpContents);

    buttons = {btnHome, btnRead, btnWrite, btnDefault, btn4, btn5, btn6, btn7, btn8, btn9, btn10, btn11, btn12, btnHelp};

    gridButtons = new QGridLayout();

    gridButtons->addWidget(btnHome, 0, 0);
    gridButtons->addWidget(btnRead, 0, 1);
    gridButtons->addWidget(btnWrite, 0, 2);
    gridButtons->addWidget(btnDefault, 0, 3);
    gridButtons->addWidget(btn4, 0, 4);
    gridButtons->addWidget(btn5, 0, 5);
    gridButtons->addWidget(btn6, 0, 6);
    gridButtons->addWidget(btn7, 0, 7);
    gridButtons->addWidget(btn8, 0, 8);
    gridButtons->addWidget(btn9, 0, 9);
    gridButtons->addWidget(btn10, 0, 10);
    gridButtons->addWidget(btn11, 0, 11);
    gridButtons->addWidget(btn12, 0, 12);
    gridButtons->addWidget(btnHelp, 0, 13);

    connect(btnHome, &QPushButton::clicked, this, [&]() {
        emit showInfo(QString());
        emit homePageClicked();
    });

    connect(btnRead, &QPushButton::clicked, this, [&]() {
        uint16_t input_lvl;
        bool sdEna, revX, revY, revU, revV, swapXY, swapUV, revEncX, revEncY;
        double acc, dec;

        bool OK = par.cnc.readSettings(
                    input_lvl, sdEna, sdEna, revX, revY, revU, revV,
                    swapXY, swapUV,
                    revEncX, revEncY,
                    acc, dec
        );

        if (OK) {
            ProgramParam::saveInputLevel(input_lvl);
            ProgramParam::saveStepDir(sdEna);
            ProgramParam::saveMotorDir(revX, revY, revU, revV, swapXY, swapUV, revEncX, revEncY);
            ProgramParam::saveAcceleration(acc, dec);

            numInputLevel->setValue(input_lvl);
            checkReverseMotorX->setCheckState(revX ? Qt::Checked : Qt::Unchecked);
            checkReverseMotorY->setCheckState(revY ? Qt::Checked : Qt::Unchecked);
            checkSwapMotorXY->setCheckState(swapXY ? Qt::Checked : Qt::Unchecked);            
#ifndef STONE
            checkStepDirEnable->setCheckState(sdEna ? Qt::Checked : Qt::Unchecked);
            checkReverseMotorU->setCheckState(revU ? Qt::Checked : Qt::Unchecked);
            checkReverseMotorV->setCheckState(revV ? Qt::Checked : Qt::Unchecked);
            checkSwapMotorUV->setCheckState(swapUV ? Qt::Checked : Qt::Unchecked);
            checkReverseEncX->setCheckState(revEncX ? Qt::Checked : Qt::Unchecked);
            checkReverseEncY->setCheckState(revEncY ? Qt::Checked : Qt::Unchecked);
            fnumAcc->setValue(acc);
            fnumDec->setValue(dec);
#endif
        }
        else
            qDebug("Read Input Levels ERROR!\n");

#ifndef STONE
        bool fbEna;
        uint32_t rbAttempts;
        double low_thld[2], high_thld[2], rbTimeout, rbLength, rbSpeed;

        if (OK)
            OK = par.cnc.readFeedback(fbEna, low_thld[0], high_thld[0], rbTimeout, rbAttempts, rbLength, rbSpeed);

        low_thld[1] = CncParam::low_thld[1];
        high_thld[1] = CncParam::high_thld[1];

        if (OK) {            
            ProgramParam::saveFeedbackParam(fbEna, low_thld, high_thld, rbTimeout, rbAttempts, rbLength, rbSpeed);

            numLowThld->blockSignals(true);
            numHighThld->blockSignals(true);
            numLowThld2->blockSignals(true);
            numHighThld2->blockSignals(true);

            groupFeedback->setChecked(fbEna);

            numLowThld->setValue(static_cast<int>(round(low_thld[0])));
            numHighThld->setValue(static_cast<int>(round(high_thld[0])));
            numLowThld2->setValue(static_cast<int>(round(low_thld[1])));
            numHighThld2->setValue(static_cast<int>(round(high_thld[1])));

            numRbTimeout->setValue(rbTimeout);
            numRbAttempts->setValue(rbAttempts);
            fnumRbLength->setValue(rbLength);
            fnumRbSpeed->setValue(rbSpeed);

            numLowThld->blockSignals(false);
            numHighThld->blockSignals(false);
            numLowThld2->blockSignals(false);
            numHighThld2->blockSignals(false);
        } else {
            groupFeedback->setChecked(false);
        }

        float step;
        bool encXY;
        float scaleX, scaleY, scaleU, scaleV, scaleEncX, scaleEncY;

        if (OK)
            OK = par.cnc.readStep(step, scaleX, scaleY, scaleU, scaleV, scaleEncX, scaleEncY, encXY);

        if (OK) {
            ProgramParam::saveStep(step, scaleX, scaleY, scaleU, scaleV, scaleEncX, scaleEncY, encXY);
            fnumStep->setValue(step);
            fnumScaleX->setValue(scaleX);
            fnumScaleY->setValue(scaleY);
            fnumScaleU->setValue(scaleU);
            fnumScaleV->setValue(scaleV);
            fnumScaleEncX->setValue(scaleEncX);
            fnumScaleEncY->setValue(scaleEncY);
            groupEnc->setChecked(encXY);
        }
#endif

        if (OK)
            emit showInfo("Read OK");
        else
            emit showError("Read ERROR!");
    });

    connect(btnWrite, &QPushButton::clicked, this, [&]() {
        uint16_t input_lvl = static_cast<uint16_t>(numInputLevel->value() & 0xFFFF);
        bool revX = checkReverseMotorX->isChecked();
        bool revY = checkReverseMotorY->isChecked();
        bool swapXY = checkSwapMotorXY->isChecked();

#ifndef STONE
        bool encXY = groupEnc->isChecked();
        bool sd_oe = checkStepDirEnable->isChecked();
        bool revU = checkReverseMotorU->isChecked();
        bool revV = checkReverseMotorV->isChecked();
        bool swapUV = checkSwapMotorUV->isChecked();
        bool revEncX = groupEnc->isChecked() && checkReverseEncX->isChecked();
        bool revEncY = groupEnc->isChecked() && checkReverseEncY->isChecked();

        float step = fnumStep->value();

        float scaleX = fnumScaleX->value();
        float scaleY = fnumScaleY->value();

        float scaleU = fnumScaleU->value();
        float scaleV = fnumScaleV->value();
        float scaleEncX = fnumScaleEncX->value();
        float scaleEncY = fnumScaleEncY->value();
#else
        const bool encXY = false;
        const bool sd_oe = false;
        const bool revU = false;
        const bool revV = false;
        const bool swapUV = false;
        const bool revEncX = false;
        const bool revEncY = false;
        const double acc = CncParam::DEFAULT_ACC;
        const double dec = CncParam::DEFAULT_DEC;

        const float step = CncParam::DEFAULT_STEP;

        const float scaleX = CncParam::DEFAULT_SCALE_XY;
        const float scaleY = CncParam::DEFAULT_SCALE_XY;

        const float scaleU = CncParam::DEFAULT_SCALE_UV;
        const float scaleV = CncParam::DEFAULT_SCALE_UV;
        const float scaleEncX = CncParam::DEFAULT_SCALE_ENC_XY;
        const float scaleEncY = CncParam::DEFAULT_SCALE_ENC_XY;
#endif

        ProgramParam::saveInputLevel(input_lvl);
        ProgramParam::saveMotorDir(revX, revY, revU, revV, swapXY, swapUV, revEncX, revEncY);

#ifndef STONE
        double acc = fnumAcc->value();
        double dec = fnumDec->value();
        ProgramParam::saveAcceleration(acc, dec);
#endif

        ProgramParam::saveStep(step, scaleX, scaleY, scaleU, scaleV, scaleEncX, scaleEncY, encXY);

        bool OK =   par.cnc.writeSettings(
                        input_lvl,
                        sd_oe, sd_oe,
                        revX, revY, revU, revV,
                        swapXY, swapUV,
                        revEncX, revEncY,
                        acc, dec
                    );

#ifndef STONE
        if (numHighThld->value() < numLowThld->value())
            numHighThld->setValue( numLowThld->value() );

        if (numHighThld2->value() < numLowThld2->value())
            numHighThld2->setValue( numLowThld2->value() );

        double lowThld[2], highThld[2];
        lowThld[0] = numLowThld->value();
        lowThld[1] = numLowThld2->value();

        highThld[0] = numHighThld->value();
        highThld[1] = numHighThld2->value();

        ProgramParam::saveFeedbackParam(
                    groupFeedback->isChecked(),
                    lowThld,
                    highThld,
                    numRbTimeout->value(),
                    numRbAttempts->value(),
                    fnumRbLength->value(),
                    fnumRbSpeed->value()
                );

        if (OK)
            OK &=   par.cnc.writeFeedback(
                    groupFeedback->isChecked(),
                    numLowThld->value(),
                    numHighThld->value(),
                    numRbTimeout->value(),
                    numRbAttempts->value(),
                    fnumRbLength->value(),
                    fnumRbSpeed->value()
                );

        if (OK)
            OK &= par.cnc.writeStep(step, scaleX, scaleY, scaleU, scaleV, scaleEncX, scaleEncY, encXY);
#endif

        if (OK) {
#ifndef STONE
            OK &= par.cnc.writeCncEnable(true);
#else
            OK &= par.cnc.writeSemaphoreCncEnable(true);
#endif
        }

        if (OK) {
            emit showInfo("Write OK");
            btnRead->click();
        } else
            emit showError("Write ERROR!");
    });

    connect(btnDefault, &QPushButton::clicked, this, [&]() {
        reset();
    });

    connect(btnHelp, &QPushButton::clicked, this, [&]() { emit helpPageClicked(help_file); });
}
