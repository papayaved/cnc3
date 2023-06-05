#include <QTimer>
#include <QDebug>
#include <QProcess>
#include <QMessageBox>

#include "form_home.h"
#include "aux_items.h"
#include "cnc.h"
#include "debug.h"

using namespace std;

FormHome::FormHome(ProgramParam& par, QWidget *parent) : QWidget(parent), par(par) {
    this->setObjectName(tr("Home"));
    createButtons();

    QHBoxLayout *hboxApps = new QHBoxLayout;

    treeApps = new QTreeWidget;
    treeApps->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    treeApps->setHeaderHidden(true);
    treeApps->setWordWrap(true);
    treeApps->setAutoFillBackground(true);
    treeApps->setColumnCount(1);

    itemWelcome = new QTreeWidgetItem(treeApps);
    itemWelcome->setText(0, tr("Welcome"));
    treeApps->addTopLevelItem(itemWelcome);

    itemSlicing = new QTreeWidgetItem(treeApps);
    itemSlicing->setText(0, tr("Slicing"));
    treeApps->addTopLevelItem(itemSlicing);

    itemComb = new QTreeWidgetItem(treeApps);
    itemComb->setText(0, tr("Slots"));
    treeApps->addTopLevelItem(itemComb);

#ifndef STONE
    itemUVSizing = new QTreeWidgetItem(treeApps);
    itemUVSizing->setText(0, tr("Tapered Cutting"));
    treeApps->addTopLevelItem(itemUVSizing);
#else
    itemUVSizing = nullptr;
#endif

    itemWelcome->setSelected(true);

    // Welcome widget
    widgetWelcome = new QWidget;

    txtWelcome = new QTextEdit;
    txtWelcome->setEnabled(false);
    txtWelcome->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    labelAbout = new QLabel(R"(<a href='http://example.com' style='color: black'>)" + tr("About") + R"(</a>)");
    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->addStretch(0);
    hbox->addWidget(labelAbout);

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addWidget(txtWelcome);
    vbox->addLayout(hbox);

    widgetWelcome->setLayout(vbox);
#ifdef DARK_GUI_THEME
    txtWelcome->setTextColor(Qt::magenta);
#endif

    widgetSlicing = new SlicingWidget(par, this);
    widgetSlicing->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    widgetComb = new SlotsWidget(par, this);
    widgetComb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    widgetUVSizing = new UVSizingWidget(par, this);
    widgetUVSizing->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    stackedWidget = new QStackedWidget;
    stackedWidget->addWidget(widgetWelcome);
    stackedWidget->addWidget(widgetSlicing);
    stackedWidget->addWidget(widgetComb);
    stackedWidget->addWidget(widgetUVSizing);

    stackedWidget->setCurrentWidget(widgetWelcome);

    hboxApps->addWidget(treeApps);
    hboxApps->addWidget(stackedWidget);
    hboxApps->setStretch(0, 2);
    hboxApps->setStretch(1, 12);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(hboxApps);
    mainLayout->addLayout(gridButtons);

    this->setLayout(mainLayout);

    //    treeBook->itemAt(0,0)->setBackgroundColor(0, Qt::blue);
    connect(treeApps, &QTreeWidget::itemClicked, this, &FormHome::onItemClicked);
    connect(treeApps, &QTreeWidget::itemActivated, this, &FormHome::onItemClicked);

    connect(widgetSlicing, &SlicingWidget::clicked, this, [&]()     { emit editPageClicked(true); /*emit runPageClicked(false);*/ });
    connect(widgetComb, &SlotsWidget::clicked, this, [&]()          { emit editPageClicked(true); /*emit runPageClicked(false);*/ });
    connect(widgetUVSizing, &UVSizingWidget::clicked, this, [&]()   { emit editPageClicked(true); });

    connect(labelAbout, &QLabel::linkActivated, this, [&](const QString& /*link*/) {
#ifdef STONE
    const QString CNC_PROGRAM_TYPE = " (" + tr("stone edition") + ")";
#else
    const QString CNC_PROGRAM_TYPE = "";
#endif

        const QString PROGRAM_NAME =    tr("Desktop app ver.") + " " + APP_VER + CNC_PROGRAM_TYPE + "<br>" +
                                        tr("built on") + " " + __DATE__ + " " + __TIME__;

        QMessageBox msgBox(this);

        msgBox.setText(
            PROGRAM_NAME + "<br><br>" +
            tr("Company") + ":<br>" + tr("OOO NPP Meatec") + "<br><a href='https://meatec.ru/'>meatec.ru</a><br><br>" +
            tr("Email") + ":<br>" + "<a href='mailto:support@meatec.ru'>support@meatec.ru</a><br><br>" +            
            tr("Author") + ":<br>" + tr("Mikhail Menshikov")
        );

        msgBox.exec();
    });
}

