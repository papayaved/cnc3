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

    const QString m_help_file = "run.html";
    const unsigned POLLING_TIME = 1000 / 5; // ms
    const unsigned REMAIN_TIMER_MAX = 2000 / POLLING_TIME - 1;
    const unsigned ADC_POLLING_TIME = 1000; // ms

    ProgramParam& m_par;
    RunWidget* m_runWidget;
    bool m_cncReaderEna {false}, m_adcEnable {false};
    std::list<std::string> m_gframes;

    QLabel *m_labelWidth, *m_labelRatio, *m_labelCurrent, *m_labelRoll;
    QSpinBox *m_numWidth, *m_numRatio, *m_numCurrent, *m_numRollVel;
    std::vector<QSpinBox*> m_numBoxes;
    QGroupBox *m_groupWidth, *m_groupRatio, *m_groupCurrent, *m_groupRoll;

    QPushButton *m_btnHome, *m_btnRoll, *m_btnPump, *m_btnBreak, *m_btnHighVolt, *m_btnWidthDec, *m_btnWidthInc, *m_btnRatioInc, *m_btnRatioDec,\
        *m_btnCurrentDec, *m_btnCurrentInc, *m_btnRollVelDec, *m_btnRollVelInc, *m_btnHelp;

#ifndef STONE
    QPushButton *m_btnLowHighVolt {nullptr};
#endif

    std::vector<QPushButton*> m_buttons;

    QGridLayout* m_gridButtons;
    QVBoxLayout* m_mainLayout;

    int m_currentCursorPosition;

    bool m_cutStateAbortReq = false;
    WireSpeed m_speed;

    StartStopElapsedTimer* m_timer;
    unsigned m_remain_tmr;
    double m_full_length {0};

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
