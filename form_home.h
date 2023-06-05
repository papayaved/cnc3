#ifndef FORMHOME_H
#define FORMHOME_H

#include <vector>

#include <QWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QTreeWidget>

#include <QVBoxLayout>
#include <QGridLayout>
#include <QStackedWidget>

#include "program_param.h"
#include "slicing_widget.h"
#include "slots_widget.h"
#include "uv_sizing_widget.h"
#include "main.h"

class FormHome : public QWidget {
    Q_OBJECT

    ProgramParam& par;

    QTreeWidget *treeApps;
    QStackedWidget* stackedWidget;

    QWidget* widgetWelcome;
    QTextEdit* txtWelcome;
    QLabel* labelAbout;

    SlicingWidget* widgetSlicing;
    SlotsWidget* widgetComb;
    UVSizingWidget* widgetUVSizing;

    QPushButton *btnHome, *btnContour, *btnGCode, *btnRun, *btnSettings, *btnRec, *btnPult, *btnCenter, *btn8, *btn9, *btnTest, *btnMinimize, *btnShutdown, *btnHelp;
    QGridLayout* gridButtons;
    std::vector<QPushButton*> buttons;

    QTreeWidgetItem *itemWelcome, *itemSlicing, *itemComb, *itemUVSizing;

#if defined(STONE)
    const QString TITLE = tr("Meatec CNC Wire-cut machine");
    const QString APP_TYPE = " (" + tr("stone") + ")";
#else
    const QString TITLE = tr("Meatec CNC Wire-cut EDM machine");
    const QString APP_TYPE = "";
#endif

    const QString APP_NAME = tr("App ver.") + " " + APP_VER + APP_TYPE + " " + tr("built on") + " " + __DATE__ + " " + __TIME__;

#ifdef DARK_GUI_THEME
    const QString info_header = R"(<h1 style="color:white; text-align:center;">)" + TITLE + R"(</h1>)"
                            + R"(<h3 style="color:white; text-align:center;">)" + APP_NAME + R"(</h3>)";
#else
    const QString info_header = R"(<h1 style="color:black; text-align:center;">)" + TITLE + R"(</h1>)"
                            + R"(<h3 style="color:black; text-align:center;">)" + APP_NAME + R"(</h3>)";
#endif
//    const QString info_header = R"(<h1 style="color:black; text-align:center;">)" + TITLE + R"(</h1>)";

    const QString help_file = "home.html";

    void createButtons();

    void showConnecting(int attempt);
    void showComNotFound();
    void showNoConnection();
    void showVersion(const std::string &mcu, const std::string &fpga);

public:
    explicit FormHome(ProgramParam& par, QWidget *parent = nullptr);
    ~FormHome();

    void setFontPointSize(int pointSize);
    void moveToWelcomeWidget();

signals:
    void contourPageClicked();
    void editPageClicked(bool plot = false);
    void runPageClicked(bool backup = false);
    void testPageClicked();
    void settingsPageClicked();
    void pultPageClicked();
    void centerPageClicked();
    void helpPageClicked(const QString& file_name);
    void programMinimize();

private slots:
    void onItemClicked(QTreeWidgetItem* item, int column);

public slots:
    void connectCnc(bool reset = true);
};

#endif // FORMHOME_H