void FormHome::onItemClicked(QTreeWidgetItem *item, int) {
    if (item == itemWelcome) {
        stackedWidget->setCurrentWidget(widgetWelcome);
    }
    else if (item == itemSlicing) {
        stackedWidget->setCurrentWidget(widgetSlicing);
    }
    else if (item == itemComb) {
        stackedWidget->setCurrentWidget(widgetComb);
    }
    else if (itemUVSizing && item == itemUVSizing) {
        stackedWidget->setCurrentWidget(widgetUVSizing);
    }
}

void FormHome::moveToWelcomeWidget() {
    stackedWidget->setCurrentWidget(widgetWelcome);
    treeApps->clearSelection();
    itemWelcome->setSelected(true);
}

FormHome::~FormHome() {}

void FormHome::createButtons() {
    btnHome = new QPushButton(tr("Home"));
    btnHome->setEnabled(false);

    btnContour = new QPushButton(tr("Contour"));
    btnContour->setStatusTip(tr("Load and edit contour"));

    btnGCode = new QPushButton(tr("G-code"));
    btnGCode->setStatusTip(tr("Open G-code editor"));

    btnRun = new QPushButton(tr("Run"));
    btnRun->setStatusTip(tr("Open Work panel"));

    btnSettings = new QPushButton(tr("Settings"));
    btnSettings->setStatusTip(tr("Change CNC settings"));

    btnPult = new QPushButton(tr("Pult"));
    btnPult->setStatusTip(tr("Manual ruling of the CNC"));

#ifndef STONE
    btnCenter = new QPushButton(tr("Center"));
#else
    btnCenter = new QPushButton;
    btnCenter->setEnabled(false);
#endif

    btnRec = new QPushButton(tr("Recovery"));
    btnRec->setStatusTip(tr("Recovery the last job"));

    btn8 = new QPushButton;
    btn8->setEnabled(false);

    btn9 = new QPushButton;
    btn9->setEnabled(false);

#if defined(TEST_PANEL)
    btnTest = new QPushButton(tr("Test"));
    btnTest->setStatusTip(tr("Diagnostic Panel"));
#else
    btnTest = new QPushButton;
    btnTest->setEnabled(false);
#endif

    btnMinimize = new QPushButton(tr("Minimize"));
    btnMinimize->setStatusTip(tr("Minimize CNC program and open Linux desktop"));

    btnShutdown = new QPushButton(tr("Shutdown"));
    btnShutdown->setStatusTip(tr("Turn off the computer"));
//    btnShutdown->setEnabled(false);

    btnHelp = new QPushButton(tr("Help"));
    btnHelp->setStatusTip(tr("Open Help") + "   F1");
    btnHelp->setShortcut(QKeySequence::HelpContents);

    buttons = {
        btnHome, btnContour, btnGCode, btnRun, btnSettings, btnPult, btnCenter, btnRec,
        btn8, btn9, btnTest, btnMinimize, btnShutdown, btnHelp
    };

    gridButtons = new QGridLayout;

    gridButtons->addWidget(btnHome, 0, 0);
    gridButtons->addWidget(btnContour, 0, 1);
    gridButtons->addWidget(btnGCode, 0, 2);
    gridButtons->addWidget(btnRun, 0, 3);
    gridButtons->addWidget(btnSettings, 0, 4);
    gridButtons->addWidget(btnPult, 0, 5);

#ifndef STONE
    gridButtons->addWidget(btnCenter, 0, 6);
    gridButtons->addWidget(btnRec, 0, 7);
#else
    gridButtons->addWidget(btnRec, 0, 6);
    gridButtons->addWidget(btnCenter, 0, 7);
#endif

    gridButtons->addWidget(btn8, 0, 8);
    gridButtons->addWidget(btn9, 0, 9);
    gridButtons->addWidget(btnTest, 0, 10);
    gridButtons->addWidget(btnMinimize, 0, 11);
    gridButtons->addWidget(btnShutdown, 0, 12);
    gridButtons->addWidget(btnHelp, 0, 13);

    connect(btnContour, &QPushButton::clicked, this, [&]() { emit contourPageClicked(); });
    connect(btnGCode, &QPushButton::clicked, this, [&]() { emit editPageClicked(); });
    connect(btnRun, &QPushButton::clicked, this, [&]() { emit runPageClicked(false); });
    connect(btnSettings, &QPushButton::clicked, this, [&]() { emit settingsPageClicked(); });
    connect(btnPult, &QPushButton::clicked, this, [&]() { emit pultPageClicked(); });
    connect(btnCenter, &QPushButton::clicked, this, [&]() { emit centerPageClicked(); });

    connect(btnRec, &QPushButton::clicked, this, [&]() {
        if (btnRec->signalsBlocked())
            return;

        btnRec->blockSignals(true);
        btnRec->setEnabled(false);

        par.loadBackup();

        if (!par.gcode.empty()) {
            // Local backup has high priority
            if (!par.cncContext.valid()) {
                cnc_context_t mcu_ctx = par.cnc.readBackup();
                toDebug(&mcu_ctx);

                if (mcu_ctx.field.backup_valid) { // todo: check parameters
                    par.cncContext.set(mcu_ctx);
                }
            } else
                toDebug(&par.cncContext.get());

            emit runPageClicked(true);
        }

        btnRec->blockSignals(false);
        btnRec->setEnabled(true);
    });

    connect(btnTest, &QPushButton::clicked, this, [&]() { emit testPageClicked(); });
    connect(btnMinimize, &QPushButton::clicked, this, [&]() { emit programMinimize(); });

    connect(btnShutdown, &QPushButton::clicked, this, [=]() {
        QMessageBox msgBox(this);
//        msgBox.setIcon(QMessageBox::Question); // don't work
        msgBox.setIcon(QMessageBox::Icon::NoIcon);
        msgBox.setWindowTitle(tr("Power off dialog"));
        msgBox.setText(R"(<b>)" + tr("Are you sure you want to turn off the computer now") + "?" + R"(</b>)");
//        msgBox.setInformativeText(tr("Quit?"));
        QPushButton *btnYes = msgBox.addButton(tr("Yes"), QMessageBox::ActionRole);
        QPushButton *btnNo = msgBox.addButton(tr("No"), QMessageBox::ActionRole);
        msgBox.setDefaultButton(btnNo);
        msgBox.exec();

        if (msgBox.clickedButton() == btnYes) {
#ifdef LINUX
            QProcess::startDetached("shutdown -P +0");
            qApp->quit();
#else
//            QProcess::startDetached("shutdown -s -f -t 00");
            QProcess::startDetached("shutdown", {"-s", "-f", "-t", "00"});
#endif
        }
    });

    connect(btnHelp, &QPushButton::clicked, this, [&]() { emit helpPageClicked(help_file); });    
}

