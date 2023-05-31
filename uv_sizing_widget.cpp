#include "uv_sizing_widget.h"

#include <QVBoxLayout>
#include <QGridLayout>

//#include "contour_pass.h"
//#include "wire_cut.h"

UVSizingWidget::UVSizingWidget(ProgramParam& par, QWidget *parent) : QWidget(parent), par(par) {
    labelMeasuring = new QLabel(R"(<h1><br>)" + tr("Size measuring for tapered cutting") + R"(</h1>)");
    groupMeasuring = new QGroupBox;
    labelWireD = new QLabel(tr("Wire diameter") + ":");
    labelUMax = new QLabel(tr("Maximum U, V motors travel") + " (UV_MAX):");
    labelL = new QLabel(tr("Distance between rollers") + " (L):");
    labelH = new QLabel(tr("Workpiece bottom height") + " (H):");
    labelT = new QLabel(tr("Workpiece thickness") + " (T):");
    labelRollerD = new QLabel(tr("Roller diameter") + ":");
    labelRollerPlane = new QLabel(tr("Roller plane") + ":");
    labelEntryLine = new QLabel(tr("Entry line length") + ":");

    btnReset = new QPushButton(tr("Reset"));
    btnReset->setStatusTip(tr("Set default values"));

    btnGen = new QPushButton(tr("Generate"));
    btnGen->setStatusTip(tr("Generate contour"));

    labelRecalc = new QLabel(R"(<h2><br>)" + tr("Recalculate") + R"(</h2>)");
    groupRecalc = new QGroupBox;
    labelDTop = new QLabel(tr("Measured top diameter") + ":");
    labelDBot = new QLabel(tr("Measured bottom diameter") + ":");

    btnRecalc = new QPushButton(tr("Recalculate"));

    labelResult = new QLabel(R"(<h2><br>)" + tr("Result") + R"(</h2>)");
    groupResult = new QGroupBox;
    labelResL = new QLabel("L:");
    labelResH = new QLabel("H:");

    fnumWireD = new QDoubleSpinBox;
    labelWireD->setBuddy(fnumWireD);
    fnumWireD->setSuffix(" " + tr("mm"));
    fnumWireD->setDecimals(3);
    fnumWireD->setSingleStep(0.001);
    fnumWireD->setRange(0, 3);    

    fnumUMax = new QDoubleSpinBox;
    labelUMax->setBuddy(fnumUMax);
    fnumUMax->setSuffix(" " + tr("mm"));
    fnumUMax->setDecimals(0);
    fnumUMax->setSingleStep(1);
    fnumUMax->setRange(0, 1000);    

    fnumL = new QDoubleSpinBox;
    labelL->setBuddy(fnumL);
    fnumL->setSuffix(" " + tr("mm"));
    fnumL->setDecimals(3);
    fnumL->setSingleStep(0.001);
    fnumL->setRange(0, 1000);    

    fnumH = new QDoubleSpinBox;
    labelH->setBuddy(fnumH);
    fnumH->setSuffix(" " + tr("mm"));
    fnumH->setDecimals(3);
    fnumH->setSingleStep(0.001);
    fnumH->setRange(0, 200);

    fnumT = new QDoubleSpinBox;
    labelT->setBuddy(fnumT);
    fnumT->setSuffix(" " + tr("mm"));
    fnumT->setDecimals(3);
    fnumT->setSingleStep(0.001);
    fnumT->setRange(0, 200);

    checkDEna = new QCheckBox;

    QHBoxLayout *hbox_D = new QHBoxLayout;
    hbox_D->addWidget(checkDEna);
    hbox_D->addWidget(labelRollerD);

    fnumDRoller = new QDoubleSpinBox;
    labelRollerD->setBuddy(fnumDRoller);
    fnumDRoller->setSuffix(" " + tr("mm"));
    fnumDRoller->setDecimals(3);
    fnumDRoller->setSingleStep(0.1);
    fnumDRoller->setRange(0, 200);

    groupRollerPlane = new QGroupBox;
    radioXZ = new QRadioButton("XZ");
    radioYZ = new QRadioButton("YZ");
    {
        QHBoxLayout* hbox = new QHBoxLayout;
        hbox->addWidget(radioXZ);
        hbox->addWidget(radioYZ);
        groupRollerPlane->setLayout(hbox);
    }

    fnumLine = new QDoubleSpinBox;
    labelEntryLine->setBuddy(fnumLine);
    fnumLine->setSuffix(" " + tr("mm"));
    fnumLine->setDecimals(0);
    fnumLine->setSingleStep(1);
    fnumLine->setRange(0, 100);

    groupAxis = new QGroupBox(tr("Cutting direction"));
    radioXp = new QRadioButton("X+");
    radioXn = new QRadioButton("X-");
    radioYp = new QRadioButton("Y+");
    radioYn = new QRadioButton("Y-");
    {
        QHBoxLayout* hbox = new QHBoxLayout;
        hbox->addWidget(radioXp);
        hbox->addWidget(radioXn);
        hbox->addWidget(radioYp);
        hbox->addWidget(radioYn);
        groupAxis->setLayout(hbox);
    }

    QGridLayout* gridMeasuring = new QGridLayout;
    gridMeasuring->addWidget(labelWireD, 0, 0, Qt::AlignRight);
    gridMeasuring->addWidget(fnumWireD, 0, 1);
    gridMeasuring->addWidget(labelUMax, 1, 0, Qt::AlignRight);
    gridMeasuring->addWidget(fnumUMax, 1, 1);
    gridMeasuring->addWidget(labelL, 2, 0, Qt::AlignRight);
    gridMeasuring->addWidget(fnumL, 2, 1);
    gridMeasuring->addWidget(labelH, 3, 0, Qt::AlignRight);
    gridMeasuring->addWidget(fnumH, 3, 1);
    gridMeasuring->addWidget(labelT, 4, 0, Qt::AlignRight);
    gridMeasuring->addWidget(fnumT, 4, 1);
    gridMeasuring->addLayout(hbox_D, 5, 0, Qt::AlignRight);
    gridMeasuring->addWidget(fnumDRoller, 5, 1);
    gridMeasuring->addWidget(labelRollerPlane, 5, 2);
    gridMeasuring->addWidget(groupRollerPlane, 5, 3, 1, 2);
    gridMeasuring->addWidget(labelEntryLine, 6, 0, Qt::AlignRight);
    gridMeasuring->addWidget(fnumLine, 6, 1);
    gridMeasuring->addWidget(groupAxis, 7, 0, 1, 2);

    gridMeasuring->addWidget(btnReset, 8, 0, Qt::AlignLeft | Qt::AlignBottom);
    gridMeasuring->addWidget(btnGen, 8, 1, Qt::AlignRight | Qt::AlignBottom);

    groupMeasuring->setLayout(gridMeasuring);

    fnumDTop = new QDoubleSpinBox;
    labelDTop->setBuddy(fnumDTop);
    fnumDTop->setSuffix(" " + tr("mm"));
    fnumDTop->setDecimals(3);
    fnumDTop->setSingleStep(0.001);
    fnumDTop->setRange(0, 100);
    fnumDTop->setValue(0);

    fnumDBot = new QDoubleSpinBox;
    labelDBot->setBuddy(fnumDBot);
    fnumDBot->setSuffix(" " + tr("mm"));
    fnumDBot->setDecimals(3);
    fnumDBot->setSingleStep(0.001);
    fnumDBot->setRange(0, 100);
    fnumDBot->setValue(0);

    QGridLayout* gridRecalc = new QGridLayout;
    gridRecalc->addWidget(labelDTop, 0, 0, 1, 2, Qt::AlignRight);
    gridRecalc->addWidget(fnumDTop, 0, 2, Qt::AlignLeft);
    gridRecalc->addWidget(labelDBot, 1, 0, 1, 2, Qt::AlignRight);
    gridRecalc->addWidget(fnumDBot, 1, 2, Qt::AlignLeft);
    gridRecalc->addWidget(btnRecalc, 2, 2);

    gridRecalc->addWidget(new QFrame, 0, 3, 3, 1); // padding

    groupRecalc->setLayout(gridRecalc);

    fnumResL = new QDoubleSpinBox;
    labelResL->setBuddy(fnumResL);
    fnumResL->setSuffix(" " + tr("mm"));
    fnumResL->setDecimals(3);
    fnumResL->setSingleStep(0.001);
    fnumResL->setRange(-9999, 9999);
    fnumResL->setValue(0);
    fnumResL->setReadOnly(true);

    fnumResH = new QDoubleSpinBox;
    labelResH->setBuddy(fnumResH);
    fnumResH->setSuffix(" " + tr("mm"));
    fnumResH->setDecimals(3);
    fnumResH->setSingleStep(0.001);
    fnumResH->setRange(-9999, 9999);
    fnumResH->setValue(0);
    fnumResH->setReadOnly(true);

    QGridLayout* gridResult = new QGridLayout;
    gridResult->addWidget(labelResL, 0, 0, Qt::AlignRight);
    gridResult->addWidget(fnumResL, 0, 1, Qt::AlignLeft);
    gridResult->addWidget(labelResH, 1, 0, Qt::AlignRight);
    gridResult->addWidget(fnumResH, 1, 1, Qt::AlignLeft);

    groupResult->setLayout(gridResult);

    // Layout
    QGridLayout* grid = new QGridLayout;
    grid->addWidget(labelMeasuring, 0, 0, Qt::AlignHCenter | Qt::AlignBottom);
    grid->addWidget(groupMeasuring, 1, 0);
    grid->addWidget(labelRecalc, 2, 0, Qt::AlignHCenter | Qt::AlignBottom);
    grid->addWidget(groupRecalc, 3, 0);
    grid->addWidget(labelResult, 4, 0, Qt::AlignHCenter | Qt::AlignBottom);
    grid->addWidget(groupResult, 5, 0);

    QGridLayout* gridCentral = new QGridLayout;
    gridCentral->addLayout(grid, 0, 0, Qt::AlignTop | Qt::AlignLeft);
    gridCentral->addWidget(new QFrame, 0, 1);
    gridCentral->addWidget(new QFrame, 1, 0, 1, 2);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addLayout(gridCentral);
    this->setLayout(mainLayout);

    connect(btnReset, &QPushButton::clicked, this, &UVSizingWidget::onReset);
    connect(btnGen, &QPushButton::clicked, this, &UVSizingWidget::onGenerate);
    connect(btnRecalc, &QPushButton::clicked, this, &UVSizingWidget::onRecalc);

    connect(checkDEna, &QCheckBox::clicked, this, [&](bool checked) {
        labelRollerD->setEnabled(checked);
        fnumDRoller->setEnabled(checked);
        labelRollerPlane->setEnabled(checked);
        groupRollerPlane->setEnabled(checked);
    });

    widgets = {
        labelWireD, labelUMax, labelL, labelH, labelT, labelEntryLine, labelDTop, labelDBot, labelResL, labelResH,
        fnumWireD, fnumUMax, fnumL, fnumH, fnumT, fnumLine, fnumDTop, fnumDBot, fnumResL, fnumResH,
        groupMeasuring, groupRecalc, groupResult,
        btnReset, btnGen, btnRecalc,
        checkDEna,
        groupAxis, radioXp, radioXn, radioYp, radioYn
    };

    onReset();
}

