#ifndef FORMPULT_H
#define FORMPULT_H

#include <QWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QGroupBox>

#include "program_param.h"
#include "pult_widget.h"
#include "start_stop_elapsed_timer.h"

class FormPult : public QWidget {
    Q_OBJECT

    const QString help_file = "pult.html";
    const int POLLING_TIME = 1000 / 5; // ms
    const unsigned ADC_POLLING_TIME = 1000; // ms

    ProgramParam& par;
    PultWidget* pultWidget;
    bool m_cncReaderEna {false}, m_adcEnable {false}, m_running_reg {false};

    QLabel *labelWidth, *labelRatio, *labelCurrent, *labelRoll;
    std::vector<QLabel*> labels;

    QSpinBox *numWidth, *numRatio, *numCurrent, *numRollVel;
    std::vector<QSpinBox*> nums;

    QGroupBox *groupWidth, *groupRatio, *groupCurrent, *groupRoll;

    QPushButton *btnHome, *btnRoll, *btnPump, *btnBreak, *btnHighVolt, *btnWidthDec, *btnWidthInc, *btnRatioInc, *btnRatioDec,\
        *btnCurrentDec, *btnCurrentInc, *btnRollVelDec, *btnRollVelInc, *btnHelp;

#ifndef STONE
    QPushButton *btnLowHighVolt = nullptr;
#endif

    std::vector<QPushButton*> buttons;

    QGridLayout* gridButtons;
    QVBoxLayout* mainLayout;

    bool cutStateAbortReq = false;
    WireSpeed::Mode wireSpeedMode;

    void addButtons();
    void createButtonLayout();

    void _init();

    void startCncReader();
    void stopCncReader();

    void startAdc();
    void stopAdc();

    void controlsEnable(bool ena, bool force = false);
    void initButtons();

    void blockSignals(bool value);

private slots:
    void on_btnHome_clicked();

    void on_btnMove_clicked(bool dir = false);
    void on_btnSet_clicked();

    void on_btnCancel_clicked();

    void readCutState();    
    void readAdc();

public:
//    explicit FormRun(QWidget *parent = nullptr);
    FormPult(ProgramParam& par, QWidget *parent = nullptr);
    ~FormPult();

    void setFontPointSize(int pointSize);
    void init();

signals:
    void homePageClicked();
    void helpPageClicked(const QString& file_name);
};

#endif // FORMPULT_H
