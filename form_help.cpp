#include "form_help.h"

#include <QFile>
#include <QCoreApplication>

FormHelp::FormHelp(const ProgramParam& par, QWidget *parent) : QWidget(parent), par(par) {
    this->setObjectName(tr("Help"));
    createButtons();

    browser = new QTextBrowser(this);
    browser->setOpenLinks(true);
    browser->setOpenExternalLinks(true);
    browser->setSearchPaths({"Help/" + par.helpSubDir()});
    browser->setContextMenuPolicy(Qt::NoContextMenu);
    QFont font = browser->font();
    font.setPointSize(14);
    browser->setFont(font);

    setFontPointSize(14);

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(browser);
    mainLayout->addLayout(gridButtons);

    this->setLayout(mainLayout);
}

void FormHelp::init(const QString& helpHtmlFile) {
    QUrl url = QUrl("Help/" + par.helpSubDir() + "/" + helpHtmlFile);

    if (url.isValid() && QFile::exists(url.path()))
        browser->setSource(url);
    else {
        QString html = R"(<h1 align="center">)" + tr("Help file is not found") + + R"(: </h1>)" +
                R"(<h3 align="center">)" + QCoreApplication::applicationDirPath() + "/" + url.path() + R"(</h3>)";
        browser->setHtml(html);
    }

    browser->setFocus(Qt::FocusReason::OtherFocusReason);
}

void FormHelp::setFontPointSize(int pointSize) {
    for (QPushButton* b: buttons) {
        QFont font = b->font();
        font.setPointSize(pointSize);
        b->setFont(font);
    }
}

void FormHelp::createButtons() {
    btnBack = new QPushButton(tr("Back"));
    btnBack->setStatusTip(tr("Quit Help") + "   Alt+Left");
    btnBack->setShortcut(QKeySequence::Back);

    btn1 = new QPushButton();
    btn1->setEnabled(false);

    btn2 = new QPushButton();
    btn2->setEnabled(false);

    btn3 = new QPushButton();
    btn3->setEnabled(false);

    btn4 = new QPushButton();
    btn4->setEnabled(false);

    btn5 = new QPushButton();
    btn5->setEnabled(false);

    btn6 = new QPushButton();
    btn6->setEnabled(false);

    btn7 = new QPushButton();
    btn7->setEnabled(false);

    btn8 = new QPushButton();
    btn8->setEnabled(false);

    btn9 = new QPushButton();
    btn9->setEnabled(false);

    btn10 = new QPushButton();
    btn10->setEnabled(false);

    btn11 = new QPushButton();
    btn11->setEnabled(false);

    btn12 = new QPushButton();
    btn12->setEnabled(false);

    btn13 = new QPushButton();
    btn13->setEnabled(false);

    buttons = {btnBack, btn1, btn2, btn3, btn4, btn5, btn6, btn7, btn8, btn9, btn10, btn11, btn12, btn13};

    gridButtons = new QGridLayout();

    gridButtons->addWidget(btnBack, 0, 0);
    gridButtons->addWidget(btn1, 0, 1);
    gridButtons->addWidget(btn2, 0, 2);
    gridButtons->addWidget(btn3, 0, 3);
    gridButtons->addWidget(btn4, 0, 4);
    gridButtons->addWidget(btn5, 0, 5);
    gridButtons->addWidget(btn6, 0, 6);
    gridButtons->addWidget(btn7, 0, 7);
    gridButtons->addWidget(btn8, 0, 8);
    gridButtons->addWidget(btn9, 0, 9);
    gridButtons->addWidget(btn10, 0, 10);
    gridButtons->addWidget(btn11, 0, 11);
    gridButtons->addWidget(btn12, 0, 12);
    gridButtons->addWidget(btn13, 0, 13);

    connect(btnBack, &QPushButton::clicked, this, [&]() {
        emit backClicked();
        browser->clear();
    });
}
