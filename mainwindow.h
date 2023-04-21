#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "main.h"
#include <QMainWindow>
#include "form_home.h"
#include "form_edit.h"
#include "form_run.h"
#include "form_test.h"
#include <QStackedWidget>
#include "program_param.h"
#include <QLCDNumber>
#include "form_contour.h"
#include "form_center.h"

#include "form_mult.h"

#if defined(STONE)
    #include "form_passes_stone.h"
#else
    #include "form_passes.h"
#endif

#include "form_settings.h"
#include "form_pult.h"
#include "form_help.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
//    Q_PROPERTY(ProgramParam param READ getParam WRITE setParam)

private:
    static int cnt;

    Ui::MainWindow *ui {nullptr};
    QLabel *labelMeatec {nullptr}, *labelTitle {nullptr}, *labelTime {nullptr};

    FormHome* m_formHome {nullptr};
    FormEdit* m_formEdit {nullptr};
    FormRun* m_formRun {nullptr};
    FormTest* m_formTest {nullptr};
    FormContour* m_formContour {nullptr};
    FormSettings* m_formSettings {nullptr};
    FormMult* m_formMux {nullptr};
#if defined(STONE)
    FormPassesStone* m_formPasses {nullptr};
#else
    FormPasses* m_formPasses {nullptr};
#endif
    FormPult* m_formPult {nullptr};
    FormCenter* m_formCenter {nullptr};
    FormHelp* m_formHelp {nullptr};
    QWidget* m_helpWidget {nullptr};

    QStackedWidget* m_stackedWidget {nullptr};
    ProgramParam par;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void connectCnc();

private slots:
    void onHomePageClickedUnlocked();
    void onHomePageClicked();
    void onCncError(const std::string& s);

    void onContourPageClicked();
    void onGenerateClicked();
    void onEditPageClicked(bool plot);
    void onRunPageClicked(bool backup);
    void onSettingsPageClicked();
    void onPultPageClicked();
    void onCenterPageClicked();
    void onTestPageClicked();
    void onFileNameChanged(const QString& file_name);

    void onMuxPageClicked(size_t ctr_num);
    void onPassesPageClicked(bool one_pass = false);

    void onHelpPageClicked(QWidget* source, const QString& helpHtmlFileName);
    void onBackHelpClicked();

    void showTime();

//public slots:
//    void onLoad();

private:
    void updateFontPointSize();

protected:
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void showEvent(QShowEvent *event);
};

#endif // MAINWINDOW_H
