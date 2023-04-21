#include "form_edit.h"

#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QTextCodec>
#include <QSplitter>
#include <QList>
#include <algorithm>

using namespace std;

FormEdit::FormEdit(ProgramParam& par, QWidget *parent) :
    QWidget(parent),
    par(par),
    plotView(QwtPlotView()),
    select_ena(false),
    file_open(false)
{
    this->setObjectName(tr("G-code Editor"));
    createButtons();

    txtEditor = new CodeEditor();
    txtEditor->setMinimumWidth(400);

    txtMsg = new QTextEdit;
    txtMsg->setReadOnly(true);
    txtMsg->setMinimumHeight(100);

    QWidget* plotWidget = new QWidget;
    plotWidget->setMinimumSize(QSize(250, 250));
    QGridLayout* grid = new QGridLayout;

//    grid->addWidget(plotView.give(), 0, 1, Qt::AlignTop | Qt::AlignRight);
    grid->addWidget(plotView.widget(), 0, 1);
    plotWidget->setLayout(grid);

    splitterTxtPlot = new QSplitter(Qt::Horizontal);
    splitterTxtPlot->addWidget(txtEditor);
    splitterTxtPlot->addWidget(plotWidget);
    splitterTxtPlot->setCollapsible(0, false);

    splitterTxtMsg = new QSplitter(Qt::Vertical);
    splitterTxtMsg->addWidget(splitterTxtPlot);
    splitterTxtMsg->addWidget(txtMsg);
    splitterTxtMsg->setCollapsible(0, false);

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(splitterTxtMsg);
    mainLayout->addLayout(gridButtons);

    setFontPointSize(14);

    this->setLayout(mainLayout);

    connect(txtEditor, &CodeEditor::lineChanged, this, &FormEdit::onLineChanged);
    connect(txtEditor, &CodeEditor::textChanged, this, [&](){
        select_ena = false;
        btnSave->setEnabled(true);
    });

    connect(splitterTxtPlot, &QSplitter::splitterMoved, this, &FormEdit::onTxtPlotMoved);
    connect(splitterTxtMsg, &QSplitter::splitterMoved, this, &FormEdit::onTxtMsgMoved);

    init();
}

FormEdit::~FormEdit() {}

void FormEdit::setFontPointSize(int pointSize) {
    for (QPushButton* b: buttons) {
        QFont font = b->font();
        font.setPointSize(pointSize);
        b->setFont(font);
//        b->setStyleSheet("font: bold");
    }
}

QString FormEdit::getFileName() const {
    return file_open ? par.gcodeFileName : "";
}

