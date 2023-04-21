#ifndef SEGDIALOG_H
#define SEGDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <vector>
//#include "fpoint_t.h"
#include "dxf_entity.h"

class SegPropertiesDialog : public QDialog {
    Q_OBJECT

    QLabel *labelCtrName {nullptr}, *labelCtrValue {nullptr}, *labelSegName {nullptr}, *labelSegValue {nullptr}, *labelLayerName {nullptr}, *labelLayerValue {nullptr};
    QLabel *labelTypeName {nullptr}, *labelTypeValue {nullptr}, *labelAB {nullptr};
    QLabel *labelLenText {nullptr}, *labelLenNum {nullptr};
    QLabel *labelRText {nullptr}, *labelRNum {nullptr}, *labelCText {nullptr}, *labelCNum {nullptr}, *labelCCWText {nullptr}, *labelCCWValue {nullptr};

    QLabel *labelContour {nullptr};
    QComboBox *comboContour {nullptr};
    QCheckBox *checkNewContour {nullptr};
    QGroupBox *groupNewContour {nullptr};
    QRadioButton *radioBefore {nullptr}, *radioAfter {nullptr};

    std::vector<QWidget*> m_widgets;

    static void setFontPointSize(QWidget *w, int pointSize);

public:
    QDialogButtonBox* buttonBox;

    explicit SegPropertiesDialog(
        const QStringList& contours_list,
        const DxfEntity* const ent,
        size_t ctr_num, size_t row_num, size_t col_num,
        bool before, bool after,
        QWidget *parent = nullptr
    );

    ~SegPropertiesDialog();

    bool isNewContour() const;
    bool isNewBefore() const;
    bool isNewAfter() const;
    int contourNumber() const;

    void setFontPointSize(int pointSize);
};

#endif // SEGDIALOG_H
