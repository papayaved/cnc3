#ifndef ROTATEDIALOG_H
#define ROTATEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QGridLayout>
#include <QDialogButtonBox>

class RotateDialog : public QDialog {
    Q_OBJECT

    QLabel *labelAngle;
    QDoubleSpinBox *inAngle;

    QRadioButton *radio0, *radio90CCW, *radio90CW, *radio180;
    QGroupBox *group;

    QGridLayout *grid;

public:
    QDialogButtonBox* buttonBox;

    explicit RotateDialog(QWidget *parent = nullptr);
    double value() const;
    ~RotateDialog();
};

#endif // ROTATEDIALOG_H
