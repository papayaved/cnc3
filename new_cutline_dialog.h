#ifndef NEW_CUTLINE_DIALOG_H
#define NEW_CUTLINE_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QGridLayout>
#include <QDialogButtonBox>
#include "fpoint_t.h"

class NewCutlineDialog : public QDialog {
    Q_OBJECT

    const double MAX_VALUE = 1e6;
    const double MIN_VALUE = -MAX_VALUE;

    fpoint_t m_in_pt {0,0}, m_out_pt {0,0};

    QLabel *labelX0 {nullptr}, *labelY0 {nullptr}, *labelX1 {nullptr}, *labelY1 {nullptr};
    QDoubleSpinBox *inX0 {nullptr}, *inY0 {nullptr}, *inX1 {nullptr}, *inY1 {nullptr};

    QRadioButton *radioAbs {nullptr}, *radioRel {nullptr};
    QGroupBox *groupAbsRel {nullptr};

    QRadioButton *radioIn {nullptr}, *radioOut {nullptr};
    QGroupBox *groupInOut {nullptr};

    QGridLayout *grid {nullptr};

private slots:
    void onAbsToggled();
    void onRelToggled();
    void onInToggled();
    void onOutToggled();

public:
    QDialogButtonBox* buttonBox;

    explicit NewCutlineDialog(const fpoint_t& in_pt, const fpoint_t& out_pt, bool before, bool signle, QWidget *parent = nullptr);
    ~NewCutlineDialog();

    void get(double& x0, double& x1, double& y0, double& y1) const;
    bool isAbs() const;
    bool isRel() const;
    bool isIn() const;
    bool isOut() const;
};

#endif // NEW_CUTLINE_DIALOG_H
