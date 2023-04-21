#ifndef FORM_MULT_H
#define FORM_MULT_H

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
#include "program_param.h"

class FormMult : public QWidget {
    Q_OBJECT

    static constexpr size_t SPLIT_POINTS = 4;
    const QString help_file = "mux.html";
    ProgramParam& par;
    size_t m_ctr_num {0};

    QLabel* labelTitle {nullptr};
    QGridLayout* gridMuxTitle {nullptr};

    QLabel *labelRowDir {nullptr};
    QRadioButton *radioX {nullptr}, *radioY {nullptr};
    QGroupBox* groupDir {nullptr};

    QLabel *labelItems {nullptr}, *labelRows {nullptr};
    QSpinBox *numItems {nullptr}, *numRows {nullptr};

    QLabel *labelItemsGap {nullptr}, *labelRowsGap {nullptr};
    QDoubleSpinBox *fnumItemsGap {nullptr}, *fnumRowsGap {nullptr};

    QCheckBox *checkAutoSplit {nullptr};

    QLabel *labelSplit[SPLIT_POINTS] {nullptr};
    QSpinBox *numSplit[SPLIT_POINTS] {nullptr};

    QLabel *labelEntryLen {nullptr}, *labelExitLen {nullptr};
    QDoubleSpinBox *fnumEntryLen {nullptr}, *fnumExitLen {nullptr};

    QPushButton *btnOK {nullptr}, *btnCancel {nullptr}, *btn2 {nullptr}, *btn3 {nullptr}, *btn4 {nullptr}, *btn5 {nullptr}, *btn6 {nullptr},
                *btn7 {nullptr}, *btn8 {nullptr}, *btn9 {nullptr}, *btn10 {nullptr}, *btn11 {nullptr}, *btnDefault {nullptr}, *btnHelp {nullptr};

    QGridLayout *gridButtons {nullptr};

    std::vector<QWidget*> widgets;

    QGridLayout *gridMux {nullptr};

    QVBoxLayout *mainLayout {nullptr};

    void createMux();
    void createButtons();

    void splitEnable(bool ena);

    void on_btnOK_clicked();

    void setSplitPoints();
    void setSplitPoints(const std::vector<size_t>& pts_num);
    std::vector<size_t> getSplitPoints();

    void setDefault();

private slots:
    void on_checkAutoSplit_clicked(bool checked);

public:
    FormMult(ProgramParam& par, QWidget *parent = nullptr);
    ~FormMult();

    void init(size_t ctr_num = 0);

    void setFontPointSize(int pointSize);

    size_t items() const;
    size_t rows() const;
    AXIS axis() const;
    DIR items_dir() const;
    DIR rows_dir() const;

    double gapItems() const;
    double gapRows() const;

    double lengthEntry() const;
    double lengthExit() const;

signals:
    void backPageClicked();
    void helpPageClicked(const QString& file_name);
};

#endif // FORM_MULT_H
