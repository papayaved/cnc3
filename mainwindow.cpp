#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QScreen>
#include <QTimer>
#include <QDate>
#include <QTime>
#include <QTranslator>
#include <QLocale>
#include <QMessageBox>

#include "main.h"
#include "contour_pass.h"

using namespace std;

int MainWindow::cnt = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

#if defined(FULL_SCREEN)
    setWindowState(Qt::WindowFullScreen);
#else
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect geometry = screen->availableGeometry();
//    this->setMaximumSize(QSize(geometry.width(), geometry.height()));
    this->resize(geometry.width() / 2, geometry.height() / 2);
//    this->setWindowState(Qt::WindowFullScreen);
#endif

    m_formHome = new FormHome(par, this);

    m_formContour = new FormContour(par, this);
    m_formMux = new FormMult(par, this);

    m_formEdit = new FormEdit(par, this);
    m_formRun = new FormRun(par, this);
    m_formSettings = new FormSettings(par, this);
    m_formPult = new FormPult(par, this);
#ifndef STONE
    m_formCenter = new FormCenter(par, this);
#endif
    m_formTest = new FormTest(par, this);

#if defined(STONE)
    m_formPasses = new FormPassesStone(par, this);
#else
    m_formPasses = new FormPasses(par, this);
#endif

    m_formHelp = new FormHelp(par, this);

    connect(&par.cnc, &Cnc::error, this, &MainWindow::onCncError);

    connect(m_formContour, &FormContour::homePageClicked, this, &MainWindow::onHomePageClicked);
    connect(m_formEdit, &FormEdit::homePageClicked, this, &MainWindow::onHomePageClicked);
    connect(m_formRun, &FormRun::homePageClicked, this, &MainWindow::onHomePageClicked);
    connect(m_formSettings, &FormSettings::homePageClicked, this, &MainWindow::onHomePageClicked);
    connect(m_formPult, &FormPult::homePageClicked, this, &MainWindow::onHomePageClicked);
    if (m_formCenter)
        connect(m_formCenter, &FormCenter::homePageClicked, this, &MainWindow::onHomePageClicked);
    connect(m_formTest, &FormTest::homePageClicked, this, &MainWindow::onHomePageClicked);

    connect(m_formHome, &FormHome::contourPageClicked, this, &MainWindow::onContourPageClicked);
    connect(m_formEdit, &FormEdit::contourPageClicked, this, [=]() {
        m_formContour->resetFileOpen();
        onContourPageClicked();
    });

#if defined(STONE)
    connect(m_formPasses, &FormPassesStone::backPageClicked, this, &MainWindow::onContourPageClicked);
    connect(m_formPasses, &FormPassesStone::generateClicked, this, &MainWindow::onGenerateClicked);
#else
    connect(m_formPasses, &FormPasses::backPageClicked, this, &MainWindow::onContourPageClicked);
    connect(m_formPasses, &FormPasses::generateClicked, this, &MainWindow::onGenerateClicked);
#endif

    connect(m_formMux, &FormMult::backPageClicked, this, &MainWindow::onContourPageClicked);

    connect(m_formHome, &FormHome::editPageClicked, this, [=](bool auto_plot) { onEditPageClicked(auto_plot); });
    connect(m_formTest, &FormTest::editPageClicked, this, [=]() { onEditPageClicked(false); });
    connect(m_formContour, &FormContour::editPageClicked, this, [=](bool auto_plot) {
        onEditPageClicked(auto_plot);
    });

    connect(m_formEdit, &FormEdit::runPageClicked, this, [=]() { onRunPageClicked(false); });
    connect(m_formHome, &FormHome::runPageClicked, this, [=](bool backup) { onRunPageClicked(backup); });

    connect(m_formHome, &FormHome::settingsPageClicked, this, &MainWindow::onSettingsPageClicked);
    connect(m_formHome, &FormHome::pultPageClicked, this, &MainWindow::onPultPageClicked);
    connect(m_formHome, &FormHome::centerPageClicked, this, &MainWindow::onCenterPageClicked);

    connect(m_formHome, &FormHome::testPageClicked, this, &MainWindow::onTestPageClicked);

    connect(m_formContour, &FormContour::muxPageClicked, this, &MainWindow::onMuxPageClicked);
    connect(m_formContour, &FormContour::passesPageClicked, this, &MainWindow::onPassesPageClicked);

    connect(m_formHome, &FormHome::helpPageClicked, this, [&](const QString& file_name) { onHelpPageClicked(m_formHome, file_name); });
    connect(m_formContour, &FormContour::helpPageClicked, this, [&](const QString& file_name) { onHelpPageClicked(m_formContour, file_name); });
    connect(m_formMux, &FormMult::helpPageClicked, this, [&](const QString& file_name) { onHelpPageClicked(m_formMux, file_name); });

