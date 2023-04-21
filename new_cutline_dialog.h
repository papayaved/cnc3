#ifndef NEW_CUTLINE_DIALOG_H
#define NEW_CUTLINE_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QGridLayout>
#include <QDialogButtonBox>

class NewCutlineDialog : public QDialog {
    Q_OBJECT

    const double MAX_VALUE = 1e6;
    const double MIN_VALUE = -MAX_VALUE;

    uint8_t m_flags;
    double m_x0, m_x1, m_y0, m_y1;

    QLabel *labelX0, *labelX1, *labelY0, *labelY1;
    QDoubleSpinBox *inX0, *inX1, *inY0, *inY1;

    QRadioButton *radioAbs, *radioRel;
    QGroupBox *group;

    QGridLayout *grid;

    bool OK;

private slots:
    void onAbsToggled();
    void onRelToggled();
//    void accept();

public:
    QDialogButtonBox* buttonBox;

    explicit NewCutlineDialog(QWidget *parent = nullptr);
    void set(double x0, double x1, double y0, double y1);
    void enable(uint8_t flags);
    void get(double& x0, double& x1, double& y0, double& y1);
    bool isAbs();
    bool isRel();
//    bool result();

    ~NewCutlineDialog();

};

#endif // NEW_CUTLINE_DIALOG_H
