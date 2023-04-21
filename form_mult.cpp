#include "form_mult.h"

#include <QSpacerItem>
#include <QMessageBox>

#include <algorithm>
#include <functional>
#include <array>

#include "cnc_types.h"

using namespace std;

FormMult::FormMult(ProgramParam& par, QWidget *parent) : QWidget(parent), par(par) {
    this->setObjectName(tr("Contour multiplication"));
    labelTitle = new QLabel(R"(<h1>)" + tr("Contour multiplication") + R"(</h1>)");
    labelTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    createMux();
    createButtons();

    init();

    gridMuxTitle = new QGridLayout;

    gridMuxTitle->addWidget(labelTitle, 0, 0, Qt::AlignHCenter);
    gridMuxTitle->addLayout(gridMux, 1, 0, Qt::AlignVCenter | Qt::AlignHCenter);
    gridMuxTitle->setSizeConstraint(QLayout::SetFixedSize);

    mainLayout = new QVBoxLayout;
    mainLayout->addLayout(gridMuxTitle);
    mainLayout->addLayout(gridButtons);

    this->setLayout(mainLayout);

    widgets = {
        labelTitle, labelRowDir, radioX, radioY, groupDir,
        labelItems, labelRows,
        numItems, numRows,
        labelItemsGap, labelRowsGap,
        fnumItemsGap, fnumRowsGap,
        checkAutoSplit,
        labelSplit[0], labelSplit[1], labelSplit[2], labelSplit[3],
        numSplit[0], numSplit[1], numSplit[2], numSplit[3],
        labelEntryLen, labelExitLen,
        fnumEntryLen, fnumExitLen,
        btnOK, btnCancel, btn2, btn3, btn4, btn5, btn6, btn7, btn8, btn9, btn10, btn11, btnDefault, btnHelp
    };

    setDefault();
}