// included reconnect
void FormHome::connectCnc(bool reset) {
    static int attempt_cnt = 1;
    string mcu_ver, fpga_ver;

    if (reset)
        attempt_cnt = 1;

    if (attempt_cnt > CONNECTION_ATTEMPTS) {
        if (par.cnc.isOpen())
            showNoConnection();
        else
            showComNotFound();

        return;
    }

    par.cncConnected = false;
    par.cnc.reconnect();

    showConnecting(attempt_cnt);
    qDebug() << "Connecting... Attempt " + QString::number(attempt_cnt);

    attempt_cnt++;

    if (!par.cnc.isOpen()) {
        QTimer::singleShot(5000, this, [&](){ this->connectCnc(false); });
        return;
    }

    if (!par.cnc.reset()) {
        QTimer::singleShot(5000, this, [&](){ this->connectCnc(false); });
        return;
    }
//    par.cnc.writeInputLevel(CncParam::inputLevel); // - There is in the reset command

    mcu_ver = par.cnc.readVersion();
    if (par.cnc.com().hasError()) {
        QTimer::singleShot(5000, this, [&](){ this->connectCnc(false); });
        return;
    }

    fpga_ver = par.cnc.m_fpga.readVersion();
    if (par.cnc.com().hasError()) {
        QTimer::singleShot(5000, this, [&](){ this->connectCnc(false); });
        return;
    }

    par.cncConnected = true;
    showVersion(mcu_ver, fpga_ver);
}

