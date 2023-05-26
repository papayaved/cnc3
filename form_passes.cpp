#include "form_passes.h"
#include <QSpacerItem>
#include "spinbox_delegate.h"
#include "combobox_delegate.h"
#include <QHeaderView>
#include <QStandardItemModel>
#include "mode_table_model.h"
#include <QLineEdit>
#include "cnc_types.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollArea>

using namespace std;

FormPasses::FormPasses(ProgramParam& par, QWidget *parent) : QWidget(parent), par(par), m_pass_num(1), m_row(0) {
    this->setObjectName(tr("Cutting settings"));

    QLabel* labelTitle = new QLabel(R"(<h1>)" + this->objectName() + R"(</h1>)");
    labelTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    createPasses();
    createModeView();
    createButtons();

    buttons = {
            btnBack, btnOpen, btnSave, btnSaveAs, btnDefault, btn5, btn6, btn7, btn8, btn9, btn10, btn11, btnGen, btnHelp,
            btnNewMode, btnDeleteMode, btnOpenModes, btnSaveModes, btnSaveAsModes, btnDefaultModes
        };
    labels = {
        labelTimes, labelOvercut, labelTab, labelTabOffset, labelTabMode, labelCutMode, labelPumpDelay, labelSpeed, tableTitle,
        labelL, labelH, labelT, labelD, labelWireD, labelAxis
    };
    radio = {radioLeftSide, radioRightSide, onePassTab, multiPassTab, radioX, radioY};
    combo = {comboTimes, comboCutLineMode, comboTabMode};
    checks = {checkUseLastSeg, checkTabPause, checkTapered, checkPumpPause};
    nums = {inputOvercut, inputTab, inputTabOffset, inputSpeed, inputL, inputH, inputT, inputD, inputWireD};
    groups = {groupTab, groupD};

//    setFontPointSize(14);

    init(par.cutParam.uv_ena);

    gridPassMode = new QGridLayout;

    gridPassMode->addWidget(labelTitle, 0, 0, 1, 3, Qt::AlignHCenter);
    gridPassMode->addLayout(gridPasses, 1, 0, Qt::AlignVCenter | Qt::AlignRight);
//    gridPassMode->addItem(new QSpacerItem(32, 0, QSizePolicy::Fixed, QSizePolicy::Preferred), 1, 1);
    gridPassMode->addItem(new QSpacerItem(32, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 1, 1);
    gridPassMode->addLayout(gridModes, 1, 2, Qt::AlignVCenter | Qt::AlignLeft);

    gridPassMode->setColumnStretch(0, 1);
    gridPassMode->setColumnStretch(1, 0);
    gridPassMode->setColumnStretch(2, 1);

    gridPassMode->setSizeConstraint(QLayout::SetFixedSize);

    //
    QWidget* widgetPassMode = new QWidget;
    widgetPassMode->setLayout(gridPassMode);

    QScrollArea* scrollArea = new QScrollArea;
    scrollArea->setWidget(widgetPassMode);
    scrollArea->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(scrollArea, Qt::AlignTop | Qt::AlignHCenter);
    mainLayout->addLayout(gridButtons);

    this->setLayout(mainLayout);
}

void FormPasses::createPasses() {
    radioLeftSide = new QRadioButton(tr("Left Offset"));
    radioRightSide = new QRadioButton(tr("Right Offset"));

    groupSide = new QGroupBox;
    groupSide->setLayout(new QHBoxLayout);
    groupSide->layout()->addWidget(radioLeftSide);
    groupSide->layout()->addWidget(radioRightSide);

    groupSide->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    labelTimes = new QLabel(tr("Number of passes"));
    comboTimes = new QComboBox;
    labelTimes->setBuddy(comboTimes);

    comboTimes->setEditable(true);
    comboTimes->lineEdit()->setReadOnly(true);
    comboTimes->lineEdit()->setAlignment(Qt::AlignCenter);

    for (size_t i = 0; i < par.cutParam.MAX_TIMES; i++) {
        comboTimes->addItem(QString::number(i + 1));
        comboTimes->setItemData(int(i), Qt::AlignCenter, Qt::TextAlignmentRole);
    }

//    comboTimes->lineEdit()->setAlignment(Qt::AlignCenter);
//    comboTimes->setRange(1, int(par.cut.TIMES_MAX));
//    comboTimes->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

    labelOvercut = new QLabel(tr("Overcut"));
    inputOvercut = new QDoubleSpinBox;
    labelOvercut->setBuddy(inputOvercut);
    inputOvercut->setSuffix(" " + tr("mm"));
    inputOvercut->setDecimals(1);
    inputOvercut->setSingleStep(1);
    inputOvercut->setMaximum(100);
    inputOvercut->setAccelerated(true);
    inputOvercut->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    labelPasses = new QLabel*[par.cutParam.MAX_TIMES];
    inputOffsets = new QDoubleSpinBox*[par.cutParam.MAX_TIMES];

    labelModes = new QLabel*[par.cutParam.MAX_TIMES];
    comboModes = new QComboBox*[par.cutParam.MAX_TIMES];

    for (size_t i = 0; i < par.cutParam.MAX_TIMES; i++) {
        labelPasses[i] = new QLabel(tr("Offset on pass") + " " + QString::number(i + 1));
        inputOffsets[i] = new QDoubleSpinBox;
        labelPasses[i]->setBuddy(inputOffsets[i]);
        inputOffsets[i]->setSuffix(" " + tr("mm"));
        inputOffsets[i]->setDecimals(3);
        inputOffsets[i]->setSingleStep(0.1);
        inputOffsets[i]->setMaximum(2);
        inputOffsets[i]->setMinimum(0);
        inputOffsets[i]->setAccelerated(true);
        inputOffsets[i]->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

//        if (i > 0) inputOffset[i]->setEnabled(false);

        labelModes[i] = new QLabel(tr("Mode of pass") + " " + QString::number(i + 1));
        comboModes[i] = new QComboBox;
        labelModes[i]->setBuddy(comboModes[i]);
        comboModes[i]->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//        if (i > 0) comboMode[i]->setEnabled(false);
    }

    labelCutMode = new QLabel(tr("Entry Line Mode"));
    comboCutLineMode = new QComboBox;
    labelCutMode->setBuddy(comboCutLineMode);

    // Tab
    onePassTab = new QRadioButton(tr("Onepass"));
    multiPassTab = new QRadioButton(tr("Multi-pass"));

    labelTab = new QLabel(tr("Tab Width"));
    inputTab = new QDoubleSpinBox;
    labelTab->setBuddy(inputTab);
    inputTab->setSuffix(" mm");
    inputTab->setDecimals(1);
    inputTab->setSingleStep(1);
    inputTab->setMaximum(100);
    inputTab->setAccelerated(true);
    inputTab->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    groupTab = new QGroupBox(tr("Workpiece Tab"));
    groupTab->setCheckable(true);
//    groupTab->setLayout(new QHBoxLayout);
//    groupTab->layout()->addWidget(onePassTab);
//    groupTab->layout()->addWidget(multiPassTab);
//    groupTab->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    labelTabOffset = new QLabel(tr("Tab Offset"));
    inputTabOffset = new QDoubleSpinBox;
    labelTabOffset->setBuddy(inputTabOffset);
    inputTabOffset->setSuffix(" " + tr("mm"));
    inputTabOffset->setDecimals(3);
    inputTabOffset->setSingleStep(0.1);
    inputTabOffset->setMaximum(2);
    inputTabOffset->setAccelerated(true);
    inputTabOffset->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    checkUseLastSeg = new QCheckBox(tr("Use the last segment"));
    checkUseLastSeg->setCheckState(Qt::CheckState::Unchecked);

    labelTabMode = new QLabel(tr("Tab Mode"));
    comboTabMode = new QComboBox;
    labelTabMode->setBuddy(comboTabMode);

    checkTabPause = new QCheckBox(tr("Tab Pause"));
    checkTabPause->setEnabled(false);
    checkTabPause->setCheckState(Qt::CheckState::Checked);

    QGridLayout* gridTab = new QGridLayout();
    gridTab->addWidget(onePassTab, 0, 1, Qt::AlignLeft | Qt::AlignTop);
    gridTab->addWidget(multiPassTab, 0, 2, Qt::AlignLeft | Qt::AlignTop);

    gridTab->addWidget(labelTab, 1, 0, Qt::AlignRight);
    gridTab->addWidget(inputTab, 1, 1, Qt::AlignLeft);
    gridTab->addWidget(checkUseLastSeg, 1, 2, 1, 2, Qt::AlignLeft);

    gridTab->addWidget(labelTabOffset, 2, 0, Qt::AlignRight);
    gridTab->addWidget(inputTabOffset, 2, 1, Qt::AlignLeft);
    gridTab->addWidget(labelTabMode, 2, 2, Qt::AlignRight);
    gridTab->addWidget(comboTabMode, 2, 3, Qt::AlignLeft);
    gridTab->addWidget(checkTabPause, 3, 1, 1, 2, Qt::AlignLeft);

    groupTab->setLayout(gridTab);

    // Pump. Don't use
    labelPumpDelay = new QLabel(tr("Pump On Pause"));
    inputPumpDelay = new QSpinBox;
    labelPumpDelay->setBuddy(inputPumpDelay);
    inputPumpDelay->setSuffix(" " + tr("sec"));
    inputPumpDelay->setRange(30, 600);
    inputPumpDelay->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    inputPumpDelay->setAccelerated(true);
    inputPumpDelay->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    checkPumpPause = new QCheckBox(tr("Pump On Stop"));

    labelSpeed = new QLabel(tr("Cutting Speed Limit"));
    inputSpeed = new QDoubleSpinBox;
    labelSpeed->setBuddy(inputSpeed);
    inputSpeed->setSuffix(" " + tr("mm/min"));
    inputSpeed->setDecimals(1);
    inputSpeed->setMinimum(0.1);
    inputSpeed->setMaximum(18);
    inputSpeed->setSingleStep(0.1);
    inputSpeed->setAccelerated(true);
    inputSpeed->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    labelL = new QLabel(tr("Distance between rollers") + " (L):");
    labelH = new QLabel(tr("Workpiece bottom height") + " (H):");
    labelT = new QLabel(tr("Workpiece thickness") + " (T):");
    labelD = new QLabel(tr("Roller diameter") + ":");
    labelWireD = new QLabel(tr("Wire diameter") + ":");
    labelAxis = new QLabel(tr("Roller plane") + ":");

//    par.cutParam.L = 190;
//    par.cutParam.H = 50;
//    par.cutParam.T = 30;
//    par.cutParam.uv_ena = m_uv_ena = false;

//    par.cutParam.D = 29.5;
//    par.cutParam.wire_D = 0.18;
//    par.cutParam.axis_D = AXIS::AXIS_X;
//    par.cutParam.D_ena = m_D_ena = false;

    checkTapered = new QCheckBox(tr("Tapered cutting"));
    checkTapered->setEnabled(false);
    checkTapered->setCheckState(Qt::CheckState::Unchecked);

    inputL = new QDoubleSpinBox;
    labelL->setBuddy(inputL);
    inputL->setSuffix(" " + tr("mm"));
    inputL->setDecimals(3);
    inputL->setSingleStep(0.001);
    inputL->setRange(0, 1000);
    inputL->setValue(par.cutParam.L);
    inputL->setAccelerated(true);
    inputL->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    inputH = new QDoubleSpinBox;
    labelH->setBuddy(inputH);
    inputH->setSuffix(" " + tr("mm"));
    inputH->setDecimals(3);
    inputH->setSingleStep(0.001);
    inputH->setRange(0, 200);
    inputH->setValue(par.cutParam.H);
    inputH->setAccelerated(true);
    inputH->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    inputT = new QDoubleSpinBox;
    labelT->setBuddy(inputT);
    inputT->setSuffix(" " + tr("mm"));
    inputT->setDecimals(3);
    inputT->setSingleStep(0.001);
    inputT->setRange(0, 200);
    inputT->setValue(par.cutParam.T);
    inputT->setAccelerated(true);
    inputT->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    inputD = new QDoubleSpinBox;
    labelD->setBuddy(inputD);
    inputD->setSuffix(" " + tr("mm"));
    inputD->setDecimals(3);
    inputD->setSingleStep(0.001);
    inputD->setRange(0, 200);
    inputD->setValue(par.cutParam.D);
    inputD->setAccelerated(true);
    inputD->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    inputWireD = new QDoubleSpinBox;
    labelWireD->setBuddy(inputWireD);
    inputWireD->setSuffix(" " + tr("mm"));
    inputWireD->setDecimals(3);
    inputWireD->setSingleStep(0.001);
    inputWireD->setRange(0, 200);
    inputWireD->setValue(par.cutParam.wire_D);
    inputWireD->setAccelerated(true);
    inputWireD->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    radioX = new QRadioButton("XZ");
    radioY = new QRadioButton("YZ");

    groupAxis = new QGroupBox();
    groupAxis->setLayout(new QHBoxLayout);
    groupAxis->layout()->addWidget(radioX);
    groupAxis->layout()->addWidget(radioY);

    groupAxis->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    groupD = new QGroupBox(tr("Use the roller diameter") + " (D)");

    QGridLayout* gridD = new QGridLayout;

    gridD->addWidget(labelD, 0, 0, Qt::AlignRight);
    gridD->addWidget(inputD, 0, 1, Qt::AlignLeft);

    gridD->addWidget(labelWireD, 1, 0, Qt::AlignRight);
    gridD->addWidget(inputWireD, 1, 1, Qt::AlignLeft);

    gridD->addWidget(labelAxis, 2, 0, Qt::AlignRight);
    gridD->addWidget(groupAxis, 2, 1, Qt::AlignLeft);

    groupD->setLayout(gridD);
    groupD->setCheckable(true);
    groupD->setChecked(par.cutParam.D_ena);

    groupTapered = new QGroupBox;

    QGridLayout* gridTapered = new QGridLayout;

    gridTapered->addWidget(labelL, 0, 0, Qt::AlignRight);
    gridTapered->addWidget(inputL, 0, 1, Qt::AlignLeft);
    gridTapered->addWidget(labelH, 1, 0, Qt::AlignRight);
    gridTapered->addWidget(inputH, 1, 1, Qt::AlignLeft);
    gridTapered->addWidget(labelT, 2, 0, Qt::AlignRight);
    gridTapered->addWidget(inputT, 2, 1, Qt::AlignLeft);

    gridTapered->addWidget(groupD, 3, 0, 1, 2);

    groupTapered->setLayout(gridTapered);
    groupTapered->setEnabled(false);

    QVBoxLayout* vboxTapered = new QVBoxLayout;
    vboxTapered->addWidget(checkTapered);
    vboxTapered->addWidget(groupTapered);

    gridPasses = new QGridLayout;

    gridPasses->addWidget(groupSide, 0, 1, 1, 3, Qt::AlignLeft | Qt::AlignTop);
    gridPasses->addWidget(labelTimes, 1, 0, Qt::AlignRight);
    gridPasses->addWidget(comboTimes, 1, 1);

    for (int i = 0; i < int(par.cutParam.MAX_TIMES); i++) {
        gridPasses->addWidget(labelPasses[i], 2 + i, 0, Qt::AlignRight);
        gridPasses->addWidget(inputOffsets[i], 2 + i, 1, Qt::AlignLeft);
        gridPasses->addWidget(labelModes[i], 2 + i, 2, Qt::AlignRight);
        gridPasses->addWidget(comboModes[i], 2 + i, 3, Qt::AlignLeft);
    }

    QFrame* hLine[5];
    for (size_t i = 0; i < sizeof(hLine)/sizeof(hLine[0]); i++) {
        hLine[i] = new QFrame;
        hLine[i]->setFrameShape(QFrame::HLine);
        hLine[i]->setFrameShadow(QFrame::Sunken);
        hLine[i]->setLineWidth(3);
    }

    gridPasses->addWidget(hLine[0], 9, 1, 1, 3);

    gridPasses->addWidget(labelCutMode, 10, 2, Qt::AlignRight);
    gridPasses->addWidget(comboCutLineMode, 10, 3, Qt::AlignLeft);

    gridPasses->addWidget(hLine[1], 11, 1, 1, 3);

    gridPasses->addWidget(groupTab, 12, 0, 1, 4, Qt::AlignJustify);
//    gridPasses->addWidget(groupTab, 12, 1, 1, 3, Qt::AlignLeft | Qt::AlignTop);

//    gridPasses->addWidget(labelTab, 13, 0, Qt::AlignRight);
//    gridPasses->addWidget(inputTab, 13, 1, Qt::AlignLeft);

//    gridPasses->addWidget(labelTabOffset, 14, 0, Qt::AlignRight);
//    gridPasses->addWidget(inputTabOffset, 14, 1, Qt::AlignLeft);
//    gridPasses->addWidget(labelTabMode, 14, 2, Qt::AlignRight);
//    gridPasses->addWidget(comboTabMode, 14, 3, Qt::AlignLeft);
//    gridPasses->addWidget(checkTabPause, 15, 1, 1, 3, Qt::AlignLeft);

    gridPasses->addWidget(hLine[2], 16, 1, 1, 3);

    gridPasses->addWidget(labelOvercut, 17, 0, Qt::AlignRight);
    gridPasses->addWidget(inputOvercut, 17, 1, Qt::AlignLeft);

    gridPasses->addWidget(hLine[3], 18, 1, 1, 3);
    gridPasses->addWidget(labelSpeed, 19, 0, Qt::AlignRight);
    gridPasses->addWidget(inputSpeed, 19, 1, Qt::AlignLeft);

    gridPasses->addWidget(hLine[4], 20, 1, 1, 3);
    gridPasses->addLayout(vboxTapered, 21, 0, 1, 4, Qt::AlignJustify);

//    gridPasses->addWidget(hLine[4], 18, 1, 1, 3);

//    gridPasses->addWidget(labelPumpDelay, 19, 0, Qt::AlignLeft);
//    gridPasses->addWidget(inputPumpDelay, 19, 1, Qt::AlignLeft);
//    gridPasses->addWidget(checkPumpPause, 20, 1, 1, 3, Qt::AlignLeft);

    gridPasses->setSizeConstraint(QLayout::SetFixedSize);

//    gridPasses->setColumnStretch(0, 0);
//    gridPasses->setColumnStretch(1, 1);
//    gridPasses->setColumnStretch(2, 0);
//    gridPasses->setColumnStretch(3, 1);
//    gridPasses->setColumnStretch(4, 0);

//    gridPasses->setRowStretch(12, 1);

    connect(radioLeftSide, &QRadioButton::clicked, this, [&]()                                          { par.cutParam.offset_side = OFFSET_SIDE::LEFT; });
    connect(radioRightSide, &QRadioButton::clicked, this, [&]()                                         { par.cutParam.offset_side = OFFSET_SIDE::RIGHT; });
//    connect(comboTimes, QOverload<int>::of(&QSpinBox::valueChanged), this, [&](int value)             { setCutTimes(value); });
    connect(comboTimes, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int index)       { setCutTimes(index + 1); });
    connect(inputOvercut, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [&](double value) { par.cutParam.overcut = value; });
    connect(inputTab, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [&](double value)     { par.cutParam.tab = value; });

    connect(groupTab, &QGroupBox::clicked, this, [&](bool) {
        if (groupTab->isChecked()) {
            par.cutParam.tab_pause = true;
            comboTabMode->setEnabled(true);
            inputTabOffset->setEnabled(true);
            checkTabPause->setCheckState(Qt::CheckState::Checked);

            if (onePassTab->isChecked())
                onePassTab->click();
            else
                multiPassTab->click();

            if (checkUseLastSeg->isChecked()) {
                par.cutParam.tab = 0;
                par.cutParam.tab_seg = 1;
                inputTab->setEnabled(false);
            } else {
                par.cutParam.tab = inputTab->value();
                par.cutParam.tab_seg = 0;
                inputTab->setEnabled(true);
            }
        } else {
            par.cutParam.tab = 0;
            par.cutParam.tab_seg = 0;
            par.cutParam.tab_pause = false;
            inputTab->setEnabled(false);
            comboTabMode->setEnabled(false);
            inputTabOffset->setEnabled(false);
            checkTabPause->setCheckState(Qt::CheckState::Unchecked);
        }
    });

    for (size_t i = 0; i < par.cutParam.offsets.size(); i++) {
        connect(inputOffsets[i], QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [&, i](double value) {
            this->par.cutParam.offsets[i].offset = value;

            if (i == 0) {
                par.cutParam.tab_offset.offset = value;
                inputTabOffset->setValue(value);
            }
        });
        connect(comboModes[i], QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&, i](int index) {
            this->par.cutParam.offsets[i].mode_id = uint8_t(index);

            if (i == 0) {
                par.cutParam.tab_offset.mode_id = uint8_t(index);
                comboTabMode->setCurrentIndex(index);
                comboCutLineMode->setCurrentIndex(index);
            }
        });
    }

    connect(onePassTab, &QRadioButton::clicked, this, [&]() {
        par.cutParam.tab_multi_pass = false;

        if (groupTab->isChecked()) {
            inputTabOffset->setEnabled(true);
            comboTabMode->setEnabled(true);
        }
    });
    connect(multiPassTab, &QRadioButton::clicked, this, [&]() {
        par.cutParam.tab_multi_pass = true;

        if (groupTab->isChecked()) {
            inputTabOffset->setEnabled(false);
            comboTabMode->setEnabled(false);
        }
    });
    connect(inputTabOffset, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [&](double value)   { par.cutParam.tab_offset.offset = value; });
    connect(comboTabMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int index)               {
        par.cutParam.tab_offset.mode_id = uint8_t(index);
    });
    connect(comboCutLineMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int index)               {
        par.cutParam.cutline_mode_id = uint8_t(index);
    });

    connect(checkUseLastSeg,    &QCheckBox::clicked, this, [&](bool checked) {
        par.cutParam.tab_seg = checked ? 1 : 0;
        inputTab->setEnabled(!checked);
    });
    connect(checkTabPause,      &QCheckBox::clicked, this, [&](bool checked)                                { par.cutParam.tab_pause = checked; });

    connect(inputPumpDelay, QOverload<int>::of(&QSpinBox::valueChanged), this, [&](int value)               { par.cutParam.pump_delay = uint16_t(value); });
    connect(checkPumpPause, &QCheckBox::clicked, this, [&](bool checked)                                    { par.cutParam.pump_pause = checked; });

    connect(inputSpeed,     QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [&](double value)   { par.cutParam.speed = value; });

    connect(checkTapered,   &QCheckBox::clicked, this, [&](bool checked) {
        par.cutParam.uv_ena = checked;
        groupTapered->setEnabled(checked);
        groupD->setEnabled(checked);
    });
    connect(inputL,         QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [&](double value)   { par.cutParam.L = value; });
    connect(inputH,         QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [&](double value)   { par.cutParam.H = value; });
    connect(inputT,         QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [&](double value)   { par.cutParam.T = value; });

    connect(inputD,         QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [&](double value)   { par.cutParam.D = value; });
    connect(inputWireD,     QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [&](double value)   { par.cutParam.wire_D = value; });
    connect(radioX,         &QRadioButton::clicked, this, [&]()                                             { par.cutParam.axis_D = AXIS::AXIS_X; });
    connect(radioY,         &QRadioButton::clicked, this, [&]()                                             { par.cutParam.axis_D = AXIS::AXIS_Y; });
}