#if defined(STONE)
    connect(m_formPasses, &FormPassesStone::helpPageClicked, this, [&](const QString& file_name) { onHelpPageClicked(m_formPasses, file_name); });
#else
    connect(m_formPasses, &FormPasses::helpPageClicked, this, [&](const QString& file_name) { onHelpPageClicked(m_formPasses, file_name); });    
#endif

    connect(m_formEdit, &FormEdit::helpPageClicked, this, [&](const QString& file_name) { onHelpPageClicked(m_formEdit, file_name); });
    connect(m_formRun, &FormRun::helpPageClicked, this, [&](const QString& file_name) { onHelpPageClicked(m_formRun, file_name); });
    connect(m_formSettings, &FormSettings::helpPageClicked, this, [&](const QString& file_name) { onHelpPageClicked(m_formSettings, file_name); });
    connect(m_formPult, &FormPult::helpPageClicked, this, [&](const QString& file_name) { onHelpPageClicked(m_formPult, file_name); });

    if (m_formCenter)
        connect(m_formCenter, &FormCenter::helpPageClicked, this, [&](const QString& file_name) { onHelpPageClicked(m_formCenter, file_name); });

    connect(m_formHelp, &FormHelp::backClicked, this, &MainWindow::onBackHelpClicked);

    connect(m_formSettings, &FormSettings::showInfo, this, [&](const QString& text) {
//        ui->statusBar->setStyleSheet("color: green; font: " + QString::number(par.fontSize) + "px;");
        // todo: html color
        statusBar()->showMessage(text);
    });

    connect(m_formSettings, &FormSettings::showWarning, this, [&](const QString& text) {
//        ui->statusBar->setStyleSheet("color: blue; font: " + QString::number(par.fontSize) + "px;");
        statusBar()->showMessage(tr("Warning") + ": " + text);
    });

    connect(m_formSettings, &FormSettings::showError, this, [&](const QString& text) {
//        ui->statusBar->setStyleSheet("color: red; font: bold " + QString::number(par.fontSize) + "px;");
        statusBar()->showMessage(tr("Error") + ": " + text);
    });

    connect(m_formHome, &FormHome::programMinimize, this, [&]() {
        setWindowState(Qt::WindowMinimized);
    });

    connect(m_formContour, &FormContour::fileNameChanged, this, &MainWindow::onFileNameChanged);
    connect(m_formEdit, &FormEdit::fileNameChanged, this, &MainWindow::onFileNameChanged);

    updateFontPointSize();

    m_stackedWidget = new QStackedWidget;

    m_stackedWidget->addWidget(m_formHome);
    m_stackedWidget->addWidget(m_formContour);
    m_stackedWidget->addWidget(m_formEdit);
    m_stackedWidget->addWidget(m_formRun);
    m_stackedWidget->addWidget(m_formSettings);
    m_stackedWidget->addWidget(m_formPult);

    if (m_formCenter)
        m_stackedWidget->addWidget(m_formCenter);

    m_stackedWidget->addWidget(m_formTest);

    m_stackedWidget->addWidget(m_formMux);
    m_stackedWidget->addWidget(m_formPasses);
    m_stackedWidget->addWidget(m_formHelp);

//    QVBoxLayout *layout = new QVBoxLayout;
//    layout->addWidget(stackedWidget);
//    setLayout(layout);

    this->setCentralWidget(m_stackedWidget);

//    for (auto s: QStyleFactory::keys()) {
//        qDebug() << s;
//    }
//    QApplication::setStyle(QStyleFactory::create(QStyleFactory::keys()[1]));
    labelMeatec = new QLabel(R"(<font color=white><b><h1>)" + tr("Meatec") + R"(</h1></b></font>)");
    labelTitle = new QLabel("File name");
    labelTime = new QLabel;
    showTime();
//    lcdTime = new QLCDNumber();
//    lcdTime->setSegmentStyle(QLCDNumber::SegmentStyle::Filled);

    QWidget* spacerWidget1 = new QWidget;
    if (spacerWidget1) {
        spacerWidget1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        spacerWidget1->setVisible(true);
    }

    QWidget* spacerWidget2 = new QWidget;
    if (spacerWidget2) {
        spacerWidget2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        spacerWidget2->setVisible(true);
    }

    ui->mainToolBar->setStyleSheet("background-color:orange;");
    ui->mainToolBar->addWidget(labelMeatec);
    ui->mainToolBar->addWidget(spacerWidget1);
    ui->mainToolBar->addWidget(labelTitle);
    ui->mainToolBar->addWidget(spacerWidget2);
    ui->mainToolBar->addWidget(labelTime);
//    ui->mainToolBar->addWidget(lcdTime);

    QTimer *timer = new QTimer(this);
    if (timer) {
        connect(timer, &QTimer::timeout, this, &MainWindow::showTime);
        timer->start(1000);
    }