void UVSizingWidget::onReset() {
    fnumWireD->setValue(cnc_param::WIRE_DIAMETER);
    fnumUMax->setValue(15);
    fnumL->setValue(cnc_param::L_DEFAULT);
    fnumH->setValue(cnc_param::H_DEFAULT);
    fnumT->setValue(cnc_param::T_DEFAULT);

    checkDEna->setChecked(true);
    checkDEna->click(); // inverse

    fnumDRoller->setValue(cnc_param::D_DEFAULT);

    cnc_param::AXIS_DEFAULT == AXIS::AXIS_Y ? radioYZ->setChecked(true) : radioXZ->setChecked(true);

    fnumLine->setValue(5);
    radioYn->setChecked(true);
}

void UVSizingWidget::onGenerate() {
    uv_sizing_t p = getSizingPar();
    uv_dia_t dia = calcDia(p);

    fnumDTop->setValue(dia.top - p.D_wire / 2);
    fnumDBot->setValue(dia.bot - p.D_wire / 2);

    bool D_ena = checkDEna->isChecked();
    double D = fnumDRoller->value() - fnumWireD->value() / 2;
    AXIS roller_axis = radioYZ->isChecked() ? AXIS::AXIS_Y : AXIS::AXIS_X;

    double cutline = fnumLine->value();
    AXIS cutline_axis = radioYp->isChecked() || radioYn->isChecked() ? AXIS::AXIS_Y : AXIS::AXIS_X;

    if (radioXn->isChecked() || radioYn->isChecked())
        cutline = -cutline;

    if (par.gcode.generate(dia.top, dia.bot, p.L, p.H, p.T, D_ena, D, roller_axis, cutline, cutline_axis)) {
        par.gcodeText = par.gcode.toText().c_str();
        par.gcodeSettings = par.gcode.getSettings();

        if (!par.gcode.empty())
            emit clicked();
    }
}

