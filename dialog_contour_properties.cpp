#include "dialog_contour_properties.h"

using namespace std;

ContourPropDialog::ContourPropDialog(const ContourPair* const pair, size_t ctr_num, QWidget *parent) : QDialog(parent) {
    labelCtrName = new QLabel(tr("Contour") + ":");
    labelCtrValue = new QLabel( QString::number(ctr_num + 1) );

    labelSegsName = new QLabel(tr("Segments") + ":");
    labelSegsValue = new QLabel( pair ? QString::number(pair->count()) : tr("Invalid") );

    labelSortedName = new QLabel(tr("Sorted") + ":");
    labelSortedValue = new QLabel( pair ? pair->isSorted() ? tr("Yes") : tr("No") : tr("Invalid") );

    labelClosedName = new QLabel(tr("Closed") + ":");
    labelClosedValue = new QLabel( pair ? pair->isSorted() && pair->isLoop() ? tr("Yes") : tr("No") : tr("Invalid") );

    QGridLayout* gridDesc = new QGridLayout;

    gridDesc->addWidget(labelCtrName, 0, 0, Qt::AlignLeft);
    gridDesc->addWidget(labelCtrValue, 0, 1, Qt::AlignLeft);
    gridDesc->addWidget(labelSegsName, 0, 2, Qt::AlignLeft);
    gridDesc->addWidget(labelSegsValue, 0, 3, Qt::AlignLeft);

    gridDesc->addWidget(labelSortedName, 1, 0, Qt::AlignLeft);
    gridDesc->addWidget(labelSortedValue, 1, 1, Qt::AlignLeft);
    gridDesc->addWidget(labelClosedName, 1, 2, Qt::AlignLeft);
    gridDesc->addWidget(labelClosedValue, 1, 3, Qt::AlignLeft);

    radioMain = new QRadioButton(tr("Main"));
    radioCutline = new QRadioButton(tr("Cutline"));

    QVBoxLayout* vboxGroup = new QVBoxLayout;
    vboxGroup->addWidget(radioMain);
    vboxGroup->addWidget(radioCutline);

    groupContourType = new QGroupBox(tr("Contour"));
    groupContourType->setLayout(vboxGroup);

    if (pair) {
        if (pair->type() == CONTOUR_TYPE::CUTLINE_CONTOUR)
            radioCutline->setChecked(true);
        else if (pair->type() == CONTOUR_TYPE::MAIN_CONTOUR)
            radioMain->setChecked(true);
    }

    m_widgets = {radioMain, radioCutline, groupContourType};

    QGridLayout* gridMain = new QGridLayout;
    gridMain->addLayout(gridDesc, 0, 0, 1, 2);
    gridMain->addWidget(groupContourType, 1, 0, 1, 2);

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
    this->setWindowTitle(tr("Contour properties"));

    this->layout()->setSizeConstraint( QLayout::SetFixedSize );

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);    
}

ContourPropDialog::~ContourPropDialog() {}

bool ContourPropDialog::isCutline() const {
    return radioCutline->isChecked();
}

void ContourPropDialog::setFontPointSize(QWidget *w, int pointSize) {
    if (w) {
        QFont font = w->font();
        font.setPointSize(pointSize);
        w->setFont(font);
    }
}

void ContourPropDialog::setFontPointSize(int pointSize) {
    for (QWidget* w: m_widgets)
        setFontPointSize(w, pointSize);
}