void FormMult::createMux() {
    labelRowDir = new QLabel(tr("Row direction") + ": ");

    radioX = new QRadioButton("X");
    radioY = new QRadioButton("Y");

    groupDir = new QGroupBox;
    labelRowDir->setBuddy(groupDir);
    groupDir->setLayout(new QHBoxLayout);
    groupDir->layout()->addWidget(radioX);
    groupDir->layout()->addWidget(radioY);

    groupDir->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    labelItems = new QLabel(tr("Parts per row") + ": ");
    numItems = new QSpinBox;
    labelItems->setBuddy(numItems);
    numItems->setRange(2, 100);    
    numItems->setAccelerated(true);
    numItems->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    labelRows = new QLabel(tr("Rows") + ": ");
    numRows = new QSpinBox;
    labelRows->setBuddy(numRows);
    numRows->setRange(1, 100);
    numRows->setAccelerated(true);
    numRows->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    labelItemsGap = new QLabel(tr("Gap between parts in each row, signed") + ": ");
    fnumItemsGap = new QDoubleSpinBox;
    labelItemsGap->setBuddy(fnumItemsGap);
    fnumItemsGap->setSuffix(" mm");
    fnumItemsGap->setDecimals(0);
    fnumItemsGap->setSingleStep(1);
    fnumItemsGap->setRange(-100, 100);
    fnumItemsGap->setAccelerated(true);
    fnumItemsGap->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    labelRowsGap = new QLabel(tr("Gap between rows, signed") + ": ");
    fnumRowsGap = new QDoubleSpinBox;
    labelRowsGap->setBuddy(fnumRowsGap);
    fnumRowsGap->setSuffix(" mm");
    fnumRowsGap->setDecimals(0);
    fnumRowsGap->setSingleStep(1);
    fnumRowsGap->setRange(-100, 100);
    fnumRowsGap->setAccelerated(true);
    fnumRowsGap->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    checkAutoSplit = new QCheckBox(tr("Auto choice in the contour 4 junction verteces"));

    labelSplit[0] = new QLabel(tr("-X junction point is at the beginning of the segment") + ": ");
    labelSplit[1] = new QLabel(tr("+X junction point is at the beginning of the segment") + ": ");
    labelSplit[2] = new QLabel(tr("+Y junction point is at the beginning of the segment") + ": ");
    labelSplit[3] = new QLabel(tr("-Y junction point is at the beginning of the segment") + ": ");

    for (size_t i = 0; i < SPLIT_POINTS; i++) {
        numSplit[i] = new QSpinBox;
        labelSplit[i]->setBuddy(numSplit[i]);
        numSplit[i]->setRange(1, SPLIT_POINTS);
        numSplit[i]->setValue(i + 1);
        numSplit[i]->setAccelerated(true);
        numSplit[i]->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
    }

    labelEntryLen = new QLabel(tr("Entry Line Length") + ": ");
    fnumEntryLen = new QDoubleSpinBox;
    labelEntryLen->setBuddy(fnumEntryLen);
    fnumEntryLen->setSuffix(" mm");
    fnumEntryLen->setDecimals(0);
    fnumEntryLen->setSingleStep(1);
    fnumEntryLen->setRange(0, 100);    
    fnumEntryLen->setAccelerated(true);
    fnumEntryLen->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

    labelExitLen = new QLabel(tr("Exit Line Length") + ": ");
    fnumExitLen = new QDoubleSpinBox;
    labelExitLen->setBuddy(fnumExitLen);
    fnumExitLen->setSuffix(" mm");
    fnumExitLen->setDecimals(0);
    fnumExitLen->setSingleStep(1);
    fnumExitLen->setRange(0, 100);
    fnumExitLen->setAccelerated(true);
    fnumExitLen->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

//    QFrame* hLine[5];
//    for (size_t i = 0; i < sizeof(hLine)/sizeof(hLine[0]); i++) {
//        hLine[i] = new QFrame;
//        hLine[i]->setFrameShape(QFrame::HLine);
//        hLine[i]->setFrameShadow(QFrame::Sunken);
//        hLine[i]->setLineWidth(3);
//    }

    gridMux = new QGridLayout;
    gridMux->addWidget(labelRowDir, 0, 0, Qt::AlignRight);
    gridMux->addWidget(groupDir, 0, 1, 1, 3, Qt::AlignLeft);

    gridMux->addWidget(labelItems, 1, 0, Qt::AlignRight);
    gridMux->addWidget(numItems, 1, 1);
    gridMux->addWidget(labelRows, 1, 2, Qt::AlignRight);
    gridMux->addWidget(numRows, 1, 3);

    gridMux->addWidget(labelItemsGap, 2, 0, Qt::AlignRight);
    gridMux->addWidget(fnumItemsGap, 2, 1);

    gridMux->addWidget(labelRowsGap, 2, 2, Qt::AlignRight);
    gridMux->addWidget(fnumRowsGap, 2, 3);

    gridMux->addWidget(checkAutoSplit, 3, 0, 1, 4);

    gridMux->addWidget(labelSplit[0], 4, 0, Qt::AlignRight);
    gridMux->addWidget(numSplit[0], 4, 1);
    gridMux->addWidget(labelSplit[1], 5, 0, Qt::AlignRight);
    gridMux->addWidget(numSplit[1], 5, 1);
    gridMux->addWidget(labelSplit[2], 6, 0, Qt::AlignRight);
    gridMux->addWidget(numSplit[2], 6, 1);
    gridMux->addWidget(labelSplit[3], 7, 0, Qt::AlignRight);
    gridMux->addWidget(numSplit[3], 7, 1);

    gridMux->addWidget(labelEntryLen, 8, 0, Qt::AlignRight);
    gridMux->addWidget(fnumEntryLen, 8, 1);

    gridMux->addWidget(labelExitLen, 9, 0, Qt::AlignRight);
    gridMux->addWidget(fnumExitLen, 9, 1);

    connect(checkAutoSplit, &QCheckBox::clicked, this, &FormMult::on_checkAutoSplit_clicked);
}

void FormMult::init(size_t ctr_num) {
    m_ctr_num = ctr_num < par.contours.size() ? ctr_num : 0;

    const ContourPair* pair = par.contours.at(m_ctr_num);

    size_t n = pair ? pair->count() + 1 : 1;

    for (size_t i = 0; i < SPLIT_POINTS; i++)
        numSplit[i]->setMaximum(n);

    checkAutoSplit->setCheckState(Qt::CheckState::Checked);
    splitEnable(false);
    setSplitPoints();
}