void FormEdit::createButtons() {
    btnHome = new QPushButton(tr("Home"));
    btnHome->setStatusTip(tr("Go to the Home panel") + "   Alt+H");
    btnHome->setShortcut(QKeySequence("Alt+H"));

    btnNew = new QPushButton(tr("New"));
    btnNew->setStatusTip(tr("Create a new G-code file") + "   Ctrl+N");
    btnNew->setShortcut(QKeySequence("Ctrl+N"));

    btnOpen = new QPushButton(tr("Open"));
    btnOpen->setStatusTip(tr("Open G-code file") + "   Ctrl+O");
    btnOpen->setShortcut(QKeySequence("Ctrl+O"));

    btnSave = new QPushButton(tr("Save"));
    btnSave->setStatusTip(tr("Save G-code file") + "   Ctrl+S");
    btnSave->setShortcut(QKeySequence("Ctrl+S"));
    btnSave->setEnabled(false);

    btnSaveAs = new QPushButton(tr("Save as"));
    btnSaveAs->setStatusTip(tr("Save G-code file with a new name"));

    btnPlot = new QPushButton(tr("Plot"));
    btnPlot->setStatusTip(tr("Show the plot") + "   Ctrl+P");
    btnPlot->setShortcut(QKeySequence("Ctrl+P"));

//    btnPlay = new QPushButton(tr("Play"));
    btnPlay = new QPushButton;
    btnPlay->setEnabled(false);

    btnContour = new QPushButton(tr("to Contour"));
    btnContour->setStatusTip(tr("Convert a G-code file into a contour"));
//    btnContour->setEnabled(false);

    btn8 = new QPushButton;
    btn8->setEnabled(false);

    btn9 = new QPushButton;
    btn9->setEnabled(false);

    btn10 = new QPushButton;
    btn10->setEnabled(false);

    btn11 = new QPushButton;
    btn11->setEnabled(false);

    btnRun = new QPushButton(tr("Run"));
    btnRun->setStatusTip(tr("Open Work panel") + "   Ctrl+R");
    btnRun->setShortcut(QKeySequence("Ctrl+R"));

    btnHelp = new QPushButton(tr("Help"));
    btnHelp->setStatusTip(tr("Open Help") + "   F1");
    btnHelp->setShortcut(QKeySequence::HelpContents);

    gridButtons = new QGridLayout;

    gridButtons->addWidget(btnHome, 0, 0);
    gridButtons->addWidget(btnNew, 0, 1);
    gridButtons->addWidget(btnOpen, 0, 2);
    gridButtons->addWidget(btnSave, 0, 3);
    gridButtons->addWidget(btnSaveAs, 0, 4);
    gridButtons->addWidget(btnPlot, 0, 5);
    gridButtons->addWidget(btnContour, 0, 6);
    gridButtons->addWidget(btn8, 0, 7);
    gridButtons->addWidget(btn9, 0, 8);
    gridButtons->addWidget(btn10, 0, 9);
    gridButtons->addWidget(btn11, 0, 10);
    gridButtons->addWidget(btnPlay, 0, 11);
    gridButtons->addWidget(btnRun, 0, 12);
    gridButtons->addWidget(btnHelp, 0, 13);

    buttons = {btnHome, btnNew, btnOpen, btnSave, btnSaveAs, btnPlot, btnContour, btn8, btn9, btn10, btn11, btnPlay, btnRun, btnHelp};

    connect(btnHome, &QPushButton::clicked, this, [&]() { emit homePageClicked(); });
    connect(btnNew, &QPushButton::clicked, this, &FormEdit::on_btnNew_clicked);
    connect(btnOpen, &QPushButton::clicked, this, &FormEdit::on_btnOpen_clicked);
    connect(btnSave, &QPushButton::clicked, this, &FormEdit::on_btnSave_clicked);
    connect(btnSaveAs, &QPushButton::clicked, this, &FormEdit::on_btnSaveAs_clicked);
    connect(btnPlot, &QPushButton::clicked, this, &FormEdit::on_btnPlot_clicked);
    connect(btnPlay, &QPushButton::clicked, this, &FormEdit::on_btnPlay_clicked);
    connect(btnContour, &QPushButton::clicked, this, &FormEdit::on_btnContour_clicked);

    connect(btnRun, &QPushButton::clicked, this, &FormEdit::on_btnRun_clicked);
    connect(btnHelp, &QPushButton::clicked, this, [&]() { emit helpPageClicked(help_file); });
}

void FormEdit::init(bool auto_plot) {
    par.gcode.clear();
    par.gcodeSettings.clear();

    par.workContours.clear();
    par.mapGcodeToContours.clear();
    plot();
    txtEditor->setPlainText(par.gcodeText);
    showPlot();
    showMessage();
//    hidePlot();
//    hideMessage();
    select_ena = false;
//    btnContour->setEnabled(par.contours.empty());
    btnSave->setEnabled(false);

    if (auto_plot)
        on_btnPlot_clicked();
}

void FormEdit::on_btnNew_clicked() {
    par.gcodeText.clear();
    txtEditor->setPlainText(par.gcodeText);
    par.gcodeFileName.clear();
    par.saveSettings();
    btnSave->setEnabled(false);
    file_open = false;
    emit fileNameChanged("");
}

void FormEdit::on_btnOpen_clicked() {
    btnSave->setEnabled(false);
    file_open = false;
    emit fileNameChanged("");

    QDir dir(par.gcodeDir);

    if (!dir.exists()) {
        par.gcodeDir = QDir::homePath() + par.gcodeDirDefault;
        dir = QDir(par.gcodeDir);

        if (!dir.exists()) {
            bool OK = dir.mkpath(".");

            if (!OK)
                par.gcodeDir.clear();
        }
    }

    QString selectedFilter = tr("G-code files") + " (*.nc *.NC)";
    QString new_filePath = QFileDialog::getOpenFileName(this, tr("Open G-code file"),
                                par.gcodeDir + "/" + par.gcodeFileName,
                                selectedFilter + ";;" + tr("All files") + " (*)",
                                &selectedFilter
//                                , QFileDialog::DontUseNativeDialog
                            );

    if (new_filePath.length()) {
        QFileInfo fInfo(new_filePath);
        par.gcodeDir = fInfo.dir().path();
        par.gcodeFileName = fInfo.fileName();

        QFile file(par.gcodeDir + "/" + par.gcodeFileName);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            QMessageBox::critical(this, tr("G-code Editor: Open file error"), file.errorString());
        else {
            QTextStream stream(&file);
            stream.setCodec("Windows-1251");
            par.gcodeText = stream.readAll();
            txtEditor->setPlainText(par.gcodeText);
            file.close();
            par.saveSettings();
            btnSave->setEnabled(true);
            file_open = true;
            emit fileNameChanged(par.gcodeFileName);
        }
    }
}

