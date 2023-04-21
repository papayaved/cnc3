#include "new_cutline_dialog.h"

void NewCutlineDialog::onAbsToggled() {
    inX0->setValue(m_x0);
    inX1->setValue(m_x1);
    inY0->setValue(m_y0);
    inY1->setValue(m_y1);
}

void NewCutlineDialog::onRelToggled() {
    (m_flags & 1) ? inX0->setValue(m_x0) : inX0->setValue(0);
    (m_flags & 2) ? inX1->setValue(m_x1) : inX1->setValue(0);
    (m_flags & 4) ? inY0->setValue(m_y0) : inY0->setValue(0);
    (m_flags & 8) ? inY1->setValue(m_y1) : inY1->setValue(0);
}

//void NewCutlineDialog::accept() {
//    OK = true;

//}

NewCutlineDialog::NewCutlineDialog(QWidget *parent) : QDialog(parent), OK(false) {
    labelX0 = new QLabel("X0");
    labelX1 = new QLabel("X1");
    labelY0 = new QLabel("Y0");
    labelY1 = new QLabel("Y1");

    inX0 = new QDoubleSpinBox;
    inX1 = new QDoubleSpinBox;
    inY0 = new QDoubleSpinBox;
    inY1 = new QDoubleSpinBox;

    inX0->setRange(MIN_VALUE, MAX_VALUE);
    inX1->setRange(MIN_VALUE, MAX_VALUE);
    inY0->setRange(MIN_VALUE, MAX_VALUE);
    inY1->setRange(MIN_VALUE, MAX_VALUE);

    labelX0->setBuddy(inX0);
    labelX1->setBuddy(inX1);
    labelY0->setBuddy(inY0);
    labelY1->setBuddy(inY1);

    radioAbs = new QRadioButton(tr("Abs"));
    radioRel = new QRadioButton(tr("Rel"));

    radioRel->setChecked(true);

    group = new QGroupBox;
    QHBoxLayout* hbox = new QHBoxLayout;

    hbox->addWidget(radioAbs);
    hbox->addWidget(radioRel);

    group->setLayout(hbox);

    QHBoxLayout* hboxButton = new QHBoxLayout;
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    hboxButton->addWidget(buttonBox);

    grid = new QGridLayout;

    grid->addWidget(group, 0, 0, 1, 4);

    grid->addWidget(labelX0, 1, 0, Qt::AlignRight);
    grid->addWidget(inX0, 1, 1);
    grid->addWidget(labelX1, 1, 2, Qt::AlignRight);
    grid->addWidget(inX1, 1, 3);

    grid->addWidget(labelY0, 2, 0, Qt::AlignRight);
    grid->addWidget(inY0, 2, 1);
    grid->addWidget(labelY1, 2, 2, Qt::AlignRight);
    grid->addWidget(inY1, 2, 3);

    grid->addLayout(hboxButton, 3, 0, 1, 4);

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

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void NewCutlineDialog::set(double x0, double x1, double y0, double y1) {
    m_x0 = x0;
    m_x1 = x1;
    m_y0 = y0;
    m_y1 = y1;

    inX0->setValue(m_x0);
    inX1->setValue(m_x1);
    inY0->setValue(m_y0);
    inY1->setValue(m_y1);
}

void NewCutlineDialog::enable(uint8_t flags) {
    m_flags = flags;

    inX0->setEnabled(m_flags & 1);
    inX1->setEnabled(m_flags & 2);
    inY0->setEnabled(m_flags & 4);
    inY1->setEnabled(m_flags & 8);
}

void NewCutlineDialog::get(double &x0, double &x1, double &y0, double &y1) {
    x0 = inX0->value();
    x1 = inX1->value();
    y0 = inY0->value();
    y1 = inY1->value();
}

bool NewCutlineDialog::isAbs() { return radioAbs->isChecked(); }
bool NewCutlineDialog::isRel() { return radioRel->isChecked(); }

//bool NewCutlineDialog::result() { return OK; }

NewCutlineDialog::~NewCutlineDialog() {}
