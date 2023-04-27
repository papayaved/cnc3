#include "new_cutline_dialog.h"

void NewCutlineDialog::onAbsToggled() {
    if (radioIn->isChecked()) {
        inX0->setValue(0);
        inY0->setValue(0);
        inX1->setValue(m_in_pt.x);
        inY1->setValue(m_in_pt.y);
    }
    else {
        inX0->setValue(m_out_pt.x);
        inY0->setValue(m_out_pt.y);
        inX1->setValue(0);
        inY1->setValue(0);
    }
}

void NewCutlineDialog::onRelToggled() {
    inX0->setValue(0);
    inY0->setValue(0);
    inX1->setValue(0);
    inY1->setValue(0);
}

void NewCutlineDialog::onInToggled() {
    inX0->setEnabled(true);
    inY0->setEnabled(true);
    inX1->setEnabled(false);
    inY1->setEnabled(false);

    if (radioAbs->isChecked())
        onAbsToggled();
    else
        onRelToggled();
}

void NewCutlineDialog::onOutToggled() {
    inX0->setEnabled(false);
    inY0->setEnabled(false);
    inX1->setEnabled(true);
    inY1->setEnabled(true);

    if (radioAbs->isChecked())
        onAbsToggled();
    else
        onRelToggled();
}

NewCutlineDialog::NewCutlineDialog(const fpoint_t& in_pt, const fpoint_t& out_pt, bool before, bool single, QWidget *parent) : QDialog(parent) {
    m_in_pt = in_pt;
    m_out_pt = out_pt;

    before |= single;

    labelX0 = new QLabel("X0");
    labelY0 = new QLabel("Y0");
    labelX1 = new QLabel("X1");
    labelY1 = new QLabel("Y1");

    inX0 = new QDoubleSpinBox;
    inY0 = new QDoubleSpinBox;
    inX1 = new QDoubleSpinBox;
    inY1 = new QDoubleSpinBox;

    inX0->setRange(MIN_VALUE, MAX_VALUE);
    inY0->setRange(MIN_VALUE, MAX_VALUE);
    inX1->setRange(MIN_VALUE, MAX_VALUE);
    inY1->setRange(MIN_VALUE, MAX_VALUE);

    labelX0->setBuddy(inX0);
    labelY0->setBuddy(inY0);
    labelX1->setBuddy(inX1);
    labelY1->setBuddy(inY1);

    //
    radioAbs = new QRadioButton(tr("Abs"));
    radioRel = new QRadioButton(tr("Rel"));

    groupAbsRel = new QGroupBox;
    QHBoxLayout* hboxAbsRel = new QHBoxLayout;

    hboxAbsRel->addWidget(radioAbs);
    hboxAbsRel->addWidget(radioRel);

    groupAbsRel->setLayout(hboxAbsRel);

    radioRel->setChecked(true);

    //
    radioIn = new QRadioButton(tr("In"));
    radioOut = new QRadioButton(tr("Out"));

    groupInOut = new QGroupBox;
    QHBoxLayout* hboxInOut = new QHBoxLayout;

    hboxInOut->addWidget(radioIn);
    hboxInOut->addWidget(radioOut);

    groupInOut->setLayout(hboxInOut);

    if (single)
        groupInOut->setEnabled(false);

    if (before) {
        radioIn->setChecked(true);
        onInToggled();
    }
    else {
        radioOut->setChecked(true);
        onOutToggled();
    }

    //
    QHBoxLayout* hboxButton = new QHBoxLayout;
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    hboxButton->addWidget(buttonBox);

    grid = new QGridLayout;

    grid->addWidget(groupInOut, 0, 0, 1, 4);
    grid->addWidget(groupAbsRel, 1, 0, 1, 4);

    grid->addWidget(labelX0, 2, 0, Qt::AlignRight);
    grid->addWidget(inX0, 2, 1);
    grid->addWidget(labelX1, 2, 2, Qt::AlignRight);
    grid->addWidget(inX1, 2, 3);

    grid->addWidget(labelY0, 3, 0, Qt::AlignRight);
    grid->addWidget(inY0, 3, 1);
    grid->addWidget(labelY1, 3, 2, Qt::AlignRight);
    grid->addWidget(inY1, 3, 3);

    grid->addLayout(hboxButton, 4, 0, 1, 4);

    grid->setColumnStretch(0, 0);
    grid->setColumnStretch(1, 1);
    grid->setColumnStretch(2, 0);
    grid->setColumnStretch(3, 1);

    this->setLayout(grid);

    inX0->setFocus();

//    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->layout()->setSizeConstraint( QLayout::SetFixedSize );
    this->setWindowTitle(tr("New cutline"));

    connect(radioAbs, &QRadioButton::toggled, this, &NewCutlineDialog::onAbsToggled);
    connect(radioRel, &QRadioButton::toggled, this, &NewCutlineDialog::onRelToggled);

    connect(radioIn, &QRadioButton::toggled, this, &NewCutlineDialog::onInToggled);
    connect(radioOut, &QRadioButton::toggled, this, &NewCutlineDialog::onOutToggled);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

NewCutlineDialog::~NewCutlineDialog() {}

void NewCutlineDialog::get(double &x0, double &x1, double &y0, double &y1) const {
    x0 = inX0->value();
    x1 = inX1->value();
    y0 = inY0->value();
    y1 = inY1->value();
}

bool NewCutlineDialog::isAbs() const { return radioAbs->isChecked(); }
bool NewCutlineDialog::isRel() const { return radioRel->isChecked(); }

bool NewCutlineDialog::isIn() const { return radioIn->isChecked(); }
bool NewCutlineDialog::isOut() const { return radioOut->isChecked(); }
