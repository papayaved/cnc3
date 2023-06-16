#include "slicing_widget.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QMessageBox>

#include "wire_cut.h"

SlicingWidget::SlicingWidget(ProgramParam& par, QWidget *parent) : QWidget(parent), par(par) {
    labelTitle = new QLabel(R"(<h1>)" + tr("Slicing") + R"(<br></h1>)");
    labelWireD = new QLabel(tr("Wire Diameter"));
    labelWidth = new QLabel(tr("Workpiece Thickness"));
    labelSpacing = new QLabel(tr("Spacing"));
    labelStep = new QLabel(tr("Slice Thickness") + ", " + tr("signed value"));
    labelStepNum = new QLabel(tr("Slices Number"));
    labelSpeedSections = new QLabel(tr("Number of sections in the speed profile"));
    labelSpeedAvg = new QLabel(tr("Cutting Speed"));

//    labels = {labelWireD, labelWidth, labelSpacing, labelSlicingStep, labelStepNum, labelSectionsNum, labelSpeedAvg};

    //
    fnumWireD = new QDoubleSpinBox;
    labelWireD->setBuddy(fnumWireD);
    fnumWireD->setSuffix(" " + tr("mm"));
    fnumWireD->setDecimals(3);
    fnumWireD->setSingleStep(0.001);
    fnumWireD->setRange(0.05, 3);
    fnumWireD->setAccelerated(true);
    fnumWireD->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    fnumWidth = new QDoubleSpinBox;
    labelWidth->setBuddy(fnumWidth);
    fnumWidth->setSuffix(" " + tr("mm"));
    fnumWidth->setDecimals(3);
    fnumWidth->setSingleStep(0.001);
    fnumWidth->setRange(1, 1000);    
    fnumWidth->setAccelerated(true);
    fnumWidth->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    fnumSpacing = new QDoubleSpinBox;
    labelSpacing->setBuddy(fnumSpacing);
    fnumSpacing->setSuffix(" " + tr("mm"));
    fnumSpacing->setDecimals(1);
    fnumSpacing->setSingleStep(1);
    fnumSpacing->setRange(1, 20);
    fnumSpacing->setAccelerated(true);
    fnumSpacing->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    fnumStep = new QDoubleSpinBox;
    labelStep->setBuddy(fnumStep);
    fnumStep->setSuffix(" " + tr("mm"));
    fnumStep->setDecimals(3);
    fnumStep->setSingleStep(0.001);
    fnumStep->setRange(-1000, 1000);
    fnumStep->setAccelerated(true);
    fnumStep->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    fnumSpeedAvg = new QDoubleSpinBox;
    labelSpeedAvg->setBuddy(fnumSpeedAvg);
    fnumSpeedAvg->setSuffix(" " + tr("mm/min"));
    fnumSpeedAvg->setDecimals(1);
    fnumSpeedAvg->setSingleStep(1);
    fnumSpeedAvg->setRange(0.1, cnc_param::SPEED_MAX);
    fnumSpeedAvg->setAccelerated(true);
    fnumSpeedAvg->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    //
    numSlicesNum = new QSpinBox;
    labelStepNum->setBuddy(numSlicesNum);
    numSlicesNum->setRange(1, 1000);    
    numSlicesNum->setAccelerated(true);
    numSlicesNum->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    numSpeedSections = new QSpinBox;
    labelSpeedSections->setBuddy(numSpeedSections);
    numSpeedSections->setRange(1, 100);
    numSpeedSections->setAccelerated(true);
    numSpeedSections->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    groupProfile = new QGroupBox(tr("Workpiece Profile"));
    radioRect = new QRadioButton(tr("Rectangle Profile"));
    radioCircle = new QRadioButton(tr("Round Profile"));
    {
        QVBoxLayout* vbox = new QVBoxLayout;
        vbox->addWidget(radioRect);
        vbox->addWidget(radioCircle);
        groupProfile->setLayout(vbox);
    }

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
    grid->addWidget(labelWidth, 2, 0);
    grid->addWidget(fnumWidth, 2, 1);
    grid->addWidget(labelSpacing, 3, 0);
    grid->addWidget(fnumSpacing, 3, 1);
    grid->addWidget(labelStep, 4, 0);
    grid->addWidget(fnumStep, 4, 1);
    grid->addWidget(labelStepNum, 5, 0);
    grid->addWidget(numSlicesNum, 5, 1);

    grid->addWidget(groupProfile, 6, 1);
    grid->addWidget(labelSpeedAvg, 7, 0);
    grid->addWidget(fnumSpeedAvg, 7, 1);
    grid->addWidget(labelSpeedSections, 8, 0);
    grid->addWidget(numSpeedSections, 8, 1);

    grid->addWidget(checkPause, 9, 0);
    grid->addWidget(numPause, 9, 1);

    grid->addWidget(groupAxis, 10, 1);
    grid->addWidget(btnReset, 11, 0, Qt::AlignLeft | Qt::AlignBottom);
    grid->addWidget(btnGen, 11, 1, Qt::AlignRight | Qt::AlignBottom);
    grid->addWidget(btnUndo, 12, 0, Qt::AlignLeft | Qt::AlignBottom);

    //
//    QWidget* widgetInside = new QWidget(this);
//    widgetInside->setLayout(grid);

    QGridLayout* gridCentral = new QGridLayout;
    gridCentral->addLayout(grid, 0, 0, Qt::AlignTop | Qt::AlignLeft);
    gridCentral->addWidget(new QFrame, 0, 1);
    gridCentral->addWidget(new QFrame, 1, 0, 1, 2);

//    QWidget* widgetCentral = new QWidget(this);
//    widgetCentral->setLayout(gridCentral);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addLayout(gridCentral);
    this->setLayout(mainLayout);

    connect(radioRect, &QRadioButton::clicked, this, [&]() {
        labelSpeedSections->setEnabled(false);
        numSpeedSections->setEnabled(false);
        labelWidth->setText(tr("Workpiece Thickness"));
        labelSpeedAvg->setText(tr("Cutting Speed"));
    });
    connect(radioCircle, &QRadioButton::clicked, this, [&]() {
        labelSpeedSections->setEnabled(true);
        numSpeedSections->setEnabled(true);
        labelWidth->setText(tr("Workpiece Diameter"));
        labelSpeedAvg->setText(tr("Average Speed"));
    });

    connect(btnUndo, &QPushButton::clicked, this, &SlicingWidget::onUndo);
    connect(btnReset, &QPushButton::clicked, this, &SlicingWidget::onReset);
    connect(btnGen, &QPushButton::clicked, this, &SlicingWidget::onGenerate);

    connect(checkPause, &QCheckBox::clicked, this, [&](bool checked) {
        numPause->setEnabled(checked);
    });

    onUndo();

    widgets = {
        labelTitle, labelWireD, labelWidth, labelSpacing, labelStep, labelStepNum, labelSpeedSections, labelSpeedAvg,
        fnumWireD, fnumWidth, fnumSpacing, fnumStep, fnumSpeedAvg,
        numSlicesNum, numSpeedSections, numPause,
        groupProfile,
        radioRect, radioCircle,
        checkPause,
        groupAxis,
        radioX, radioY,
        btnReset, btnUndo, btnGen
    };
}

