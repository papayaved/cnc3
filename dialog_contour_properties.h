#ifndef DIALOG_CTR_H
#define DIALOG_CTR_H

#include <QDialog>
#include <QLabel>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <vector>
#include "fpoint_t.h"
#include "contour_pair.h"

class ContourPropDialog : public QDialog {
    Q_OBJECT

    QLabel *labelCtrName {nullptr}, *labelCtrValue {nullptr}, *labelSegsName {nullptr}, *labelSegsValue {nullptr},
        *labelSortedName {nullptr}, *labelSortedValue {nullptr}, *labelClosedName {nullptr}, *labelClosedValue {nullptr};

    QGroupBox *groupContourType;
    QRadioButton *radioMain, *radioCutline;

    std::vector<QWidget*> m_widgets;

    static void setFontPointSize(QWidget *w, int pointSize);

public:
    QDialogButtonBox* buttonBox;

    explicit ContourPropDialog(const ContourPair* const pair, size_t ctr_num, QWidget *parent = nullptr);
    ~ContourPropDialog();

    bool isCutline() const;

    void setFontPointSize(int pointSize);
};

#endif // DIALOG_CTR_H
