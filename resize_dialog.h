#ifndef RESIZEDIALOG_H
#define RESIZEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QGridLayout>
#include <QDialogButtonBox>
#include "fpoint_t.h"

class ResizeDialog : public QDialog {
    Q_OBJECT

    QLabel *labelWidth, *labelHeight, *labelBaseSize, *labelNewSize, *labelCx, *labelCy;
    QDoubleSpinBox *inPct, *inWidth, *inHeight, *inBaseSize, *inNewSize, *inCx, *inCy;

    QRadioButton *radioPct, *radioRect, *radioRatio;
    QGroupBox *group, *groupCenter;

public:
    QDialogButtonBox* buttonBox;

    explicit ResizeDialog(double width, double height, const fpoint_valid_t& center, QWidget *parent = nullptr);

    bool isPct() const;
    bool isRect() const;
    bool isRatio() const;

    double pct() const;
    double rectWidth() const;
    double rectHeight() const;
    double ratioBaseSize() const;
    double ratioNewSize() const;

    fpoint_valid_t center() const;
    bool validCenter() const;

    ~ResizeDialog();
};

#endif // RESIZEDIALOG_H
