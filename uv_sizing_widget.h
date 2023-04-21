#ifndef UV_SIZING_WIDGET_H
#define UV_SIZING_WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QDoubleSpinBox>
//#include <QSpinBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QPushButton>
#include <vector>

#include "my_types.h"
#include "program_param.h"

struct uv_sizing_t {
    double uv_max, L, H, T, D_wire;
};

struct uv_dia_t {
    double top, bot;
};

struct uv_result_t {
    double L, H;
};

class UVSizingWidget : public QWidget {
    Q_OBJECT

    ProgramParam& par;

    QLabel *labelMeasuring, *labelRecalc, *labelResult, *labelWireD, *labelUMax, *labelL, *labelH, *labelT, *labelRollerD, *labelRollerPlane, *labelEntryLine, *labelDTop, *labelDBot, *labelResL, *labelResH;
    QDoubleSpinBox *fnumWireD, *fnumUMax, *fnumL, *fnumH, *fnumT, *fnumRollerD, *fnumLine, *fnumDTop, *fnumDBot, *fnumResL, *fnumResH;
    QRadioButton *radioXp, *radioXn, *radioYp, *radioYn, *radioXZ, *radioYZ;
    QGroupBox *groupMeasuring, *groupRecalc, *groupResult, *groupAxis, *groupRollerPlane;
    QPushButton *btnGen, *btnRecalc;

    std::vector<QWidget*> widgets;

    static void setFontPointSize(QWidget* w, int pointSize);

private slots:
    void onGenerate();
    void onRecalc();

    uv_sizing_t getSizingPar() const;

    static uv_dia_t calcDia(const uv_sizing_t& p);
    static double calc_dx(double H, double L, double u_max);
    static uv_result_t recalc(const uv_sizing_t& p, uv_dia_t d);

signals:
    void clicked();

public:
    explicit UVSizingWidget(ProgramParam& par, QWidget *parent = nullptr);

    void setFontPointSize(int pointSize);

signals:

};

#endif // UV_SIZING_WIDGET_H