//    statusBar()->showMessage("");

    onHomePageClicked();
}

void MainWindow::showTime() {
    QDateTime dt = QDateTime::currentDateTime();
    QLocale loc = ProgramParam::lang == InterfaceLanguage::RUSSIAN ? QLocale(QLocale::Russian, QLocale::Russia) : QLocale();

    QString s = R"(<h3><font color=black>)" + loc.toString(dt, QLocale::ShortFormat) + R"(</font><\h3>)";
//    s = R"(<h3>)" + dt.toString("dd.MM.yyyy hh:mm") + R"(<\h3>)";

    if (labelTime) labelTime->setText(s);
}

void MainWindow::updateFontPointSize() {
    if (m_formHome)     m_formHome->setFontPointSize(par.fontSize);
    if (m_formContour)  m_formContour->setFontPointSize(par.fontSize);
    if (m_formEdit)     m_formEdit->setFontPointSize(par.fontSize);
    if (m_formRun)      m_formRun->setFontPointSize(par.fontSize);
    if (m_formSettings) m_formSettings->setFontPointSize(par.fontSize);
    if (m_formPult)     m_formPult->setFontPointSize(par.fontSize);
    if (m_formCenter)   m_formCenter->setFontPointSize(par.fontSize);
//    m_formTest->setFontPointSize(par.fontSize);
    if (m_formPasses)   m_formPasses->setFontPointSize(par.fontSize);
    if (m_formHelp)     m_formHelp->setFontPointSize(par.fontSize);
    if (m_formMux)      m_formMux->setFontPointSize(par.fontSize);

    if (ui) {
        QFont font = ui->statusBar->font();
        font.setPointSize(par.fontSize);
        ui->statusBar->setFont(font);
//      ui->statusBar->setStyleSheet("color: black; font: " + QString::number(par.fontSize) + "px;");
    }
}

//void MainWindow::onLoad() { m_formHome->connectCnc(); }

MainWindow::~MainWindow() {
    if (ui) {
        delete ui;
        ui = nullptr;
    }
}

void MainWindow::onFileNameChanged(const QString& file_name) {
    labelTitle->setText(R"(<font color=black><h2>)" + file_name + R"(</h2></font>)");
}

void MainWindow::onContourPageClicked() {
    const QMutexLocker locker(&par.mutexParamLocker);

    if (m_stackedWidget && m_formContour) {
        onFileNameChanged(m_formContour->getFileName());
        this->setWindowTitle(m_formContour->objectName());
        m_stackedWidget->setCurrentWidget(m_formContour);
        m_formContour->init();
    }
}

void MainWindow::onEditPageClicked(bool auto_plot) {
    const QMutexLocker locker(&par.mutexParamLocker);

    if (m_stackedWidget && m_formEdit) {
        this->setWindowTitle(m_formEdit->objectName());
        onFileNameChanged(m_formEdit->getFileName());
        m_stackedWidget->setCurrentWidget(m_formEdit);
        m_formEdit->init(auto_plot);
    }
}

void MainWindow::onRunPageClicked(bool recovery) {
    const QMutexLocker locker(&par.mutexParamLocker);

    if (m_stackedWidget && m_formRun) {
        labelTitle->setText("");
        this->setWindowTitle(m_formRun->objectName());
        m_stackedWidget->setCurrentWidget(m_formRun);
        m_formRun->init(recovery);
    }
}

void MainWindow::onSettingsPageClicked() {
    const QMutexLocker locker(&par.mutexParamLocker);

    if (m_stackedWidget && m_formSettings) {
        labelTitle->setText("");
        this->setWindowTitle(m_formSettings->objectName());
        m_stackedWidget->setCurrentWidget(m_formSettings);
    }
}

void MainWindow::onPultPageClicked() {
    const QMutexLocker locker(&par.mutexParamLocker);

    if (m_stackedWidget && m_formPult) {
        labelTitle->setText("");
        this->setWindowTitle(m_formPult->objectName());
        m_stackedWidget->setCurrentWidget(m_formPult);
        m_formPult->init();
    }
}

void MainWindow::onCenterPageClicked() {
    const QMutexLocker locker(&par.mutexParamLocker);

    if (m_stackedWidget && m_formCenter) {
        labelTitle->setText("");
        this->setWindowTitle(m_formCenter->objectName());
        m_stackedWidget->setCurrentWidget(m_formCenter);
        m_formCenter->init();
    }
}

void MainWindow::onTestPageClicked() {
    const QMutexLocker locker(&par.mutexParamLocker);

    if (m_stackedWidget && m_formTest) {
        labelTitle->setText("");
        this->setWindowTitle(m_formTest->objectName());
        m_stackedWidget->setCurrentWidget(m_formTest);
    }
}

