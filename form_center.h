#ifndef FORMCENTER_H
#define FORMCENTER_H

#include <QWidget>
#include <QPushButton>
#include <QGridLayout>

#include "program_param.h"
#include "center_widget.h"
//#include "start_stop_elapsed_timer.h"

class FormCenter : public QWidget {
    Q_OBJECT

    const QString help_file = "centering.html";
    const unsigned POLLING_TIME = 1000 / 5; // ms
    const unsigned REMAIN_TIMER_MAX = 2000 / POLLING_TIME - 1;
    const unsigned ADC_POLLING_TIME = 1000; // ms

    ProgramParam& par;
    CenterWidget* centerWidget;
    bool cncReaderEna {false}, adcEnable {false};

    QPushButton *btnHome, *btnXDec, *btnXInc, *btnYDec, *btnYInc, *btnCenter, *btn6, *btn7, *btn8,\
        *btn9, *btnWeakHV, *btnHold, *btnCancel, *btnHelp;

    std::vector<QPushButton*> buttons;

    QGridLayout* gridButtons;
    QVBoxLayout* mainLayout;

    WireSpeed::Mode wireSpeedMode;

    auxItems::Reporter report;

    CenterWidget::CenterMode centerMode;

    void addButtons();
    void initButtons();

    void startCncReader();
    void stopCncReader();

    void startAdc();
    void stopAdc();

private slots:
    void on_btnHome_clicked();
    void on_btnXYForward_clicked(int dir, AXIS axis);
    void on_btnCenter_clicked();
    void on_btnCancel_clicked();

    void readCutState();
    void readAdc();

public:
//    explicit FormRun(QWidget *parent = nullptr);
    FormCenter(ProgramParam& par, QWidget *parent = nullptr);
    ~FormCenter();

    void setFontPointSize(int pointSize);

    void init();

signals:
    void homePageClicked();
    void helpPageClicked(const QString& file_name);
};

#endif // FORMCENTER_H
