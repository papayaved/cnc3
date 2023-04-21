#ifndef COMBWIDGET_H
#define COMBWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QPushButton>
#include <vector>

#include "my_types.h"
#include "program_param.h"

class SlotsWidget : public QWidget {
    Q_OBJECT

    ProgramParam& par;

    QLabel *labelTitle, *labelWireD, *labelDepth, *labelStep, *labelSlotsNum, *labelSpeed, *labelIdleSpeed, *labelRollVel, *labelIdleRollVel;
    QDoubleSpinBox *fnumWireD, *fnumDepth, *fnumStep, *fnumSpeed, *fnumIdleSpeed;
    QSpinBox *numSlotsNum, *numRollVel, *numIdleRollVel, *numPause;
    QCheckBox* checkPause;
    QGroupBox *groupAxis;
    QRadioButton *radioX, *radioY;
    QPushButton *btnUndo, *btnReset, *btnGen;

    std::vector<QWidget*> widgets;

    void init();
    static void setFontPointSize(QWidget* w, int pointSize);

private slots:
    void onUndo();
    void onReset();
    void onGenerate();

signals:
    void clicked();

public:
    explicit SlotsWidget(ProgramParam& par, QWidget *parent = nullptr);    

    comb_t comb() const;

    void setFontPointSize(int pointSize);

signals:

};

#endif // COMBWIDGET_H