//
void MainWindow::onHomePageClickedUnlocked() {
    if (m_stackedWidget && m_formHome) {
        labelTitle->setText("");
        this->setWindowTitle(m_formHome->objectName());
        m_stackedWidget->setCurrentWidget(m_formHome);
        m_formHome->moveToWelcomeWidget();
    }
}

void MainWindow::onHomePageClicked() {
    const QMutexLocker locker(&par.mutexParamLocker);
    onHomePageClickedUnlocked();
}

void MainWindow::onCncError(const string &s) {
    std::cerr << ("CNC connection failed: " + s) << std::endl;

    QMessageBox::critical(
        this,
        "CNC connection failed",
        "CNC connection failed.\nApp will try to reconnect.\n\nError message: " + QString::fromStdString(s)
    );

    if (m_stackedWidget && m_formHome && m_stackedWidget->currentWidget() != m_formHome) {
        onHomePageClickedUnlocked();
        connectCnc();
    }
}

void MainWindow::connectCnc() {
    if (m_formHome) {
        m_formHome->connectCnc();

        if (par.cncConnected) {
#if defined(STONE)
            par.cnc.writeSemaphoreCncEnable(true);
#else
            par.cnc.writeCncEnable(true);
#endif
        }
    }
}

void MainWindow::onMuxPageClicked(size_t ctr_num) {
    const QMutexLocker locker(&par.mutexParamLocker);

    if (m_stackedWidget && m_formMux) {
        labelTitle->setText("");
        this->setWindowTitle(m_formMux->objectName());
        m_stackedWidget->setCurrentWidget(m_formMux);
        m_formMux->init(ctr_num);
    }
}

void MainWindow::onPassesPageClicked(bool one_pass) {
    const QMutexLocker locker(&par.mutexParamLocker);

    if (m_stackedWidget && m_formPasses) {
        labelTitle->setText("");
        this->setWindowTitle(m_formPasses->objectName());
        m_stackedWidget->setCurrentWidget(m_formPasses);
        m_formPasses->init(one_pass);
    }
}

void MainWindow::onHelpPageClicked(QWidget *source, const QString& helpHtmlFileName) {
    const QMutexLocker locker(&par.mutexParamLocker);

    if (m_stackedWidget && m_formHelp && source && !helpHtmlFileName.isEmpty()) {
        labelTitle->setText("");
        this->setWindowTitle(m_formHelp->objectName());
        m_helpWidget = source;
        m_stackedWidget->setCurrentWidget(m_formHelp);
        m_formHelp->init(helpHtmlFileName);
    }
}

void MainWindow::onBackHelpClicked() {
    const QMutexLocker locker(&par.mutexParamLocker);

    if (m_stackedWidget && m_helpWidget) {
        labelTitle->setText("");
        this->setWindowTitle(m_helpWidget->objectName()); // restore windows name
        m_stackedWidget->setCurrentWidget(m_helpWidget);
        m_helpWidget = nullptr;
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event && event->key() == Qt::Key::Key_F11) {
        setWindowState(isFullScreen() ? Qt::WindowNoState : Qt::WindowFullScreen);
    }
}

void MainWindow::showEvent(QShowEvent *event) {
    setWindowState(Qt::WindowFullScreen);

    if (cnt++ == 0)
        connectCnc();

    if (event)
        event->ignore();
}

void MainWindow::onGenerateClicked() {
    const size_t count = par.contours.size();

    if (count == 0) {
        m_formContour->setText(tr("No contour"));
        onContourPageClicked();
        return;
    }

    par.contours.sort();

    if (!par.contours.isSorted()) {
        m_formContour->setText(tr("G-code generation error: Contour not sorted"));
        onContourPageClicked();
        return;
    }

    const bool isCutLine = par.contours.at(0)->type() == CONTOUR_TYPE::CUTLINE_CONTOUR;

    if (count == 1 && isCutLine) {
        m_formContour->setText(tr("No contour. There is only a cutline"));
        onContourPageClicked();
        return;
    }

    const bool isLoop = count > 2 && par.contours.at(1)->isLoop();
    const bool hasOut = count > 2 && par.contours.at(1)->hasOut();

    // for loop with only incut automatic add outcut. Don't use
    if (count == 2 && isCutLine && isLoop && !hasOut) {
        ContourPair* incut = par.contours.at(0);
        ContourPair outcut(*incut);
        outcut.reverse();
        par.contours.push_back(std::move(outcut));
    }

    par.contours.moveTo(); // to (0,0)
    par.contours.setCutParam(par.cutParam, par.genModesList);

    ContourPass pass(par.contours);
    pass.generate();

    if (par.gcode.generate( par.cutParam, pass.contours(), CncParam() )) {
        par.gcodeText = par.gcode.toText().c_str();
        onEditPageClicked(true);
        return;
    }
    else
        m_formContour->setText(par.gcode.lastError().c_str());

    onContourPageClicked();
}