void FormHome::showConnecting(int attempt) {
#ifdef DARK_GUI_THEME
    const QString color = R"(LightGray)";
#else
    const QString color = R"(gray)";
#endif

    QString s = R"(<p><font color=)" + color + R"(>)" + tr("Connecting") + "... " + tr("Attempt") + " " + QString::number(attempt) + R"(</font></p>)";
    QString info = info_header + R"(<p style="text-align:center;"><h3>)" + s + R"(</h3></p>)";
    txtWelcome->setHtml(info);
}

void FormHome::showComNotFound() {
#ifdef DARK_GUI_THEME
    const QString errColor = R"(yellow)";
    const QString color = R"(LightYellow)";
#else
    const QString errColor = R"(red)";
    const QString color = R"(DarkGray)";
#endif

    QString s = R"(<p><font color=)" + errColor + R"(>)" + tr("Error") + ": " +
            R"(</font><i><font color=)" + color + R"(>)" +
                tr("Serial port is not found") +
            R"(</font></i></p>)";
    QString info = info_header + R"(<p style="text-align:right;">)" + s + R"(</p>)";
    txtWelcome->setHtml(info);
}

void FormHome::showNoConnection() {
#ifdef DARK_GUI_THEME
    const QString errColor = R"(yellow)";
    const QString color = R"(LightYellow)";
#else
    const QString errColor = R"(red)";
    const QString color = R"(red)";
#endif

    QString s = R"(<p><font color=)" + errColor + R"(>)" + tr("Error") + ": " +
            R"(</font><i><font color=)" + color + R"(>)" +
                tr("No connection. Check conection between computer and CNC board") +
            R"(</font></i></p>)";
    QString info = info_header + R"(<p style="text-align:right;">)" + s + R"(</p>)";
    txtWelcome->setHtml(info);
}

void FormHome::showVersion(const std::string &mcu, const std::string &fpga) {
#ifdef DARK_GUI_THEME
    QString info = info_header +
            R"(<font color=white><p style="text-align:center;"><h3>)" + QString(mcu.c_str()) + R"(</h3></p></font>)" +
            R"(<font color=white><p style="text-align:center;"><h3>)" + QString(fpga.c_str()) + R"(</h3></p></font>)";
#else
    QString info = info_header +
            R"(<font color=black><p style="text-align:center;"><h3>)" + QString(mcu.c_str()) + R"(</h3></p></font>)" +
            R"(<font color=black><p style="text-align:center;"><h3>)" + QString(fpga.c_str()) + R"(</h3></p></font>)";
#endif
    txtWelcome->setHtml(info);
}

void FormHome::setFontPointSize(int pointSize) {
    for (QPushButton* b: buttons) {
        QFont font = b->font();
        font.setPointSize(pointSize);
        b->setFont(font);
//        b->setStyleSheet("font: bold");
    }

    QFont font = treeApps->font();
    font.setPointSize(pointSize);
    treeApps->setFont(font);

    itemWelcome->setFont(0, font);
    itemSlicing->setFont(0, font);
    itemComb->setFont(0, font);
    if (itemUVSizing) itemUVSizing->setFont(0, font);

    widgetSlicing->setFontPointSize(pointSize);
    widgetComb->setFontPointSize(pointSize);
    widgetUVSizing->setFontPointSize(pointSize);

    font = labelAbout->font();
    font.setPointSize(pointSize);
    labelAbout->setFont(font);
}
