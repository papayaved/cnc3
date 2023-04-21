#ifndef FORMTEST_H
#define FORMTEST_H

#include <QWidget>
#include <QTextEdit>
#include "program_param.h"
#include "aux_items.h"

namespace Ui {
class FormTest;
}

class FormTest : public QWidget {
    Q_OBJECT

public:
//    explicit FormTest(QWidget *parent = nullptr);
    FormTest(ProgramParam& par, QWidget* parent = nullptr);
    ~FormTest();

    size_t m_pct_pos;

signals:
    void homePageClicked();
    void editPageClicked();

private slots:
    void on_btnHome_clicked();
    void on_btnQuickTest_clicked();
    void on_btnFullTest_clicked();
    void on_btnGCode_clicked();

    void on_btnImit_clicked();

    void on_btnReadGCode_clicked();

    void on_btnMemTest_clicked();

    void on_btnConn_clicked();

private:
    const std::string job_dir = "c:\\Work\\Meatec\\Prototype\\NC\\";

    Ui::FormTest *ui;
    ProgramParam& par;
    auxItems::Reporter m_report;

    void clear();
    void writeln(const QString& s);

    void showPctForce(int pct);
    void showPct(int pct);
};

#endif // FORMTEST_H
