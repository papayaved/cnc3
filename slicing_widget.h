#ifndef SLICINGWIDGET_H
#define SLICINGWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QPushButton>
#include <QCheckBox>
#include <vector>

#include "my_types.h"
#include "program_param.h"

class SlicingWidget : public QWidget {
    Q_OBJECT

    ProgramParam& par;

    QLabel *labelTitle, *labelWireD, *labelWidth, *labelSpacing, *labelStep, *labelStepNum, *labelSpeedSections, *labelSpeedAvg;
    QDoubleSpinBox *fnumWireD, *fnumWidth, *fnumSpacing, *fnumStep, *fnumSpeedAvg;
    QSpinBox *numSlicesNum, *numSpeedSections, *numPause;
    QGroupBox *groupProfile;
    QRadioButton *radioRect, *radioCircle;
    QCheckBox* checkPause;
    QGroupBox *groupAxis;
    QRadioButton *radioX, *radioY;
    QPushButton *btnUndo, *btnReset, *btnGen;

//    std::vector<QLabel*> labels;
//    std::vector<QDoubleSpinBox*> fNumbers;
//    std::vector<QSpinBox*> numbers;
//    std::vector<QRadioButton*> radios;

     std::vector<QWidget*> widgets;

    static void setFontPointSize(QWidget* w, int pointSize);

private slots:
    void onUndo();
    void onReset();
    void onGenerate();

signals:
    void clicked();

public:
    explicit SlicingWidget(ProgramParam& par, QWidget *parent = nullptr);

    void init();
    snake_t snake() const;

    void setFontPointSize(int pointSize);

signals:

};

#endif // SLICINGWIDGET_H
