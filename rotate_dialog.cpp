#include "rotate_dialog.h"


RotateDialog::RotateDialog(QWidget *parent) : QDialog(parent) {
    labelAngle = new QLabel(tr("Angle") + ": ");

    inAngle = new QDoubleSpinBox;
    inAngle->setRange(-180, 180);
    inAngle->setSuffix("°");
    inAngle->setValue(0);
    inAngle->setDecimals(3);
    inAngle->setSingleStep(1);
    labelAngle->setBuddy(inAngle);

    radio0 = new QRadioButton("0°");
    radio90CCW = new QRadioButton("90°CCW");
    radio90CW = new QRadioButton("90°CW");
    radio180 = new QRadioButton("180°");
    radio0->setChecked(true);

    QHBoxLayout* hboxRadio = new QHBoxLayout;
    hboxRadio->addWidget(radio0);
    hboxRadio->addWidget(radio90CCW);
    hboxRadio->addWidget(radio90CW);
    hboxRadio->addWidget(radio180);

    group = new QGroupBox;
//    group->setLayout(hboxRadio);

    QHBoxLayout* hboxInput = new QHBoxLayout;

    hboxInput->addWidget(labelAngle, Qt::AlignRight);
    hboxInput->addWidget(inAngle, Qt::AlignLeft);

    QHBoxLayout* hboxButton = new QHBoxLayout;
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    hboxButton->addWidget(buttonBox);

    grid = new QGridLayout;

    grid->addLayout(hboxRadio, 0, 0);
    grid->addLayout(hboxInput, 1, 0);
    grid->addLayout(hboxButton, 2, 0);
    this->setLayout(grid);

    inAngle->setFocus();

//    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->layout()->setSizeConstraint( QLayout::SetFixedSize );
    this->setWindowTitle(tr("Rotate"));

    connect(radio0, &QRadioButton::toggled, this, [&](bool checked) {
        if (checked) {
            inAngle->blockSignals(true);
            inAngle->setValue(0);
            inAngle->blockSignals(false);

    //        radio0->setChecked(false);
            radio90CCW->setChecked(false);
            radio90CW->setChecked(false);
            radio180->setChecked(false);
        }
    });
    connect(radio90CCW, &QRadioButton::toggled, this, [&](bool checked) {
        if (checked) {
            inAngle->blockSignals(true);
            inAngle->setValue(90);
            inAngle->blockSignals(false);

            radio0->setChecked(false);
    //        radio90CCW->setChecked(false);
            radio90CW->setChecked(false);
            radio180->setChecked(false);
        }
    });
    connect(radio90CW, &QRadioButton::toggled, this, [&](bool checked) {
        if (checked) {
            inAngle->blockSignals(true);
            inAngle->setValue(-90);
            inAngle->blockSignals(false);

            radio0->setChecked(false);
            radio90CCW->setChecked(false);
    //        radio90CW->setChecked(false);
            radio180->setChecked(false);
        }
    });
    connect(radio180, &QRadioButton::toggled, this, [&](bool checked) {
        if (checked) {
            inAngle->blockSignals(true);
            inAngle->setValue(180);
            inAngle->blockSignals(false);

            radio0->setChecked(false);
            radio90CCW->setChecked(false);
            radio90CW->setChecked(false);
    //        radio180->setChecked(false);
        }
    });

    connect(inAngle, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double /*x*/) {
//        group->setChecked(false);
        radio0->setChecked(false);
        radio90CCW->setChecked(false);
        radio90CW->setChecked(false);
        radio180->setChecked(false);
    });

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

double RotateDialog::value() const {
    return inAngle->value();
}

RotateDialog::~RotateDialog() {}
