#include "dialog_segment_properties.h"
#include "dxf_arc.h"

using namespace std;

SegPropertiesDialog::SegPropertiesDialog(
    const QStringList& list,
    const DxfEntity* const ent,
    size_t ctr_num, size_t row_num, size_t col_num,
    bool before, bool after,
    QWidget *parent
)
    : QDialog(parent)
{
    labelCtrName = new QLabel(tr("Contour") + ":");
    labelCtrValue = new QLabel( QString::number(ctr_num + 1) );

    labelSegName = new QLabel(tr("Segment") + ":");
    labelSegValue = new QLabel( QString::number(row_num + 1) );

    labelLayerName = new QLabel(tr("Layer") + ":");
    labelLayerValue = new QLabel(col_num ? tr("UV plane") : tr("XY plane"));

    labelTypeName = new QLabel(tr("Type") + ":");
    labelTypeValue = new QLabel( ent ? QString::fromStdString( ent->typeString() ) : tr("Invalid") );

    labelAB = new QLabel( ent ? QString::fromStdString( ent->point_0().toString() + " - " + ent->point_1().toString()) : tr("Invalid") );

    bool isArc = ent && ent->type() == ENTITY_TYPE::ARC;

    if (isArc) {
        const DxfArc* const arc = reinterpret_cast<const DxfArc*>(ent);

        labelRText = new QLabel(tr("Radius") + ":");

        QString s;
        s += QString::number(arc->radius(), 'f', 3) + " " + tr("mm");
        labelRNum = new QLabel(s);

        labelCText = new QLabel(tr("Center") + ":");
        labelCNum = new QLabel( QString::fromStdString( arc->center().toString() ) );

        labelCCWText = new QLabel(tr("CCW") + ":");
        labelCCWValue = new QLabel( arc->CCW() ? tr("Yes") : tr("No") );
    }
    else {
        labelRText = new QLabel;
        labelRNum = new QLabel;
        labelCText = new QLabel;
        labelCNum = new QLabel;
    }

    labelLenText = new QLabel(tr("Length") + ":");
    QString s;

    s += ( ent ? QString::number(ent->length(), 'f', 3) : tr("Invalid") ) + " " + tr("mm");

    labelLenNum = new QLabel(s);

    QGridLayout* gridDesc = new QGridLayout;

    gridDesc->addWidget(labelCtrName, 0, 0, Qt::AlignLeft);
    gridDesc->addWidget(labelCtrValue, 0, 1, Qt::AlignLeft);
    gridDesc->addWidget(labelSegName, 0, 2, Qt::AlignLeft);
    gridDesc->addWidget(labelSegValue, 0, 3, Qt::AlignLeft);

    gridDesc->addWidget(labelLayerName, 1, 0, Qt::AlignLeft);
    gridDesc->addWidget(labelLayerValue, 1, 1, Qt::AlignLeft);
    gridDesc->addWidget(labelTypeName, 1, 2, Qt::AlignLeft);
    gridDesc->addWidget(labelTypeValue, 1, 3, Qt::AlignLeft);

    gridDesc->addWidget(labelAB, 2, 0, 1, 4, Qt::AlignLeft);

    if (isArc) {
        gridDesc->addWidget(labelRText, 3, 0, Qt::AlignLeft);
        gridDesc->addWidget(labelRNum, 3, 1, Qt::AlignLeft);
        gridDesc->addWidget(labelCText, 3, 2, Qt::AlignLeft);
        gridDesc->addWidget(labelCNum, 3, 3, Qt::AlignLeft);
        gridDesc->addWidget(labelCCWText, 4, 0, Qt::AlignLeft);
        gridDesc->addWidget(labelCCWValue, 4, 1, Qt::AlignLeft);
    }

    gridDesc->addWidget(labelLenText, 5, 0, Qt::AlignLeft);
    gridDesc->addWidget(labelLenNum, 5, 1, Qt::AlignLeft);

    labelContour = new QLabel(tr("Contour"));

    comboContour = new QComboBox;
    comboContour->insertItems(0, list);

    checkNewContour = new QCheckBox(tr("New contour"));

    radioBefore = new QRadioButton(tr("Add before"));
    radioAfter = new QRadioButton(tr("Add after"));

    QVBoxLayout* vboxNewContour = new QVBoxLayout;
    vboxNewContour->addWidget(radioBefore);
    vboxNewContour->addWidget(radioAfter);

    groupNewContour = new QGroupBox;
    groupNewContour->setLayout(vboxNewContour);
    groupNewContour->setEnabled(false);

    m_widgets = {labelContour, comboContour, checkNewContour, radioBefore, radioAfter, groupNewContour};

    QGridLayout* gridMain = new QGridLayout;
    gridMain->addLayout(gridDesc, 0, 0, 1, 2);
    gridMain->addWidget(new QFrame, 1, 0, 1, 2);
    gridMain->addWidget(labelContour, 2, 0, Qt::AlignRight);
    gridMain->addWidget(comboContour, 2, 1, Qt::AlignLeft);
    gridMain->addWidget(checkNewContour, 3, 0, 1, 2, Qt::AlignLeft);
    gridMain->addWidget(groupNewContour, 4, 0, 1, 2, Qt::AlignLeft);
    gridMain->setRowMinimumHeight(1, 10);

    QGridLayout* grid = new QGridLayout;
    grid->addLayout(gridMain, 0, 1);
    grid->addWidget(new QFrame, 1, 0, 1, 3);

    grid->setRowMinimumHeight(1, 10);
    grid->setColumnMinimumWidth(0, 40);
    grid->setColumnMinimumWidth(2, 40);

    // Dialog
//    QHBoxLayout* hbox = new QHBoxLayout(this);
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
//    hbox->addWidget(buttonBox);

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addLayout(grid);
//    vbox->addLayout(hbox);
    vbox->addWidget(buttonBox);

    this->setLayout(vbox);
    this->setWindowTitle(tr("Segment properties"));

    this->layout()->setSizeConstraint( QLayout::SetFixedSize );

    connect(checkNewContour, &QCheckBox::clicked, this, [&](bool checked) {
        labelContour->setEnabled(!checked);
        comboContour->setEnabled(!checked);
        groupNewContour->setEnabled(checked);
    });

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    if (before || after) {
        checkNewContour->click();

        if (before)
            radioBefore->setChecked(true);
        else if (after)
            radioAfter->setChecked(true);
    }
}

SegPropertiesDialog::~SegPropertiesDialog() {}

bool SegPropertiesDialog::isNewContour() const { return checkNewContour->isChecked(); }

bool SegPropertiesDialog::isNewBefore() const { return checkNewContour->isChecked() && radioBefore->isChecked(); }

bool SegPropertiesDialog::isNewAfter() const { return checkNewContour->isChecked() && radioAfter->isChecked(); }

int SegPropertiesDialog::contourNumber() const { return comboContour->currentIndex(); }

void SegPropertiesDialog::setFontPointSize(QWidget *w, int pointSize) {
    if (w) {
        QFont font = w->font();
        font.setPointSize(pointSize);
        w->setFont(font);
    }
}

void SegPropertiesDialog::setFontPointSize(int pointSize) {
    for (QWidget* w: m_widgets)
        setFontPointSize(w, pointSize);
}