void FormEdit::on_btnSave_clicked() {
    if (!par.gcodeDir.length() || !par.gcodeFileName.length() || !file_open)
        on_btnSaveAs_clicked();
    else {
        QFile file(par.gcodeDir + "/" + par.gcodeFileName);

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            QMessageBox::critical(this, tr("G-code Editor: Save file error"), file.errorString());
        else {
            par.gcodeText = txtEditor->toPlainText();

            QTextStream stream(&file);
            stream.setCodec("Windows-1251");
            stream << par.gcodeText;
            stream.flush();
            file.close();
        }
    }
}

void FormEdit::on_btnSaveAs_clicked() {
    QDir dir(par.gcodeDir);

    if (!dir.exists()) {
        par.gcodeDir = QDir::homePath() + par.gcodeDirDefault;
        dir = QDir(par.gcodeDir);

        if (!dir.exists()) {
            bool OK = dir.mkpath(".");

            if (!OK)
                par.gcodeDir.clear();
        }
    }

    QString new_filePath = QFileDialog::getSaveFileName(this, tr("Save G-code file"),
                               par.gcodeDir + "/" + par.gcodeFileName,
                               tr("G-code files") + " (*.NC *.nc)");

    if (new_filePath.length()) {
        QFileInfo fInfo(new_filePath);
        par.gcodeDir = fInfo.dir().path();        
        par.gcodeFileName = fInfo.completeBaseName() + ".NC";
        file_open = true;
        on_btnSave_clicked();
        par.saveSettings();
        btnSave->setEnabled(true);        
        emit fileNameChanged(par.gcodeFileName);
    }
}

void FormEdit::on_btnPlay_clicked() {
    if (txtEditor->isReadOnly()) {
        btnPlay->setText(tr("Play"));
        buttonsEnable();
        txtEditor->setReadOnly(false);
    }
    else {
        buttonsDisable(btnPlay);
        btnPlay->setText(tr("Stop"));
        txtEditor->setReadOnly(true);
        // run timer
    }
}

// onTimer
// Move cursor

void FormEdit::on_btnMsg_clicked() {
    if (splitterTxtMsg->sizes().size() >= 2 && splitterTxtMsg->sizes().at(1) != 0)
        hideMessage();
    else
        showMessage();
}

void FormEdit::on_btnPlot_clicked() {
    GCode gcode = compile();
    par.workContours = ContourList( gcode.getContours(&par.mapGcodeToContours) );

    if (!par.workContours.empty()) {
        showPlot();
        plot();
        select_ena = true;
    }
    else {
        if (splitterTxtPlot->sizes().size() >= 2 && splitterTxtPlot->sizes().at(1) != 0)
            hidePlot();
        else
            showPlot();
    }
}

void FormEdit::on_btnContour_clicked() {
    if (!par.contours.empty()) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle(tr("Warning"));
        msgBox.setText(R"(<b>)" + tr("The contour is not empty") + "." + R"(</b>)");
        msgBox.setInformativeText(tr("Do you want to overwrite it?"));
        QPushButton *btnYes = msgBox.addButton(tr("Yes"), QMessageBox::ActionRole);
        msgBox.addButton(tr("No"), QMessageBox::ActionRole);
        msgBox.setDefaultButton(btnYes);
        msgBox.exec();

        if (msgBox.clickedButton() != btnYes)
            return;

        par.contours.clear();
    }

    GCode gcode = compile();
    ContourList contours = ContourList( gcode.getContours(&par.mapGcodeToContours) );

    if (!contours.empty()) {
        par.contours = contours;
        emit contourPageClicked();
    }
}

void FormEdit::on_btnRun_clicked() {
    par.gcode = compile();
    par.gcodeSettings = par.gcode.getSettings();

    if (!par.gcode.empty())
        emit runPageClicked();
}

void FormEdit::on_btnImit_clicked() {
//    par.mode = CncMode::RUN_IMIT;
//    emit testPageClicked();
}