void SlicingWidget::init() {
    fnumWireD->setValue(par.m_snake.wireD);
    fnumWidth->setValue(par.m_snake.width);
    fnumSpacing->setValue(par.m_snake.spacing);
    fnumStep->setValue(par.m_snake.step);
    fnumSpeedAvg->setValue(par.m_snake.speed_avg);
    numSlicesNum->setValue(par.m_snake.slices_num);
    numSpeedSections->setValue(par.m_snake.sections_num);

    if (par.m_snake.profile == SLICING_PROFILE::ROUND)
        radioCircle->click();
    else
        radioRect->click();

    checkPause->setChecked(par.m_snake.pause_ena);
    numPause->setEnabled(par.m_snake.pause_ena);
    numPause->setValue(par.m_snake.pause);

    if (par.m_snake.axis == AXIS::AXIS_Y)
        radioY->click();
    else
        radioX->click();
}

snake_t SlicingWidget::snake() const {
    snake_t res;
    res.slices_num          = numSlicesNum->value();
    res.sections_num        = numSpeedSections->value();
    res.wireD               = fnumWireD->value();
    res.width               = fnumWidth->value();
    res.spacing             = fnumSpacing->value();
    res.step                = fnumStep->value();
    res.speed_avg           = fnumSpeedAvg->value();
    res.profile             = radioCircle->isChecked() ? SLICING_PROFILE::ROUND : SLICING_PROFILE::RECT;
    res.pause_ena           = checkPause->isChecked();
    res.pause               = numPause->value();
    res.axis                = radioY->isChecked() ? AXIS::AXIS_Y : AXIS::AXIS_X;
    res.speed_idle          = cnc_param::SPEED_MAX;
    res.roll_vel            = cnc_param::ROLL_MAX;
    res.roll_vel_idle       = cnc_param::ROLL_MIN;

    return res;
}

void SlicingWidget::onUndo() {
    snake_t snake;
    par.loadSlices(snake);
    init();
}

void SlicingWidget::onReset() {
    par.m_snake = snake_t();
    init();
}

void SlicingWidget::onGenerate() {
    try {
        btnGen->blockSignals(true);
        btnGen->setEnabled(false);

        Contour bot;
        par.saveSlices( snake() );
        init();

        bot.generate(par.m_snake);

        par.contours.clear();
        par.contours.new_back();
        par.contours.back()->setBot(bot);

        ContourPair* pair = par.contours.front();
        std::deque<ContourPair> ar;
        ar.push_back(*pair);

        if (par.gcode.generate( par.cutParam, ar, CncParam() )) {
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
        qDebug("SlicingWidget::onGenerate>>%s", ex.what());
        QMessageBox::critical(this, tr("Slicing generate error"), ex.what());
    } catch(...) {
        qDebug("SlicingWidget::onGenerate>>%s", std::exception().what());
        QMessageBox::critical(this, tr("Slicing generate error"), std::exception().what());
    }

    btnGen->setEnabled(true);
    btnGen->blockSignals(false);
}

void SlicingWidget::setFontPointSize(QWidget *w, int pointSize) {
    QFont font = w->font();
    font.setPointSize(pointSize);
    w->setFont(font);
}

void SlicingWidget::setFontPointSize(int pointSize) {
    for (QWidget* w: widgets)
        setFontPointSize(w, pointSize);
}
