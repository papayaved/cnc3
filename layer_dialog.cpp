#include "layer_dialog.h"
#include "main.h"

using namespace std;

QStringList LayerDialog::getStringList(const std::set<std::string> &layers) {
    QStringList list;

    for (const auto& s: layers) {
        list.push_back(QString( s.c_str() ));
    }

    return list;
}

LayerDialog::LayerDialog(const set<string>& layers, QWidget* parent) : QDialog(parent) {
    QStringList list = getStringList(layers);

#ifndef STONE
    labelTitle = new QLabel(tr("Select layers") + ":");
    labelLayer = new QLabel(tr("Layer"));
#else
    labelTitle = new QLabel(tr("Select layer") + ":");
#endif


    labelXY = new QLabel(tr("XY plane"));
    comboXY = new QComboBox(this);
    comboXY->insertItems(0, list);

#ifndef STONE
    checkUV = new QCheckBox(this);

    labelUV = new QLabel(tr("UV plane"));

    comboUV = new QComboBox(this);
    comboUV->insertItems(0, list);

    if (list.length() > 1) {
        checkUV->setCheckState(Qt::CheckState::Checked);
        comboUV->setCurrentIndex(1);
    }
    else {
        checkUV->setCheckState(Qt::CheckState::Unchecked);
        comboUV->setCurrentIndex(0);
        labelUV->setEnabled(false);
        comboUV->setEnabled(false);
    }
#endif

    m_widgets = {labelTitle, labelXY, comboXY,
#ifndef STONE
                 labelLayer, checkUV, labelUV, comboUV
#endif
                };

    QGridLayout* grid = new QGridLayout;
    grid->addWidget(labelTitle, 0, 1, 1, 3, Qt::AlignCenter);

#ifndef STONE
    grid->addWidget(labelLayer, 1, 3, Qt::AlignCenter);
#endif

    grid->addWidget(labelXY, 2, 2);
    grid->addWidget(comboXY, 2, 3);

#ifndef STONE
    grid->addWidget(checkUV, 3, 1, Qt::AlignRight);
    grid->addWidget(labelUV, 3, 2);
    grid->addWidget(comboUV, 3, 3);
#endif

    grid->addWidget(new QFrame(), 4, 0, 1, 5);

    grid->setRowMinimumHeight(4, 14);
    grid->setColumnMinimumWidth(0, 50);
    grid->setColumnMinimumWidth(4, 50);

    // Dialog
    QHBoxLayout* hbox = new QHBoxLayout;
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    hbox->addWidget(buttonBox);

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addLayout(grid);
    vbox->addLayout(hbox);

    this->setLayout(vbox);
    this->setWindowTitle(tr("Layer selector"));

    this->layout()->setSizeConstraint( QLayout::SetFixedSize );

#ifndef STONE
    connect(checkUV, &QCheckBox::clicked, this, [&](bool checked) {
        labelUV->setEnabled(checked);
        comboUV->setEnabled(checked);
    });
#endif

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

LayerDialog::~LayerDialog() {}

bool LayerDialog::isXY() const {
    return comboXY->currentText().length() != 0;
}

bool LayerDialog::isUV() const {
#ifndef STONE
    if (checkUV->isChecked())
        return comboUV->currentText().length() != 0;
#endif

    return false;
}

string LayerDialog::xyLayerName() const {
    return comboXY->currentText().toStdString();
}

string LayerDialog::uvLayerName() const {
#ifndef STONE
    if (checkUV->isChecked())
        return comboUV->currentText().toStdString();
#endif

    return string();
}

void LayerDialog::setFontPointSize(QWidget *w, int pointSize) {
    if (w) {
        QFont font = w->font();
        font.setPointSize(pointSize);
        w->setFont(font);
    }
}

void LayerDialog::setFontPointSize(int pointSize) {
    for (QWidget* w: m_widgets)
        setFontPointSize(w, pointSize);
}