void FormPasses::initTableModes() {
    QItemSelectionModel* m = tableModes->selectionModel(); // ??
    tableModes->setModel(new ModeTableModel(par.genModesList, this));
    delete m;

    tableModes->resizeColumnsToContents();
    initComboMode();
}

void FormPasses::init(bool uv_ena) {
    initTableModes();

    bool rightChecked = ProgramParam::swapXY ? par.cutParam.offset_side == OFFSET_SIDE::RIGHT : par.cutParam.offset_side == OFFSET_SIDE::LEFT;
    radioLeftSide->setChecked(rightChecked);
    radioRightSide->setChecked(!rightChecked);

    comboTimes->setCurrentIndex(par.cutParam.times - 1);

    setCutTimes(par.cutParam.times);

    inputOvercut->setValue(par.cutParam.overcut);

    for (size_t i = 0; i < par.cutParam.offsets.size(); i++) {
        if (i < par.cutParam.offsets.size()) {
            inputOffsets[i]->setValue(par.cutParam.offsets[i].offset);
            comboModes[i]->setCurrentIndex(par.cutParam.offsets[i].mode_id);
        }
        else {
//            inputOffset[i]->setValue(0);
//            comboMode[i]->setCurrentIndex(0);
        }
    }

    comboCutLineMode->setCurrentIndex(par.cutParam.cutline_mode_id);

    inputTab->setValue(par.cutParam.tab);
    groupTab->setChecked(abs(par.cutParam.tab) > CncParam::PRECISION);

    onePassTab->setChecked(!par.cutParam.tab_multi_pass);
    multiPassTab->setChecked(par.cutParam.tab_multi_pass);

    inputTabOffset->setEnabled(!par.cutParam.tab_multi_pass);
    comboTabMode->setEnabled(!par.cutParam.tab_multi_pass);

    checkUseLastSeg->setCheckState(Qt::CheckState::Unchecked);
    inputTabOffset->setValue(par.cutParam.tab_offset.offset);
    comboTabMode->setCurrentIndex(par.cutParam.tab_offset.mode_id);
    checkTabPause->setCheckState((par.cutParam.tab_pause) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

    inputPumpDelay->setValue(par.cutParam.pump_delay);
    checkPumpPause->setCheckState((par.cutParam.pump_pause) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

    inputSpeed->setValue(par.cutParam.speed);

    m_uv_ena = uv_ena;
    par.cutParam.uv_ena = m_uv_ena;
    checkTapered->setCheckState(m_uv_ena ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    checkTapered->setEnabled(m_uv_ena);
    groupTapered->setEnabled(m_uv_ena);

    inputL->setValue(par.cutParam.L);
    inputH->setValue(par.cutParam.H);
    inputT->setValue(par.cutParam.T);

    if (m_uv_ena) {
        par.cutParam.times = 1;
        comboTimes->setCurrentIndex(0);
    }

    groupD->setEnabled(m_uv_ena);
    groupD->setChecked(par.cutParam.D_ena);

    inputD->setValue(par.cutParam.D);
    inputWireD->setValue(par.cutParam.wire_D);

    radioX->setChecked(par.cutParam.axis_D == AXIS::AXIS_X);
    radioY->setChecked(par.cutParam.axis_D == AXIS::AXIS_Y);
}

void FormPasses::resizeModeView() {
    tableModes->resizeColumnsToContents();

    int width = 0;
    for (int i = 0; i < tableModes->model()->columnCount(); i++)
        width += tableModes->columnWidth(i);

    width += tableModes->verticalHeader()->width() + tableModes->columnWidth(0) / 4;

    int height = tableModes->rowHeight(0) * 10;

    tableModes->setFixedSize(width, height);
}

void FormPasses::createModeView() {
    tableTitle = new QLabel(R"(<h3>)" + tr("Generator Operation Modes") + R"(</h3>)");

    tableModes = new QTableView;

    tableModes->setModel(new ModeTableModel(par.genModesList, this));
//    tableModes->setItemDelegate(new SpinBoxDelegate(this));
    tableModes->setItemDelegateForColumn(0, new SpinBoxDelegate(cnc_param::ROLL_MIN, cnc_param::ROLL_MAX, this));
    tableModes->setItemDelegateForColumn(1, new SpinBoxDelegate(1, cnc_param::CURRENT_MAX, this));
    tableModes->setItemDelegateForColumn(2, new SpinBoxDelegate(cnc_param::PULSE_WIDTH_MIN, cnc_param::PULSE_WIDTH_MAX, this));
    tableModes->setItemDelegateForColumn(3, new SpinBoxDelegate(cnc_param::PULSE_RATIO_MIN, cnc_param::PULSE_RATIO_MAX, this));
    tableModes->setItemDelegateForColumn(4, new ComboBoxDelegate(this));
    resizeModeView();

    tableModes->horizontalHeader()->setStretchLastSection(true);

//    tableMode->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    btnNewMode = new QPushButton(tr("New"));
    btnDeleteMode = new QPushButton(tr("Delete"));
    btnOpenModes = new QPushButton(tr("Open"));
    btnSaveModes = new QPushButton(tr("Save"));
    btnSaveModes->setEnabled(false);
    btnSaveAsModes = new QPushButton(tr("Save As"));
    btnDefaultModes = new QPushButton(tr("Default"));

    vboxEdit = new QVBoxLayout;
    vboxEdit->addWidget(btnNewMode);
    vboxEdit->addWidget(btnDeleteMode);
    vboxEdit->addWidget(btnOpenModes);
    vboxEdit->addWidget(btnSaveModes);
    vboxEdit->addWidget(btnSaveAsModes);
    vboxEdit->addWidget(btnDefaultModes);
//    vboxEdit->addItem(new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Expanding));
    vboxEdit->setSizeConstraint(QLayout::SetFixedSize);

    gridModes = new QGridLayout;
    gridModes->addWidget(tableTitle, 0, 0, Qt::AlignHCenter | Qt::AlignBottom);

    gridModes->addWidget(tableModes, 1, 0, Qt::AlignLeft | Qt::AlignTop);
    gridModes->addLayout(vboxEdit, 1, 1, Qt::AlignLeft | Qt::AlignTop);
    gridModes->setSizeConstraint(QLayout::SetFixedSize);

//    gridMode->setColumnStretch(0, 0);
//    gridMode->setColumnStretch(1, 1);
//    gridMode->setColumnStretch(2, 1);

    connect(tableModes, &QTableView::clicked, this, [&](const QModelIndex& index) {
        if (index.isValid())
            m_row = index.row() < 0 ? 0 : index.row();
    });
//    connect(tableModes, SIGNAL(clicked(const QModelIndex&)), this, SLOT(on_tableMode_clicked(const QModelIndex&)));

    connect(btnNewMode, &QPushButton::clicked, this, [&]() {
        tableModes->model()->insertRow(int(par.genModesList.size()));
        initComboMode();
        unbindModesFile();
    });

    connect(btnOpenModes, &QPushButton::clicked, this, &FormPasses::on_btnOpenModes_clicked);
    connect(btnSaveModes, &QPushButton::clicked, this, &FormPasses::on_btnSaveModes_clicked);
    connect(btnSaveAsModes, &QPushButton::clicked, this, &FormPasses::on_btnSaveAsModes_clicked);

    connect(btnDeleteMode, &QPushButton::clicked, this, [&]() {
        tableModes->model()->removeRow(m_row);
        initComboMode();
        unbindModesFile();
    });

    connect(btnDefaultModes, &QPushButton::clicked, this, [&]() {
        par.setDefaultGenModeList();
        initTableModes();
        unbindModesFile();
    });
}

void FormPasses::bindModesFile() { btnSaveModes->setEnabled(true); }
void FormPasses::unbindModesFile() { btnSaveModes->setEnabled(false); }
bool FormPasses::bindedModesFile() const { return btnSaveModes->isEnabled(); }

void FormPasses::on_btnOpenModes_clicked() {
    QDir dir(par.projDir);

    if (!dir.exists()) {
        par.projDir = QDir::homePath() + par.projDirDefault;
        dir = QDir(par.projDir);

        if (!dir.exists()) {
            bool OK = dir.mkpath(".");

            if (!OK)
                par.projDir.clear();
        }
    }

    QString selectedFilter = tr("Modes files") + " (*.xmd *.XMD)";
    QString new_filePath = QFileDialog::getOpenFileName(this, tr("Open modes file"),
                                par.projDir + "/" + par.modesFileName,
                                selectedFilter + ";;" + tr("All files") + " (*)",
                                &selectedFilter
                            );

    if (new_filePath.length() != 0) {
        QFileInfo fInfo(new_filePath);
        par.projDir = fInfo.dir().path();
        par.modesFileName = fInfo.fileName();

        QFile file(par.projDir + "/" + par.modesFileName);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            QMessageBox::critical(this, tr("Operation modes: Open file error"), file.errorString());
        else {
            GeneratorModeList list(par.genModesList);
            list.readXML(file);

            if (list.hasError())
                QMessageBox::critical(this, tr("Operation modes: Parse file error"), list.errorString(), QMessageBox::Ok);
            else {
                par.genModesList = list.get();                
                initTableModes();
                bindModesFile();
            }

            file.close();
        }

//            par.saveSettings();
    }
}

void FormPasses::on_btnSaveModes_clicked() {
    if (!bindedModesFile() || par.projDir.length() == 0 || par.modesFileName.length() == 0)
        on_btnSaveAsModes_clicked();
    else {
        QFile file(par.projDir + "/" + par.modesFileName);

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            QMessageBox::critical(this, tr("Operation modes: Save file error"), file.errorString());
        else {
            GeneratorModeList list(par.genModesList);
            list.writeXML(file);
            file.close();
        }
    }
}

void FormPasses::on_btnSaveAsModes_clicked() {
    QDir dir(par.projDir);

    if (!dir.exists()) {
        par.projDir = QDir::homePath() + par.projDirDefault;
        dir = QDir(par.projDir);

        if (!dir.exists()) {
            bool OK = dir.mkpath(".");

            if (!OK)
                par.projDir.clear();
        }
    }

    QString new_filePath = QFileDialog::getSaveFileName(this, tr("Save modes file"),
                               par.projDir + "/" + par.modesFileName,
                               tr("Modes files") + " (*.xmd *.XMD)");

    if (new_filePath.length() != 0) {
        QFileInfo fInfo(new_filePath);
        par.projDir = fInfo.dir().path();
        par.modesFileName = fInfo.completeBaseName() + ".xmd";
        bindModesFile();
        on_btnSaveModes_clicked();
//            par.saveSettings();
    }
}

void FormPasses::on_btnOpen_clicked() {
    QDir dir(par.projDir);

    if (!dir.exists()) {
        par.projDir = QDir::homePath() + par.projDirDefault;
        dir = QDir(par.projDir);

        if (!dir.exists()) {
            bool OK = dir.mkpath(".");

            if (!OK)
                par.projDir.clear();
        }
    }

    QString selectedFilter = tr("Cutting settings files") + " (*.xpar *.XPAR)";
    QString new_filePath = QFileDialog::getOpenFileName(this, tr("Open cutting settings file"),
                                par.projDir + "/" + par.parFileName,
                                selectedFilter + ";;" + tr("All files") + " (*)",
                                &selectedFilter
                            );

    if (new_filePath.length()) {
        QFileInfo fInfo(new_filePath);
        par.projDir = fInfo.dir().path();
        par.parFileName = fInfo.fileName();

        QFile file(par.projDir + "/" + par.parFileName);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            QMessageBox::critical(this, tr("Cutting settings: Open file error"), file.errorString());
        else {
            QXmlStreamReader xml;
            xml.setDevice(&file);
            bool err = false;

            while (xml.tokenType() != QXmlStreamReader::StartDocument) {
                err = xml.atEnd() || xml.hasError();
                if (err) break;
                xml.readNext();
            }

            if (!err)
                err = !par.cutParam.read(xml);

            if (!err) {
                GeneratorModeList list(par.genModesList);
                err = !list.read(xml);
                if (!err) {
//                    QItemSelectionModel* m = tableMode->selectionModel();

                    par.genModesList = list.get();

//                    tableMode->setModel(new ModeTableModel(par.genModesList, this));
//                    delete m;
//                    tableMode->resizeColumnsToContents();
//                    initComboMode();
//                    bindModesFile();
                }
            }

            if (xml.hasError()) {
                QMessageBox::critical(this, tr("Cutting settings: Parse file error"), xml.errorString(), QMessageBox::Ok);
                return;
            }

            file.close();
            init();
        }
    }
}

void FormPasses::on_btnSave_clicked() {
    par.cutParam.D_ena = groupD->isChecked();

//    if (!bindedModesFile() || par.fileDir.length() == 0 || par.modesFileName.length() == 0)
    if (par.projDir.length() == 0 || par.parFileName.length() == 0)
        on_btnSaveAs_clicked();
    else {
        QFile file(par.projDir + "/" + par.parFileName);

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            QMessageBox::critical(this, tr("Cutting settigns: Save file error"), file.errorString());
        else {
            QXmlStreamWriter xml(&file);
            xml.setAutoFormatting(true);
            xml.writeStartDocument();
            xml.writeStartElement("param");
                par.cutParam.write(xml);
                GeneratorModeList(par.genModesList).write(xml);
            xml.writeEndElement();
            xml.writeEndDocument();

            file.close();
        }
    }
}

void FormPasses::on_btnSaveAs_clicked() {
    QDir dir(par.projDir);

    if (!dir.exists()) {
        par.projDir = QDir::homePath() + par.projDirDefault;
        dir = QDir(par.projDir);

        if (!dir.exists()) {
            bool OK = dir.mkpath(".");

            if (!OK)
                par.projDir.clear();
        }
    }

    QString new_filePath = QFileDialog::getSaveFileName(this, tr("Save cutting settings"),
                               par.projDir + "/" + par.parFileName,
                               tr("Cutting settings files") + " (*.xpar *.XPAR)");

    if (new_filePath.length() != 0) {
        QFileInfo fInfo(new_filePath);
        par.projDir = fInfo.dir().path();
        par.parFileName = fInfo.completeBaseName() + ".xpar";
//        bindModesFile();
        on_btnSave_clicked();
//            par.saveSettings();
    }
}

void FormPasses::createButtons() {
    btnBack = new QPushButton(tr("Back"));
    btnBack->setStatusTip(tr("Return to Contour editor") + "   Alt+Left");
    btnBack->setShortcut(QKeySequence::Back);

    btnOpen = new QPushButton(tr("Open"));
    btnOpen->setStatusTip(tr("Open cutting settings from file") + "   Ctrl+O");
    btnOpen->setShortcut(QKeySequence("Ctrl+O"));

    btnSave = new QPushButton(tr("Save"));
    btnSave->setStatusTip(tr("Save cutting settings") + "   Ctrl+S");
    btnSave->setShortcut(QKeySequence("Ctrl+S"));

    btnSaveAs = new QPushButton(tr("Save as"));
    btnSaveAs->setStatusTip(tr("Save cutting settings into a new file"));

    btnDefault = new QPushButton(tr("Default"));
    btnDefault->setStatusTip(tr("Set default values"));

    btnGen = new QPushButton(tr("Generate"));
    btnGen->setStatusTip(tr("Generate G-code"));

    btn6 = new QPushButton;
    btn6->setEnabled(false);

    btn7 = new QPushButton;
    btn7->setEnabled(false);

    btn8 = new QPushButton;
    btn8->setEnabled(false);

    btn9 = new QPushButton;
    btn9->setEnabled(false);

    btn10 = new QPushButton;
    btn10->setEnabled(false);

    btn11 = new QPushButton;
    btn11->setEnabled(false);

    btn5 = new QPushButton;
    btn5->setEnabled(false);

    btnHelp = new QPushButton(tr("Help"));
    btnHelp->setStatusTip(tr("Open Help") + "   F1");
    btnHelp->setShortcut(QKeySequence::HelpContents);

    gridButtons = new QGridLayout;

    gridButtons->addWidget(btnBack, 0, 0);
    gridButtons->addWidget(btnOpen, 0, 1);
    gridButtons->addWidget(btnSave, 0, 2);
    gridButtons->addWidget(btnSaveAs, 0, 3);
    gridButtons->addWidget(btnDefault, 0, 4);
    gridButtons->addWidget(btn5, 0, 5);
    gridButtons->addWidget(btn6, 0, 6);
    gridButtons->addWidget(btn7, 0, 7);
    gridButtons->addWidget(btn8, 0, 8);
    gridButtons->addWidget(btn9, 0, 9);
    gridButtons->addWidget(btn10, 0, 10);
    gridButtons->addWidget(btn11, 0, 11);
    gridButtons->addWidget(btnGen, 0, 12);
    gridButtons->addWidget(btnHelp, 0, 13);

    connect(btnBack, &QPushButton::clicked, this, [&]() { emit backPageClicked(); });
    connect(btnOpen, &QPushButton::clicked, this, &FormPasses::on_btnOpen_clicked);
    connect(btnSave, &QPushButton::clicked, this, &FormPasses::on_btnSave_clicked);
    connect(btnSaveAs, &QPushButton::clicked, this, &FormPasses::on_btnSaveAs_clicked);

    connect(btnDefault, &QPushButton::clicked, this, [&]() {
        par.setDefaultCutParam();
        init(m_uv_ena);
    });

    connect(btnGen, &QPushButton::clicked, this, [&]() {
        par.cutParam.D_ena = groupD->isChecked();
        emit generateClicked();
    });

    connect(btnHelp, &QPushButton::clicked, this, [&]() { emit helpPageClicked(help_file); });
}

void FormPasses::initComboMode() {
    QStringList newText;

    int modes_num = par.genModesList.size();

    for (int i = 0; i < modes_num; i++)
        newText.push_back(tr("Mode") + " " + QString::number(i + 1));

    for (uint i = 0; i < par.cutParam.MAX_TIMES; i++) {
        int index = comboModes[i]->currentIndex();

        comboModes[i]->clear();
        comboModes[i]->addItems(newText);

        if (index < modes_num)
            comboModes[i]->setCurrentIndex(index);
        else if (modes_num > 0)
            comboModes[i]->setCurrentIndex(modes_num - 1);
    }

    {
        int index = comboTabMode->currentIndex();
        comboTabMode->clear();
        comboTabMode->addItems(newText);

        if (index < modes_num)
            comboTabMode->setCurrentIndex(index);
        else if (modes_num > 0)
            comboTabMode->setCurrentIndex(modes_num - 1);
    }

    {
        int index = comboCutLineMode->currentIndex();
        comboCutLineMode->clear();
        comboCutLineMode->addItems(newText);

        if (index < modes_num)
            comboCutLineMode->setCurrentIndex(index);
        else if (modes_num > 0)
            comboCutLineMode->setCurrentIndex(modes_num - 1);
    }
}

void FormPasses::setCutTimes(int times) {
    par.cutParam.times = (times > 0) ? uint8_t(times) : 1;

    for (uint i = 0; i < par.cutParam.MAX_TIMES; i++) {
        if (i < par.cutParam.times) {
            inputOffsets[i]->setEnabled(true);

            uint modes_num = par.genModesList.size();

            if (i < modes_num)
                comboModes[i]->setCurrentIndex(i);
            else if (modes_num > 0)
                comboModes[i]->setCurrentIndex(modes_num - 1);

            comboModes[i]->setEnabled(true);
        }
        else {
            inputOffsets[i]->setEnabled(false);
            comboModes[i]->setEnabled(false);
        }
    }
}

//void FormPasses::on_tableMode_clicked(const QModelIndex& index) {
//    if (index.isValid())
//        m_row = index.row() < 0 ? 0 : index.row();
//}

FormPasses::~FormPasses() {
    if (labelPasses) {
        delete[] labelPasses;
        labelPasses = nullptr;
    }
    if (labelModes) {
        delete[] labelModes;
        labelModes = nullptr;
    }
    if (inputOffsets) {
        delete[] inputOffsets;
        inputOffsets = nullptr;
    }
    if (comboModes) {
        delete[] comboModes;
        comboModes = nullptr;
    }
}

void FormPasses::setFontPointSize(int pointSize) {
    QFont font;

    for (QPushButton* o: buttons) {
        font = o->font();
        font.setPointSize(pointSize);
        o->setFont(font);
    }

    for (QLabel* o: labels) {
        font = o->font();
        font.setPointSize(pointSize);
        o->setFont(font);
    }

    for (size_t i = 0; i < par.cutParam.MAX_TIMES; i++) {
        font = labelPasses[i]->font();
        font.setPointSize(pointSize);
        labelPasses[i]->setFont(font);

        font = labelModes[i]->font();
        font.setPointSize(pointSize);
        labelModes[i]->setFont(font);

        font = comboModes[i]->font();
        font.setPointSize(pointSize);
        comboModes[i]->setFont(font);

        font = inputOffsets[i]->font();
        font.setPointSize(pointSize);
        inputOffsets[i]->setFont(font);
    }

    for (QRadioButton* o: radio) {
        font = o->font();
        font.setPointSize(pointSize);
        o->setFont(font);
    }

    for (QComboBox* o: combo) {
        font = o->font();
        font.setPointSize(pointSize);
        o->setFont(font);
    }

    for (QCheckBox* o: checks) {
        font = o->font();
        font.setPointSize(pointSize);
        o->setFont(font);
    }

    for (QDoubleSpinBox* o: nums) {
        font = o->font();
        font.setPointSize(pointSize);
        o->setFont(font);
    }

    for (QGroupBox* o: groups) {
        font = o->font();
        font.setPointSize(pointSize);
        o->setFont(font);
    }

    font = inputPumpDelay->font();
    font.setPointSize(pointSize);
    inputPumpDelay->setFont(font);

    font = tableModes->font();
    font.setPointSize(pointSize);
    tableModes->setFont(font);
    resizeModeView();
}
