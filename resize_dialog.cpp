#include "resize_dialog.h"

ResizeDialog::ResizeDialog(double width, double height, const fpoint_valid_t& C, QWidget* parent) : QDialog(parent) {
    labelWidth = new QLabel(tr("Width"));
    labelHeight = new QLabel(tr("Height"));
    labelBaseSize = new QLabel(tr("Size"));
    labelNewSize = new QLabel(tr("New size"));

    inPct = new QDoubleSpinBox;
    inPct->setRange(0, 200);
    inPct->setValue(100);
    inPct->setSuffix(" %");
    inPct->setSingleStep(1);
    inPct->setDecimals(3);

    inWidth = new QDoubleSpinBox;
    inWidth->setRange(0, 1000);
    inWidth->setValue(width);
    inWidth->setSuffix(" " + tr("mm"));
    inWidth->setSingleStep(1);
    inWidth->setDecimals(3);

    inHeight = new QDoubleSpinBox;
    inHeight->setRange(0, 1000);
    inHeight->setValue(height);
    inHeight->setSuffix(" " + tr("mm"));
    inHeight->setSingleStep(1);
    inHeight->setDecimals(3);

    inBaseSize = new QDoubleSpinBox;
    inBaseSize->setRange(0, 1000);
    inBaseSize->setValue(100);
    inBaseSize->setSuffix(" " + tr("mm"));
    inBaseSize->setSingleStep(1);
    inBaseSize->setDecimals(3);

    inNewSize = new QDoubleSpinBox;
    inNewSize->setRange(0, 1000);
    inNewSize->setValue(100);
    inNewSize->setSuffix(" " + tr("mm"));
    inNewSize->setSingleStep(1);
    inNewSize->setDecimals(3);

    labelWidth->setBuddy(inWidth);
    labelHeight->setBuddy(inHeight);
    labelBaseSize->setBuddy(inBaseSize);
    labelNewSize->setBuddy(inNewSize);

    radioPct = new QRadioButton;
    radioRect = new QRadioButton;
    radioRatio = new QRadioButton;

    QGridLayout* grid = new QGridLayout;
    grid->addWidget(radioPct,0,0);
    grid->addWidget(inPct,0,2);

    grid->addWidget(radioRect,1,0);
    grid->addWidget(labelWidth,1,1);
    grid->addWidget(inWidth,1,2);
    grid->addWidget(labelHeight,1,3);
    grid->addWidget(inHeight,1,4);

    grid->addWidget(radioRatio,2,0);
    grid->addWidget(labelBaseSize,2,1);
    grid->addWidget(inBaseSize,2,2);
    grid->addWidget(labelNewSize,2,3);
    grid->addWidget(inNewSize,2,4);

    grid->setColumnStretch(0, 0);
    grid->setColumnStretch(1, 1);
    grid->setColumnStretch(2, 0);
    grid->setColumnStretch(3, 1);

    group = new QGroupBox;
    group->setLayout(grid);

    radioPct->setChecked(true);
    inPct->setEnabled(true);
    inWidth->setEnabled(false);
    inHeight->setEnabled(false);
    inBaseSize->setEnabled(false);
    inNewSize->setEnabled(false);

    // Center
    groupCenter = new QGroupBox;
    groupCenter->setCheckable(true);
    groupCenter->setChecked(C.valid);
    groupCenter->setTitle(tr("Center"));

    labelCx = new QLabel("x");
    labelCy = new QLabel("y");

    inCx = new QDoubleSpinBox;
    inCx->setRange(0, 1000);
    inCx->setValue(C.x);
    inCx->setSuffix(" " + tr("mm"));
    inCx->setSingleStep(1);
    inCx->setDecimals(3);

    inCy = new QDoubleSpinBox;
    inCy->setRange(0, 1000);
    inCy->setValue(C.y);
    inCy->setSuffix(" " + tr("mm"));
    inCy->setSingleStep(1);
    inCy->setDecimals(3);

    labelCx->setBuddy(inCx);
    labelCy->setBuddy(inCy);

    QHBoxLayout *hboxCenter = new QHBoxLayout;
    hboxCenter->addWidget(labelCx);
    hboxCenter->addWidget(inCx);
    hboxCenter->addWidget(labelCy);
    hboxCenter->addWidget(inCy);

    hboxCenter->setStretch(0,0);
    hboxCenter->setStretch(1,1);
    hboxCenter->setStretch(2,0);
    hboxCenter->setStretch(3,1);

    groupCenter->setLayout(hboxCenter);

    // Dialog
    QHBoxLayout* hbox = new QHBoxLayout;
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    hbox->addWidget(buttonBox);

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addWidget(group);
    vbox->addWidget(groupCenter);
    vbox->addLayout(hbox);

    this->setLayout(vbox);

    inPct->setFocus();

    this->layout()->setSizeConstraint( QLayout::SetFixedSize );
    this->setWindowTitle(tr("Resize"));

    connect(radioPct, &QRadioButton::toggled, this, [&]() {
        inPct->setEnabled(true);
        inWidth->setEnabled(false);
        inHeight->setEnabled(false);
        inBaseSize->setEnabled(false);
        inNewSize->setEnabled(false);
    });
    connect(radioRect, &QRadioButton::toggled, this, [&]() {
        inPct->setEnabled(false);
        inWidth->setEnabled(true);
        inHeight->setEnabled(true);
        inBaseSize->setEnabled(false);
        inNewSize->setEnabled(false);
    });
    connect(radioRatio, &QRadioButton::toggled, this, [&]() {
        inPct->setEnabled(false);
        inWidth->setEnabled(false);
        inHeight->setEnabled(false);
        inBaseSize->setEnabled(true);
        inNewSize->setEnabled(true);
    });

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

bool ResizeDialog::isPct() const { return radioPct->isChecked(); }
bool ResizeDialog::isRect() const { return radioRect->isChecked(); }
bool ResizeDialog::isRatio() const { return radioRatio->isChecked(); }

double ResizeDialog::pct() const { return inPct->value(); }
double ResizeDialog::rectWidth() const { return inWidth->value(); }
double ResizeDialog::rectHeight() const { return inHeight->value(); }
double ResizeDialog::ratioBaseSize() const { return inBaseSize->value(); }
double ResizeDialog::ratioNewSize() const { return inNewSize->value(); }

fpoint_valid_t ResizeDialog::center() const {
    return validCenter() ? fpoint_valid_t(inCx->value(), inCy->value(), true) : fpoint_valid_t(0,0,false);
}

bool ResizeDialog::validCenter() const { return groupCenter->isChecked(); }

ResizeDialog::~ResizeDialog() {}
