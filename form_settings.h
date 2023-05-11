#ifndef FORM_SETTINGS_H
#define FORM_SETTINGS_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QGroupBox>
#include <QCheckBox>
#include "program_param.h"

class FormSettings : public QWidget
{
    Q_OBJECT

    const QString help_file = "settings.html";
    ProgramParam& par;

    QVBoxLayout *mainLayout = nullptr;

    QPushButton *btnHome = nullptr, *btnRead = nullptr, *btnWrite = nullptr, *btnDefault = nullptr,\
        *btn4 = nullptr, *btn5 = nullptr, *btn6 = nullptr, *btn7 = nullptr, *btn8 = nullptr, *btn9 = nullptr, *btn10 = nullptr, *btn11 = nullptr, *btn12 = nullptr,\
        *btnHelp = nullptr;
    QGridLayout* gridButtons = nullptr;

    QWidget *widgetSettings = nullptr;
    QGridLayout *gridSettings = nullptr;

    QLabel *labelTitle = nullptr, *labelCNC = nullptr, *labelLanguage = nullptr, *labelInputLevel = nullptr, *labelStep = nullptr,\
        *labelX = nullptr, *labelY = nullptr, *labelU = nullptr, *labelV = nullptr, *labelEncX = nullptr,\
        *labelEncY = nullptr, *labelPrecision = nullptr, *labelMotor = nullptr, *labelEncoder = nullptr,
        *labelFbAcc = nullptr, *labelFbDec = nullptr, *labelAcc = nullptr, *labelDec = nullptr;

    QComboBox* comboLanguage = nullptr;
    QCheckBox *checkReverseX = nullptr, *checkReverseY = nullptr, *checkSwapXY = nullptr, *checkShowXY = nullptr,\
        *checkStepDirEnable = nullptr,\
        *checkReverseMotorX = nullptr, *checkReverseMotorY = nullptr, *checkReverseMotorU = nullptr, *checkReverseMotorV = nullptr,\
        *checkSwapMotorXY = nullptr, *checkSwapMotorUV = nullptr,\
        *checkReverseEncX = nullptr, *checkReverseEncY = nullptr;

    QSpinBox *numInputLevel = nullptr;
    QComboBox* comboInputLevel = nullptr;

    QGroupBox *groupFeedback = nullptr, *groupAcc = nullptr, *groupEnc = nullptr;
    QLabel *labelLowThld = nullptr, *labelHighThld = nullptr, *labelLowThld2 = nullptr, *labelHighThld2 = nullptr, *labelRbTimeout = nullptr, *labelRbAttempts = nullptr, *labelRbLength = nullptr, *labelRbSpeed = nullptr;
    QSpinBox *numLowThld = nullptr, *numHighThld = nullptr, *numLowThld2 = nullptr, *numHighThld2 = nullptr, *numRbTimeout = nullptr, *numRbAttempts = nullptr;
    QDoubleSpinBox *fnumRbLength = nullptr, *fnumRbSpeed = nullptr;

    QDoubleSpinBox *fnumStep = nullptr, *fnumScaleX = nullptr, *fnumScaleY = nullptr, *fnumScaleU = nullptr, *fnumScaleV = nullptr,\
        *fnumScaleEncX = nullptr, *fnumScaleEncY = nullptr,\
        *fnumFbAcc = nullptr, *fnumFbDec = nullptr,
        *fnumAcc = nullptr, *fnumDec = nullptr;

    std::vector<QCheckBox*> checks;
    std::vector<QDoubleSpinBox*> scaleNums;
    std::vector<QPushButton*> buttons;

    void createButtons();
    void createSettingsWidget();
    void init();
    void selectComboInputLevel(int bits);

    static QGroupBox* groupLabelNum(QLabel* label, QDoubleSpinBox* num);
    static QGroupBox* groupLabelNum(QLabel* label, QSpinBox* num, QComboBox* combo);
    static QGroupBox* groupLabelCombo(QLabel* label, QComboBox* combo);

    void onStateChangedMotor(int state);
    void onNumScaleChanged(double value);

//    void initSettings();

private slots:

public:
//    explicit FormSettings(QWidget *parent = nullptr);
    FormSettings(ProgramParam& par, QWidget *parent = nullptr);
    ~FormSettings();

    void reset();

    void setFontPointSize(int pointSize);

signals:
    void homePageClicked();
    void helpPageClicked(const QString& file_name);

    void showInfo(const QString& text);
    void showWarning(const QString& text);
    void showError(const QString& text);

    void fontChanged(int fontSize);
};

#endif // FORM_SETTINGS_H
