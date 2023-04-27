#include "dialog_move_seg.h"
#include "dxf_arc.h"

using namespace std;

MoveSegDialog::MoveSegDialog(const QStringList& list, size_t ctr_num, size_t sel_segs_num, QWidget *parent) : QDialog(parent) {
    labelCtrName = new QLabel(tr("Current contour") + ":");
    labelCtrValue = new QLabel( (int)ctr_num < list.count() ? list.at(ctr_num) : "Null" );

    labelSegName = new QLabel(tr("Selected segments") + ":");
    labelSegValue = new QLabel( QString::number(sel_segs_num) );

    QGridLayout* gridDesc = new QGridLayout;

    gridDesc->addWidget(labelCtrName, 0, 0, Qt::AlignLeft);
    gridDesc->addWidget(labelCtrValue, 0, 1, Qt::AlignLeft);
    gridDesc->addWidget(labelSegName, 1, 0, Qt::AlignLeft);
    gridDesc->addWidget(labelSegValue, 1, 1, Qt::AlignLeft);

    labelContour = new QLabel(tr("Contour"));

    comboContour = new QComboBox;
    comboContour->insertItems(0, list);
    comboContour->setCurrentIndex(ctr_num);

    m_widgets = {labelContour, comboContour};

    QGridLayout* gridMain = new QGridLayout;
    gridMain->addLayout(gridDesc, 0, 0, 1, 2);
    gridMain->addWidget(new QFrame, 1, 0, 1, 2);
    gridMain->addWidget(labelContour, 2, 0, Qt::AlignRight);
    gridMain->addWidget(comboContour, 2, 1, Qt::AlignLeft);
    gridMain->setRowMinimumHeight(1, 10);

    QGridLayout* grid = new QGridLayout;
    grid->addLayout(gridMain, 0, 1);
    grid->addWidget(new QFrame, 1, 0, 1, 3);

    grid->setRowMinimumHeight(1, 10);
    grid->setColumnMinimumWidth(0, 40);
    grid->setColumnMinimumWidth(2, 40);

    // Dialog
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addLayout(grid);
    vbox->addWidget(buttonBox);

    this->setLayout(vbox);
    this->setWindowTitle(tr("Move to another contour"));

    this->layout()->setSizeConstraint( QLayout::SetFixedSize );

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

MoveSegDialog::~MoveSegDialog() {}

int MoveSegDialog::contourNumber() const { return comboContour->currentIndex(); }

void MoveSegDialog::setFontPointSize(QWidget *w, int pointSize) {
    if (w) {
        QFont font = w->font();
        font.setPointSize(pointSize);
        w->setFont(font);
    }
}

void MoveSegDialog::setFontPointSize(int pointSize) {
    for (QWidget* w: m_widgets)
        setFontPointSize(w, pointSize);
}