void UVSizingWidget::onRecalc() {
    uv_sizing_t p = getSizingPar();

    uv_dia_t dia = {0,0};
    dia.top = fnumDTop->value() + p.D_wire / 2;
    dia.bot = fnumDBot->value() + p.D_wire / 2;

    uv_result_t res = recalc(p, dia);
    fnumResL->setValue(res.L);
    fnumResH->setValue(res.H);
}

uv_sizing_t UVSizingWidget::getSizingPar() const {
    uv_sizing_t res;
    res.D_wire = fnumWireD->value();
    res.uv_max = fnumUMax->value();
    res.L = fnumL->value();
    res.H = fnumH->value();
    res.T = fnumT->value();
    return res;
}

uv_dia_t UVSizingWidget::calcDia(const uv_sizing_t &p) {
    uv_dia_t res;
    res.top = (p.L - p.H - p.T) * p.uv_max / p.L;
    res.bot = (p.L - p.H + p.T) * p.uv_max / p.L;
    return res;
}

double UVSizingWidget::calc_dx(double H, double L, double u_max) {
    return H * u_max / L;
}

uv_result_t UVSizingWidget::recalc(const uv_sizing_t &p, uv_dia_t d) {
    uv_result_t res;
    res.L = 2.0 * p.uv_max * p.T / (d.bot - d.top);
    res.H = p.H / p.L * res.L;
    return res;
}

void UVSizingWidget::setFontPointSize(QWidget *w, int pointSize) {
    QFont font = w->font();
    font.setPointSize(pointSize);
    w->setFont(font);
}

void UVSizingWidget::setFontPointSize(int pointSize) {
    for (QWidget* w: widgets)
        setFontPointSize(w, pointSize);
}