void FormEdit::onTxtPlotMoved(int /*pos*/, int /*index*/) {
    plotView.onResizeEvent( plotViewSize() );
}

void FormEdit::onTxtMsgMoved(int /*pos*/, int /*index*/) {
    plotView.onResizeEvent( plotViewSize() );
}

void FormEdit::onLineChanged(int row) {
    if (select_ena && !par.workContours.empty() && !par.mapGcodeToContours.empty()) {
        if (row >= 0)
            par.workContours.select( par.getDxfEntityNum(size_t(row)) );
        else
            par.workContours.clearSelected();

//        plotWidget->plot(par.workContours);
        plot();
    }
}

GCode FormEdit::compile() {
    const QString& txt = txtEditor->toPlainText();
    GCode gcode;
    int err = gcode.parse(txt.toStdString());

    if (err == 0) {
        string new_txt = gcode.toText();
        qDebug() << new_txt.c_str();
        par.gcodeText = new_txt.c_str();
        txtEditor->setPlainText(par.gcodeText);
        showMessage("Compilation completed successfully");
    }
    else {
        QTextCursor textCursor = txtEditor->textCursor();

        textCursor.movePosition(QTextCursor::Start);
        textCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, err - 1);

        txtEditor->setTextCursor(textCursor);

        showMessage("Error: compalation error at line " + QString::number(err));

        txtEditor->setFocus();
        gcode.clear();
    }

    return gcode;
}

void FormEdit::showMessage(QString s) {
    showMessage();
    txtMsg->setText(s);
}

void FormEdit::showPlot() {
    QList<int> sizes = splitterTxtPlot->sizes();

    if (sizes.size() >= 2) {
        int w = sizes[0] + sizes[1];
        splitterTxtPlot->setSizes({int(w * 4.0/14.0), w - int(w * 4.0/14.0)});
    }
}

void FormEdit::hidePlot() {
    splitterTxtPlot->setSizes({100, 0});
}

void FormEdit::showMessage() {
    QList<int> sizes = splitterTxtMsg->sizes();

    if (sizes.size() >= 2) {
        int w = sizes[0] + sizes[1];
        splitterTxtMsg->setSizes({w - 100, 100});
    }
}

void FormEdit::hideMessage() {
    splitterTxtMsg->setSizes({100, 0});
}

//void FormEdit::on_splitterEditorPlot_splitterMoved(int pos, int index) {}

void FormEdit::plot() {
    QList<int> sizes = splitterTxtPlot->sizes();

    if (sizes.size() >= 2) {
        int w = sizes[1];
        int h = splitterTxtPlot->size().height();
        QSize size = QSize(w, h);
        plotView.setSwapXY(par.swapXY);
        plotView.setInverseX(par.reverseX);
        plotView.setInverseY(par.reverseY);
        plotView.setShowXY(par.showXY);
        plotView.plot(par.workContours, size);
//        plotView.plot(par.workContours);
    }
}

QSize FormEdit::plotViewSize() const {
    QList<int> hsizes = splitterTxtPlot->sizes();
    QList<int> vsizes = splitterTxtMsg->sizes();

    if (hsizes.size() >= 2 && vsizes.size() >= 2) {
        int w = hsizes[1];
//        int h = splitterTxtPlot->size().height();
        int h = vsizes[0];
        QSize size(w, h);
        qDebug() << "Plot splitter size: " << size;
        return size;
    }
    return QSize(0, 0);
}

void FormEdit::resizeEvent(QResizeEvent* event) {
    showMessage();
    QSize size = plotViewSize();

    if (!plotView.onResizeEvent(size))
        event->ignore();
}

void FormEdit::keyPressEvent(QKeyEvent *e) {
    if (!e->text().isNull() && !e->text().isEmpty() && e->modifiers() == Qt::ControlModifier) {
        if (e->key() == Qt::Key_S || e->key() == Qt::Key_Save)
            on_btnSave_clicked();
        else if (e->key() == Qt::Key_O || e->key() == Qt::Key_Open)
            on_btnOpen_clicked();
        else
            QWidget::keyPressEvent(e);
    }
    else {
        QWidget::keyPressEvent(e);
    }
}

void FormEdit::buttonsDisable(QPushButton *except) {
    for (QPushButton* btn: buttons)
        if (btn != except)
            btn->setEnabled(false);
}

void FormEdit::buttonsEnable() {
    for (QPushButton* btn: buttons)
        btn->setEnabled(true);
}
