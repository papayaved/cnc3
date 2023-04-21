#ifndef FORMRUN_H
#define FORMRUN_H

#include <QWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QGroupBox>

#include "program_param.h"
#include "run_widget.h"
#include "start_stop_elapsed_timer.h"

class FormRun : public QWidget {
    Q_OBJECT

    const QString help_file = "run.html";
    const unsigned POLLING_TIME = 1000 / 5; // ms
    const unsigned REMAIN_TIMER_MAX = 2000 / POLLING_TIME - 1;
    const unsigned ADC_POLLING_TIME = 1000; // ms

    ProgramParam& par;
    RunWidget* runWidget;
    bool cncReaderEna {false}, adcEnable {false};
    std::list<std::string> m_gframes;

    QLabel *labelWidth, *labelRatio, *labelCurrent, *labelRoll;
    QSpinBox *numWidth, *numRatio, *numCurrent, *numRollVel;
    std::vector<QSpinBox*> numBoxes;
    QGroupBox *groupWidth, *groupRatio, *groupCurrent, *groupRoll;

    QPushButton *btnHome, *btnRoll, *btnPump, *btnBreak, *btnHighVolt, *btnWidthDec, *btnWidthInc, *btnRatioInc, *btnRatioDec,\
        *btnCurrentDec, *btnCurrentInc, *btnRollVelDec, *btnRollVelInc, *btnHelp;

#ifndef STONE
    QPushButton *btnLowHighVolt {nullptr};
#endif

    std::vector<QPushButton*> buttons;

    QGridLayout* gridButtons;
    QVBoxLayout* mainLayout;

    int currentCursorPosition;

    bool cutStateAbortReq = false;
    WireSpeed m_speed;

    StartStopElapsedTimer* timer;
    unsigned remain_tmr;

    auxItems::Reporter m_report;

    QString m_info;

    void addButtons();
    void createSpinBoxes();
    void blockSignals(bool value);

    void _init();
    bool loadGcode();

    static QString addLineNum(QString txt);
    void updateButtons();
    GCode compile(const QString& txt);
    void setCursor(int row);
    void setCursorToBegin();
    void setCursorToEnd();

    void startTimer();
    void stopTimer();

    double doneDistance(int frame_num, const fpoint_t& bot_pos, const fpoint_t& top_pos);

    void startCncReader();
    void stopCncReader();

    void startAdc();
    void stopAdc();

private slots:
    void on_btnHome_clicked();
    void on_btnStart_clicked();
    void on_btnReverse_clicked();
    void on_btnCancel_clicked();

    void highlightCurrentLine();

    void readCncContext();
    void readAdc();

    void onFrameChanged(int frame_num, const fpoint_t& bot_pos, const fpoint_t& top_pos);

public:
//    explicit FormRun(QWidget *parent = nullptr);
    FormRun(ProgramParam& par, QWidget *parent = nullptr);
    ~FormRun();

    void setFontPointSize(int pointSize);

    void init(bool backup);

signals:
    void homePageClicked();
    void helpPageClicked(const QString& file_name);
};

#endif // FORMRUN_H