void FormMult::createButtons() {
    btnOK = new QPushButton("OK");
    btnOK->setStatusTip(tr("Generate multiplied contours and return to the Contour Editor"));

    btnCancel = new QPushButton(tr("Cancel"));
    btnCancel->setStatusTip(tr("Return to the Contour Editor without any changes") + "   Alt+Left");
    btnCancel->setShortcut(QKeySequence::Back);

    btn2 = new QPushButton;
    btn2->setEnabled(false);

    btn3 = new QPushButton;
    btn3->setEnabled(false);

    btn4 = new QPushButton;
    btn4->setEnabled(false);

    btn5 = new QPushButton;
    btn5->setEnabled(false);

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

    btnDefault = new QPushButton(tr("Default"));
    btnDefault->setStatusTip(tr("Set default values"));

    btnHelp = new QPushButton(tr("Help"));
    btnHelp->setStatusTip(tr("Open Help") + "   F1");
    btnHelp->setShortcut(QKeySequence::HelpContents);

    gridButtons = new QGridLayout;

    gridButtons->addWidget(btnOK, 0, 0);
    gridButtons->addWidget(btnCancel, 0, 1);
    gridButtons->addWidget(btn2, 0, 2);
    gridButtons->addWidget(btn3, 0, 3);
    gridButtons->addWidget(btn4, 0, 4);
    gridButtons->addWidget(btn5, 0, 5);
    gridButtons->addWidget(btn6, 0, 6);
    gridButtons->addWidget(btn7, 0, 7);
    gridButtons->addWidget(btn8, 0, 8);
    gridButtons->addWidget(btn9, 0, 9);
    gridButtons->addWidget(btn10, 0, 10);
    gridButtons->addWidget(btn11, 0, 11);
    gridButtons->addWidget(btnDefault, 0, 12);
    gridButtons->addWidget(btnHelp, 0, 13);

    connect(btnCancel, &QPushButton::clicked, this, [&]() { emit backPageClicked(); });

    connect(btnDefault, &QPushButton::clicked, this, [&]() {
        setDefault();
        init(m_ctr_num);
    });

    connect(btnOK, &QPushButton::clicked, this, [&]() {
        on_btnOK_clicked();
        emit backPageClicked();
    });

    connect(btnHelp, &QPushButton::clicked, this, [&]() { emit helpPageClicked(help_file); });
}

void FormMult::splitEnable(bool ena) {
    for (size_t i = 0; i < SPLIT_POINTS; i++) {
        labelSplit[i]->setEnabled(ena);
        numSplit[i]->setEnabled(ena);
    }
}

FormMult::~FormMult() {

}

void FormMult::setFontPointSize(int pointSize) {
    QFont font;

    for (QWidget* o: widgets) {
        font = o->font();
        font.setPointSize(pointSize);
        o->setFont(font);
    }
}

size_t FormMult::items() const { return numItems->value() > 0 ? numItems->value() : 0; }
size_t FormMult::rows() const { return numRows->value() > 0 ? numRows->value() : 0; }

AXIS FormMult::axis() const { return radioY->isChecked() ? AXIS::AXIS_Y : AXIS::AXIS_X; }

DIR FormMult::items_dir() const { return fnumItemsGap->value() < 0 ? DIR::DIR_MINUS : DIR::DIR_PLUS; }
DIR FormMult::rows_dir() const { return fnumRowsGap->value() < 0 ? DIR::DIR_MINUS : DIR::DIR_PLUS; }

double FormMult::gapItems() const { return fabs(fnumItemsGap->value()); }
double FormMult::gapRows() const { return fabs(fnumRowsGap->value()); }

double FormMult::lengthEntry() const { return fnumEntryLen->value() > 0 ? fnumEntryLen->value() : 0 ; }

double FormMult::lengthExit() const { return fnumExitLen->value() > 0 ? fnumExitLen->value() : 0 ; }

void FormMult::on_btnOK_clicked() {
    const ContourPair* const pair = par.contours.at(m_ctr_num);

    if (pair && pair->isSorted() && pair->isLoop()) {
        bool changed = par.contours.generateMux(
                    pair,
                    items(), rows(),
                    axis(), items_dir(), rows_dir(),
                    getSplitPoints(),
                    gapItems(), gapRows(),
                    lengthEntry(), lengthExit()
        );

        if (changed)
            par.contours.saveUndo();
    }
}

void FormMult::setSplitPoints(const std::vector<size_t> &pts_num) {
    if (pts_num.size() == SPLIT_POINTS)
        for (size_t i = 0; i < SPLIT_POINTS; i++)
            numSplit[i]->setValue(pts_num[i] + 1);
}

std::vector<size_t> FormMult::getSplitPoints() {
    std::vector<size_t> res(4, 0);

    if (!checkAutoSplit->isChecked()) {
        res = par.contours.getSplitPoints(m_ctr_num, axis(), items_dir(), rows_dir());
        setSplitPoints(res);
        return res;
    }

    for (size_t i = 0; i < SPLIT_POINTS; i++)
        res[i] = numSplit[i]->value() - 1;

    return res;
}

void FormMult::setSplitPoints() {
    vector<size_t> split_pts = par.contours.getSplitPoints(m_ctr_num, axis(), items_dir(), rows_dir());
    setSplitPoints(split_pts);
}

void FormMult::on_checkAutoSplit_clicked(bool checked) {
    if (checked) {
        splitEnable(false);
        setSplitPoints();
    } else
        splitEnable(true);
}

void FormMult::setDefault() {
    radioX->setChecked(true);

    checkAutoSplit->setCheckState(Qt::CheckState::Unchecked);

    numItems->setValue(5);
    numRows->setValue(2);
    fnumItemsGap->setValue(5);
    fnumRowsGap->setValue(-5);

    fnumEntryLen->setValue(10);
    fnumExitLen->setValue(10);
}
