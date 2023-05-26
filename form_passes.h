#ifndef FORM_PASSES_H
#define FORM_PASSES_H

#include <QWidget>
#include <QRadioButton>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QGridLayout>
#include <QTableView>
#include "program_param.h"

class FormPasses : public QWidget {
    Q_OBJECT

    const QString help_file = "passes.html";
    ProgramParam& par;

    QRadioButton *radioLeftSide {nullptr}, *radioRightSide {nullptr};
    QGroupBox* groupSide {nullptr};

    QLabel *labelTimes {nullptr}, *labelOvercut {nullptr}, *labelTab {nullptr};
    QLabel** labelPasses {nullptr};
    QLabel** labelModes {nullptr};

    QLabel *labelTabOffset {nullptr}, *labelTabMode {nullptr};

    QComboBox *comboTimes {nullptr};
    QDoubleSpinBox *inputOvercut {nullptr}, *inputTab {nullptr};

    QDoubleSpinBox** inputOffsets {nullptr};
    QComboBox** comboModes {nullptr};

    QLabel* labelCutMode {nullptr};
    QComboBox* comboCutLineMode {nullptr};

    QRadioButton *onePassTab {nullptr}, *multiPassTab {nullptr};
    QGroupBox *groupTab {nullptr};
    QDoubleSpinBox* inputTabOffset {nullptr};
    QComboBox* comboTabMode {nullptr};
    QCheckBox *checkUseLastSeg {nullptr}, *checkTabPause {nullptr}, *checkTapered {nullptr};

    QLabel* labelPumpDelay {nullptr};
    QSpinBox* inputPumpDelay {nullptr};
    QCheckBox* checkPumpPause {nullptr};

    QLabel* labelSpeed {nullptr};
    QDoubleSpinBox* inputSpeed {nullptr};

    QGroupBox *groupTapered {nullptr}, *groupD {nullptr}, *groupAxis {nullptr};
    QLabel *labelL {nullptr}, *labelH {nullptr}, *labelT {nullptr}, *labelD {nullptr}, *labelWireD {nullptr}, *labelAxis {nullptr};
    QDoubleSpinBox *inputL {nullptr}, *inputH {nullptr}, *inputT {nullptr}, *inputD {nullptr}, *inputWireD {nullptr};
    QRadioButton *radioX {nullptr}, *radioY {nullptr};

    QPushButton *btnBack {nullptr}, *btnOpen {nullptr}, *btnSave {nullptr}, *btnSaveAs {nullptr}, *btnDefault {nullptr},
        *btn5 {nullptr}, *btn6 {nullptr}, *btn7 {nullptr}, *btn8 {nullptr}, *btn9 {nullptr}, *btn10 {nullptr}, *btn11 {nullptr}, *btnGen {nullptr}, *btnHelp {nullptr};
    QGridLayout* gridButtons {nullptr};

    QGridLayout *gridPasses {nullptr}, *gridPassMode {nullptr};
    QVBoxLayout* mainLayout {nullptr};

    QLabel* tableTitle {nullptr};
    QTableView* tableModes {nullptr};
    QPushButton *btnNewMode {nullptr}, *btnDeleteMode {nullptr}, *btnOpenModes {nullptr}, *btnSaveModes {nullptr}, *btnSaveAsModes {nullptr}, *btnDefaultModes {nullptr};
    QVBoxLayout *vboxEdit {nullptr};
    QGridLayout* gridModes {nullptr};

    std::vector<QPushButton*> buttons;
    std::vector<QLabel*> labels;
    std::vector<QRadioButton*> radio;
    std::vector<QComboBox*> combo;
    std::vector<QCheckBox*> checks;
    std::vector<QDoubleSpinBox*> nums;
    std::vector<QGroupBox*> groups;

    size_t m_pass_num {0};
    int m_row {0};

    bool m_uv_ena {false}, m_D_ena {false};

    void createPasses();

    void resizeModeView();
    void createModeView();
    void createButtons();

    void initComboMode();
    void setCutTimes(int times);

    void bindModesFile();
    void unbindModesFile();
    bool bindedModesFile() const;

    void initTableModes();

private slots:
    void on_btnOpen_clicked();
    void on_btnSave_clicked();
    void on_btnSaveAs_clicked();

    void on_btnOpenModes_clicked();
    void on_btnSaveModes_clicked();
    void on_btnSaveAsModes_clicked();

//    void on_tableMode_clicked(const QModelIndex& index);

public:
//    explicit FormPasses(QWidget *parent = nullptr);
    FormPasses(ProgramParam& par, QWidget *parent = nullptr);
    ~FormPasses();

    void init(bool uv_ena = false);

    void setFontPointSize(int pointSize);

signals:
    void backPageClicked();
    void generateClicked();
    void helpPageClicked(const QString& file_name);
};

#endif // FORM_PASSES_H
