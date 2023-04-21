#include "form_contour.h"

#include <QFileDialog>
//#include <QSizePolicy>
#include <QLineEdit>
#include <QHeaderView>
#include <QMessageBox>

#include "new_cutline_dialog.h"
#include "rotate_dialog.h"
#include "resize_dialog.h"
#include "contour_pass.h"
#include "frect_t.h"
#include "xml_ext.h"
#include "layer_dialog.h"
#include "dialog_segment_properties.h"
#include "dialog_contour_properties.h"

using namespace std;
//using namespace QtCharts;

FormContour::FormContour(ProgramParam& par, QWidget *parent) :
    QWidget(parent),
    par(par), m_plotView(QwtPlotView()),
    m_ctr_num(0), m_row_num(0), m_col_num(0),
    file_open(false)
{
    this->setObjectName(tr("Contour Editor"));
    createButtons();

    txtMsg = new QTextEdit();
    txtMsg->setReadOnly(true);

//    QSizePolicy spMsg(QSizePolicy::Maximum, QSizePolicy::Maximum);
//    spMsg.setHeightForWidth(true);
//    txtMsg->setSizePolicy(spMsg);

    createGridView();

    buttons = {
        btnHome, btnOpen, btnSave, btnSaveAs, btnLoadDxf, btnMulti, btn6, btn7, btn8, btn9, btn10, btn11, btnGenerate, btnHelp,
        btnNew, btnNewEntryLine, btnDelete, btnClear,
        btnFirst, btnUp, btnDown, btnLast, btnSort,
        btnProperties, btnEdit, btnUndo
    };

//    setFontPointSize(14);

    gridPlot = new QGridLayout();
    gridPlot->addLayout(gridView, 0, 0);
//    gridPlot->addWidget(plotView.give(), 0, 1, Qt::AlignLeft | Qt::AlignBottom);
    gridPlot->addWidget(m_plotView.widget(), 0, 1);
    gridPlot->addWidget(txtMsg, 1, 0, 1, 2);

    m_plot_view_pct = (14.0 - 3) / 14;

    gridPlot->setColumnStretch(0, 3);
    gridPlot->setColumnStretch(1, 14 - 3);

    gridPlot->setRowStretch(0, 9);
    gridPlot->setRowStretch(1, 1);

    mainLayout = new QVBoxLayout;
    mainLayout->addLayout(gridPlot);
    mainLayout->addLayout(gridButtons);

    this->setLayout(mainLayout);

    m_fontSize = btnHome->font().pointSize();

    connect(&m_plotView, &QwtPlotView::clicked, this, &FormContour::on_plotClicked);
}

void FormContour::createButtons() {
    btnHome = new QPushButton(tr("Home"));
    btnHome->setStatusTip(tr("Go to the Home panel") + "   Alt+H");
    btnHome->setShortcut(QKeySequence("Alt+H"));

    btnOpen = new QPushButton(tr("Open"));
    btnOpen->setStatusTip(tr("Open project from file") + "   Ctrl+O");
    btnOpen->setShortcut(QKeySequence("Ctrl+O"));

    btnSave = new QPushButton(tr("Save"));
    btnSave->setStatusTip(tr("Save project in the current file") + "   Ctrl+S");
    btnSave->setShortcut(QKeySequence("Ctrl+S"));

    btnSaveAs = new QPushButton(tr("Save as"));
    btnSaveAs->setStatusTip(tr("Save project as a new file"));

    btnLoadDxf = new QPushButton(tr("Load DXF"));
    btnLoadDxf->setStatusTip(tr("Load contour from DXF file"));

    btnMulti = new QPushButton(tr("Multiply"));
    btnMulti->setStatusTip(tr("Multiplicate the selected closed contour. All other contours will be removed."));

    btn6 = new QPushButton;
    btn6->setEnabled(false);

    btn7 = new QPushButton;
    btn7->setEnabled(false);

    btn8 = new QPushButton;
    btn8->setEnabled(false);

    btn9 = new QPushButton;
    btn9->setEnabled(false);

    btn10 = new QPushButton;
    btn10->setEnabled(false);

    btn11 = new QPushButton;
    btn11->setEnabled(false);

    btnGenerate = new QPushButton(tr("Generate"));
    btnGenerate->setStatusTip(tr("Setup cut settings and generate G-code"));

    btnHelp = new QPushButton(tr("Help"));
    btnHelp->setStatusTip(tr("Open Help") + "   F1");
    btnHelp->setShortcut(QKeySequence::HelpContents);

    gridButtons = new QGridLayout();

    gridButtons->addWidget(btnHome, 0, 0);
    gridButtons->addWidget(btnOpen, 0, 1);
    gridButtons->addWidget(btnSave, 0, 2);
    gridButtons->addWidget(btnSaveAs, 0, 3);
    gridButtons->addWidget(btnLoadDxf, 0, 4);    
    gridButtons->addWidget(btnMulti, 0, 5);
    gridButtons->addWidget(btn6, 0, 6);
    gridButtons->addWidget(btn7, 0, 7);
    gridButtons->addWidget(btn8, 0, 8);
    gridButtons->addWidget(btn9, 0, 9);
    gridButtons->addWidget(btn10, 0, 10);
    gridButtons->addWidget(btn11, 0, 11);
    gridButtons->addWidget(btnGenerate, 0, 12);
    gridButtons->addWidget(btnHelp, 0, 13);

    connect(btnHome, &QPushButton::clicked, this, [&]() { emit homePageClicked(); });

    connect(btnOpen, &QPushButton::clicked, this, &FormContour::on_btnOpen_clicked);
    connect(btnSave, &QPushButton::clicked, this, &FormContour::on_btnSave_clicked);
    connect(btnSaveAs, &QPushButton::clicked, this, &FormContour::on_btnSaveAs_clicked);

    connect(btnLoadDxf, &QPushButton::clicked, this, &FormContour::on_btnLoadDxf_clicked);    
    connect(btnMulti, &QPushButton::clicked, this, [&]() {
        on_btnSortClicked();
//        updateCurrentViewPos(); // updated

        const ContourPair* const pair = par.contours.at(m_ctr_num);

        if (pair && !pair->empty() && pair->type() == CONTOUR_TYPE::MAIN_CONTOUR && pair->isSorted() && pair->isLoop())
            emit muxPageClicked(m_ctr_num);
        else {
            QMessageBox::warning(this, tr("Incorrect contour"), tr("Please select a closed sorted contour"));
            return;
        }
    });

    connect(btnGenerate, &QPushButton::clicked, this, [&]() { emit passesPageClicked( par.contours.hasAnyTop() ); });
    connect(btnHelp, &QPushButton::clicked, this, [&](){ emit helpPageClicked(help_file); });
}

void FormContour::createGridView() {
    lineSegment = new QLineEdit;
    lineContour = new QLineEdit;

    viewContours = new QTableView;
    viewSegments = new QTableView;

    ContourTableModel* model = new ContourTableModel();

    viewContours->setModel(new ContoursModel());
    viewContours->horizontalHeader()->hide();

    viewSegments->setModel(model);

    actPropCtr = new QAction(tr("Properties"), this);
    actPropCtr->setStatusTip(tr("Current contour properties"));
    QFont act_bold_font = actPropCtr->font();
    act_bold_font.setBold(true);
    actPropCtr->setFont(act_bold_font);

    actSortCtr = new QAction(tr("Auto sort"), this);
    actSortCtr->setStatusTip(tr("Auto sort segments in the current contour"));

    actChangeDirCtr = new QAction(tr("Change Direction"), this);
    actChangeDirCtr->setStatusTip(tr("Change the current contour direction"));

    actFirstCtr = new QAction(tr("First"), this);
    actFirstCtr->setStatusTip(tr("Set the selected contour as the first contour"));

    actUpCtr = new QAction(tr("Up"), this);
    actUpCtr->setStatusTip(tr("Move up the selected contour"));

    actDownCtr = new QAction(tr("Down"), this);
    actDownCtr->setStatusTip(tr("Move down the selected contour"));

    actLastCtr = new QAction(tr("Last"), this);
    actLastCtr->setStatusTip(tr("Set the selected contour as the last contour"));

    actDeleteCtr = new QAction(tr("Delete"), this);
    actDeleteCtr->setStatusTip(tr("Delete current contour"));

    viewContours->setContextMenuPolicy(Qt::ContextMenuPolicy::ActionsContextMenu);
    viewContours->addAction(actPropCtr);
    viewContours->addAction(actSortCtr);
    viewContours->addAction(actChangeDirCtr);

    viewContours->addAction(actFirstCtr);
    viewContours->addAction(actUpCtr);
    viewContours->addAction(actDownCtr);
    viewContours->addAction(actLastCtr);
    viewContours->addAction(actDeleteCtr);

    actPropSeg = new QAction(tr("Properties"), this);
    actPropSeg->setStatusTip(tr("Current segment properties"));
    actPropSeg->setFont(act_bold_font);

    actEntryLineSeg = new QAction(tr("Use as Entry line"), this);
    actEntryLineSeg->setStatusTip(tr("Use selected segment as the Entry line for a simple contour"));

    actFirstSeg = new QAction(tr("First"), this);
    actFirstSeg->setStatusTip(tr("Set the selected segment as the first of the contour"));

    actUpSeg = new QAction(tr("Up"), this);
    actUpSeg->setStatusTip(tr("Move up the selected segment"));

    actDownSeg = new QAction(tr("Down"), this);
    actDownSeg->setStatusTip(tr("Move down the selected segment"));

    actLastSeg = new QAction(tr("Last"), this);
    actLastSeg->setStatusTip(tr("Set the selected segment as the last of the contour"));

    actDeleleSeg = new QAction(tr("Delete"), this);
    actDeleleSeg->setStatusTip(tr("Delete current segment"));

    viewSegments->setContextMenuPolicy(Qt::ContextMenuPolicy::ActionsContextMenu);
    viewSegments->addAction(actPropSeg);
    viewSegments->addAction(actEntryLineSeg);
    viewSegments->addAction(actFirstSeg);
    viewSegments->addAction(actUpSeg);
    viewSegments->addAction(actDownSeg);
    viewSegments->addAction(actLastSeg);
    viewSegments->addAction(actDeleleSeg);

    groupContours = new QGroupBox(tr("Contours"));
    groupContour = new QGroupBox(tr("Current"));

    groupContours->setLayout(new QVBoxLayout());
    groupContours->layout()->addWidget(lineContour);
    groupContours->layout()->addWidget(viewContours);

    groupContour->setLayout(new QVBoxLayout());
    groupContour->layout()->addWidget(lineSegment);
    groupContour->layout()->addWidget(viewSegments);

    createViewControl();

    gridView = new QGridLayout;
    gridView->addWidget(groupContours, 0, 0);
    gridView->addWidget(groupContour, 0, 1, 1, 2);
    gridView->addLayout(vboxLeft, 1, 0, Qt::AlignLeft | Qt::AlignTop);
    gridView->addLayout(vboxRight_0, 1, 1, Qt::AlignLeft | Qt::AlignTop);
    gridView->addLayout(vboxRight_1, 1, 2, Qt::AlignLeft | Qt::AlignTop);

    gridView->setColumnStretch(0, 1);
    gridView->setColumnStretch(1, 1);
    gridView->setColumnStretch(2, 1);

    viewContours->show();
    viewSegments->show();

    connect(viewContours, &QTableView::clicked, this, &FormContour::onViewContoursClicked);
    connect(viewContours, &QTableView::activated, this, &FormContour::onViewContoursClicked);
    connect(viewContours, &QTableView::entered, this, &FormContour::onViewContoursClicked);
    connect(viewContours, &QTableView::pressed, this, &FormContour::onViewContoursClicked);

    connect(viewSegments, &QTableView::clicked, this, &FormContour::onViewSegmentsClicked); // one mouse click
    connect(viewSegments, &QTableView::activated, this, &FormContour::onViewSegmentsClicked); // two mouse click or Enter
    connect(viewSegments, &QTableView::entered, this, &FormContour::onViewSegmentsClicked);
    connect(viewSegments, &QTableView::pressed, this, &FormContour::onViewSegmentsClicked);

    actions = {
        actPropCtr, actSortCtr, actChangeDirCtr, actFirstCtr, actUpCtr, actDownCtr, actLastCtr, actDeleteCtr,
        actPropSeg, actEntryLineSeg, actFirstSeg, actUpSeg, actDownSeg, actLastSeg, actDeleleSeg
    };
}

void FormContour::createViewControl() {
    btnNew = new QPushButton(tr("New"));
    btnNew->setStatusTip(tr("Add a new empty contour"));

    btnNewEntryLine = new QPushButton(tr("New Entry"));
    btnNewEntryLine->setStatusTip(tr("Add a new entry line before the first segment of the first contour"));

    btnDelete = new QPushButton(tr("Delete"));
    btnDelete->setStatusTip(tr("Delete selected contour"));

    btnClear = new QPushButton(tr("Clear"));
    btnClear->setStatusTip(tr("Clear all contours"));

    vboxLeft = new QVBoxLayout;
    vboxLeft->addWidget(btnNew);
    vboxLeft->addWidget(btnNewEntryLine);
    vboxLeft->addWidget(btnDelete);
    vboxLeft->addWidget(btnClear);
    vboxLeft->setSizeConstraint(QLayout::SetFixedSize);

    btnFirst = new QPushButton(tr("First"));
    btnFirst->setStatusTip(tr("Set the selected contour or segment as the first in the list"));

    btnUp = new QPushButton(tr("Up"));
    btnUp->setStatusTip(tr("Move up the selected contour or segment"));

    btnDown = new QPushButton(tr("Down"));
    btnDown->setStatusTip(tr("Move down the selected contour or segment"));

    btnLast = new QPushButton(tr("Last"));
    btnLast->setStatusTip(tr("Set the selected contour or segment as the last in the list"));

    btnSort = new QPushButton(tr("Sort"));
    btnSort->setStatusTip(tr("Auto sort segments in the current contour"));

    //
    actEntryLine = new QAction(tr("Use as Entry line"));
    actEntryLine->setStatusTip(tr("Use selected segment as the Entry line of a single contour"));

//    actBridgeLine = new QAction(tr("Use as Bridge line"));
//    actBridgeLine->setStatusTip(tr("Use selected segment as the Bridge line"));

//    actExitLine = new QAction(tr("Use as Exit line"));
//    actExitLine->setStatusTip(tr("Use selected segment as the Exit line"));

//    menuUseAs = new QMenu;
//    menuUseAs->addAction(actEntryLine);
//    menuUseAs->addAction(actBridgeLine);
//    menuUseAs->addAction(actExitLine);

//    btnUseAs->setMenu(menuUseAs);

    btnProperties = new QPushButton(tr("Properties"));
    btnProperties->setStatusTip(tr("Current contour/segment properties"));

    btnUndo = new QPushButton(tr("Undo"));
    btnUndo->setStatusTip(tr("Undo the last change"));
    btnUndo->setEnabled(false);

    actChangeDir = new QAction(tr("Change Direction"));
    actChangeDir->setStatusTip(tr("Change direction of the current contour"));

    actRotate = new QAction(tr("Rotate"));
    actRotate->setStatusTip(tr("Rotate all contours in the project by a given angle"));

    actFlipLeftRight = new QAction(tr("Flip Left-Right"));
    actFlipLeftRight->setStatusTip(tr("Flip all contours in the project Left-Right"));

    actFlipUpDown = new QAction(tr("Flip Upside-Down"));
    actFlipUpDown->setStatusTip(tr("Flip all contours in the project Upside-Down"));

    actResize = new QAction(tr("Resize"));
    actResize->setStatusTip(tr("Resize all contours in the project"));

    menuEdit = new QMenu;
    menuEdit->addAction(actEntryLine);
    menuEdit->addAction(actChangeDir);
    menuEdit->addAction(actRotate);
    menuEdit->addAction(actFlipLeftRight);
    menuEdit->addAction(actFlipUpDown);
    menuEdit->addAction(actResize);

    btnEdit = new QPushButton(tr("Edit"));
    btnEdit->setMenu(menuEdit);

    vboxRight_0 = new QVBoxLayout;
    vboxRight_0->addWidget(btnFirst);
    vboxRight_0->addWidget(btnUp);
    vboxRight_0->addWidget(btnDown);
    vboxRight_0->addWidget(btnLast);
    vboxRight_0->addWidget(btnSort);

    vboxRight_1 = new QVBoxLayout;
    vboxRight_1->addWidget(btnProperties);
    vboxRight_1->addWidget(btnEdit);
    vboxRight_1->addWidget(btnUndo);

    connect(btnNew, &QPushButton::clicked, this, &FormContour::on_btnNewContour_clicked);
    connect(btnNewEntryLine, &QPushButton::clicked, this, &FormContour::on_btnEntryCutline_clicked);

    connect(btnDelete, &QPushButton::clicked, this, [&](bool /*checked*/) {
        switch (m_viewState) {
        case VIEW_STATE::TABLE_VIEW_CONTOURS: on_actDeleteCtr_clicked(); break;
        case VIEW_STATE::TABLE_VIEW_SEGMENTS: on_actDeleteSeg_clicked(); break;
        default: break;
        }
    });

    connect(btnClear, &QPushButton::clicked, this, &FormContour::on_btnClear_clicked);

    connect(btnFirst, &QPushButton::clicked, this, [&](bool /*checked*/) {
        switch (m_viewState) {
            case VIEW_STATE::TABLE_VIEW_CONTOURS: on_actFirstCtr_clicked(); break;
            case VIEW_STATE::TABLE_VIEW_SEGMENTS: on_actFirstSeg_clicked(); break;
            default: break;
        }
    });

    connect(btnLast, &QPushButton::clicked, this, [&](bool /*checked*/) {
        switch (m_viewState) {
            case VIEW_STATE::TABLE_VIEW_CONTOURS: on_actLastCtr_clicked(); break;
            case VIEW_STATE::TABLE_VIEW_SEGMENTS: on_actLastSeg_clicked(); break;
            default: break;
        }
    });

    connect(btnUp, &QPushButton::clicked, this, [&](bool /*checked*/) {
        switch (m_viewState) {
            case VIEW_STATE::TABLE_VIEW_CONTOURS: on_actUpCtr_clicked(); break;
            case VIEW_STATE::TABLE_VIEW_SEGMENTS: on_actUpSeg_clicked(); break;
            default: break;
        }
    });

    connect(btnDown, &QPushButton::clicked, this, [&](bool /*checked*/) {
        switch (m_viewState) {
            case VIEW_STATE::TABLE_VIEW_CONTOURS: on_actDownCtr_clicked(); break;
            case VIEW_STATE::TABLE_VIEW_SEGMENTS: on_actDownSeg_clicked(); break;
            default: break;
        }
    });

    connect(btnSort, &QPushButton::clicked, this, &FormContour::on_btnSortClicked);

    connect(btnProperties, &QPushButton::clicked, this, [&](bool /*checked*/) {
        switch (m_viewState) {
            case VIEW_STATE::TABLE_VIEW_CONTOURS: on_btnCtrProp_clicked(); break;
            case VIEW_STATE::TABLE_VIEW_SEGMENTS: on_btnSegProp_clicked(); break;
            default: break;
        }
    });

    connect(actPropCtr, &QAction::triggered, this, &FormContour::on_btnCtrProp_clicked);
    connect(actPropSeg, &QAction::triggered, this, &FormContour::on_btnSegProp_clicked);

    connect(actDeleteCtr, &QAction::triggered, this, &FormContour::on_actDeleteCtr_clicked);
    connect(actDeleleSeg, &QAction::triggered, this, &FormContour::on_actDeleteSeg_clicked);

    connect(actSortCtr, &QAction::triggered, this, &FormContour::on_btnSortClicked);

    connect(actChangeDir, &QAction::triggered, this, &FormContour::on_btnChangeDir_clicked);
    connect(actChangeDirCtr, &QAction::triggered, this, &FormContour::on_btnChangeDir_clicked);

    connect(actEntryLine, &QAction::triggered, this, &FormContour::on_actEntryLine_clicked);
    connect(actEntryLineSeg, &QAction::triggered, this, &FormContour::on_actEntryLine_clicked);

    connect(actFirstCtr, &QAction::triggered, this, &FormContour::on_actFirstCtr_clicked);
    connect(actUpCtr, &QAction::triggered, this, &FormContour::on_actUpCtr_clicked);
    connect(actDownCtr, &QAction::triggered, this, &FormContour::on_actDownCtr_clicked);
    connect(actLastCtr, &QAction::triggered, this, &FormContour::on_actLastCtr_clicked);

    connect(actFirstSeg, &QAction::triggered, this, &FormContour::on_actFirstSeg_clicked);
    connect(actUpSeg, &QAction::triggered, this, &FormContour::on_actUpSeg_clicked);
    connect(actDownSeg, &QAction::triggered, this, &FormContour::on_actDownSeg_clicked);
    connect(actLastSeg, &QAction::triggered, this, &FormContour::on_actLastSeg_clicked);

    connect(actRotate, &QAction::triggered, this, &FormContour::on_actRotate_triggered);
    connect(actFlipLeftRight, &QAction::triggered, this, &FormContour::on_actFlipLeftRight_triggered);
    connect(actFlipUpDown, &QAction::triggered, this, &FormContour::on_actFlipUpDown_triggered);
    connect(actResize, &QAction::triggered, this, &FormContour::on_actResize_triggered);

    connect(btnUndo, &QPushButton::clicked, this, [&](bool /*checked*/) {
        par.contours.undo();
        btnUndo->setEnabled(false);
        restoreViewPos(0,0,0);
    });
}

FormContour::~FormContour() {}

void FormContour::setFontPointSize(QWidget* const w, int pointSize) {
    if (w) {
        QFont font = w->font();
        font.setPointSize(pointSize);
        w->setFont(font);
    }
}

void FormContour::setFontPointSize(QAction* const act, int pointSize) {
    if (act) {
        QFont font = act->font();
        font.setPointSize(pointSize);
        act->setFont(font);
    }
}

void FormContour::setFontPointSize(int pointSize) {
    for (QPushButton* b: buttons)
        setFontPointSize(b, pointSize);

    setFontPointSize(groupContours, pointSize);
    setFontPointSize(groupContour, pointSize);

    for (QAction* act: actions)
        setFontPointSize(act, pointSize);

    m_fontSize = pointSize;
}

QString FormContour::getFileName() const{
    return file_open ? par.dxfFileName : "";
}

void FormContour::resetFileOpen() { file_open = false; }

void FormContour::init() {
    m_ctr_num = m_row_num = m_col_num = 0;
    setViewState(VIEW_STATE::TABLE_VIEW_UNDEFINED);
    _init();
}

void FormContour::_init() {
//    if (m_contour_num >= par.contours.count())
//        m_contour_num = 0;

//    ContourPair* pair = par.contours.at(m_contour_num);

//    if (pair) {
//        if (m_row >= pair->bot()->count())
//            m_row = 0;

//        if (m_column > 1)
//            m_column = 0;
//    }
//    else
//        m_row = m_column = 0;

    plot();
    initView();

    btnUndo->setEnabled( par.contours.hasUndo() );

    if (par.contours.empty()) {
        viewContours->setContextMenuPolicy(Qt::ContextMenuPolicy::PreventContextMenu);
        viewSegments->setContextMenuPolicy(Qt::ContextMenuPolicy::PreventContextMenu);
    }
    else {
        viewContours->setContextMenuPolicy(Qt::ContextMenuPolicy::ActionsContextMenu);
        viewSegments->setContextMenuPolicy(Qt::ContextMenuPolicy::ActionsContextMenu);
    }
}

void FormContour::on_btnLoadDxf_clicked() {
    QFile file;
    file_open = false;
    emit fileNameChanged("");

    QDir dir(par.dxfDir);

    if (!dir.exists()) {
        par.dxfDir = QDir::homePath() + par.dxfDirDefault;
        dir = QDir(par.dxfDir);

        if (!dir.exists()) {
            bool OK = dir.mkpath(".");

            if (!OK)
                par.dxfDir.clear();
        }
    }

    QString selectedFilter = tr("DXF files") + " (*.dxf *.DXF)";
    QString new_filePath = QFileDialog::getOpenFileName(this, tr("Open DXF file"),
                                par.dxfDir + "/" + par.dxfFileName,
                                selectedFilter + ";;" + tr("All files") + " (*)",
                                &selectedFilter
                            );

    if (new_filePath.length()) {
        QFileInfo fInfo(new_filePath);
        par.dxfDir = fInfo.dir().path();
        par.dxfFileName = fInfo.fileName();

        QFile file(new_filePath);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            QMessageBox::critical(this, tr("Contour: Open DXF file error"), file.errorString());
        else {
            Dxf dxf, bot, top;
            dxf.init(par.dxfDir.toStdString(), par.dxfFileName.toStdString());

            if (dxf.parse()) {
                txtMsg->setText(tr("Number of loaded layers") + ": " + QString::number(dxf.numberLayers()) + ". " + tr("Entities") + ": " + QString::number(dxf.count()) + ".");

                bool layersSelected = false;

                if (dxf.numberLayers() > 0) {
                    txtMsg->setText(txtMsg->toPlainText() + " " + tr("Layers' names") + ":");

                    const set<string>& layers = dxf.getLayers();
                    for (auto& it: layers) {
                        txtMsg->setText(txtMsg->toPlainText() + " " + QString( it.c_str() ));
                    }

                    txtMsg->setText(txtMsg->toPlainText() + "\n");

                    if (layers.size() == 1) {
                        layersSelected = true;
                        bot = dxf;
                    }
                    else {
                        // Dialog
                        LayerDialog* dialog = new LayerDialog(layers, this);

                        if (!dialog)
                            return;

                        dialog->setFontPointSize(14);
                        dialog->exec();

                        layersSelected = dialog->result() == QDialog::Accepted;
                        if (layersSelected) {
                            bot = dxf.getLayer(dialog->xyLayerName());
                            top = dxf.getLayer(dialog->uvLayerName());
                        }

                        delete dialog;
                    }
                }
                else if (!dxf.empty()) { // impossible case
                    txtMsg->setText(txtMsg->toPlainText() + " " + tr("Layers has no names") + "\n");
                    layersSelected = true;
                    bot = dxf;
                }

                if (layersSelected) {
                    file_open = true;
                    emit fileNameChanged(par.dxfFileName);
                }

                dxf = bot;

                Dxf free, unused;
                bool OK = dxf.sort(free, unused);
                // OK = false - sort manually

                if (!free.empty()) {
                    txtMsg->setText(txtMsg->toPlainText() + tr("Removed unconnected segments from DXF") + ":\n");

                    for (list<DxfEntity*>::const_iterator it = free.entities().cbegin(); it != free.entities().cend(); ++it)
                        if (*it)
                            txtMsg->setText(txtMsg->toPlainText() + (*it)->toString().c_str() + "\n");
                        else
                            txtMsg->setText(txtMsg->toPlainText() + "Error at void FormContour::on_btnBot_clicked() (1): DxfEntity is NULL\n");
                }

                if (!unused.empty()) {
                    txtMsg->setText(txtMsg->toPlainText() + tr("Removed extra tails from DXF") + ":\n");

                    for (list<DxfEntity*>::const_iterator it = unused.entities().cbegin(); it != unused.entities().cend(); ++it)
                        if (*it)
                            txtMsg->setText(txtMsg->toPlainText() + (*it)->toString().c_str() + "\n");
                        else
                            txtMsg->setText(txtMsg->toPlainText() + "Error at void FormContour::on_btnBot_clicked() (2): DxfEntity is NULL\n");
                }

                if (!OK)
                    txtMsg->setText(txtMsg->toPlainText() + tr("Bottom layer DXF segments sorting error") + ": " + dxf.lastError().c_str() + "\n");

                if (OK && free.empty() && unused.empty()) {
                    bot = dxf;
                }

                par.contours.new_back();
                par.contours.back()->setBot(bot);

                // todo: check top
                par.contours.back()->setTop(top);

                init();
                clearUndo();
            }
            else {
                txtMsg->setText(tr("Bottom layer DXF parsing error") + ": " + QString(bot.lastError().c_str()));
                plot(bot);
            }

            par.saveSettings();
        }
    }
}

void FormContour::on_btnNewContour_clicked() {
    updateCurrentViewPos();

    par.contours.new_back();
//    initView();

    restoreViewPos(par.contours.size() - 1, 0, 0);
}

void FormContour::on_btnEntryCutline_clicked() {
    updateCurrentViewPos();

    if (!par.contours.empty()) {
        const ContourPair* const pair = par.contours.at(m_ctr_num);
        const fpoint_valid_t entry_pt = pair && !pair->empty() && pair->bot() ? pair->bot()->first_point() : fpoint_valid_t(false);

        if (pair && pair->type() == CONTOUR_TYPE::MAIN_CONTOUR && entry_pt.valid) {
            bool insert_before = m_ctr_num == 0 || par.contours.at(m_ctr_num - 1)->type() == CONTOUR_TYPE::MAIN_CONTOUR;

            const fpoint_valid_t exit_pt = pair && pair->bot() ? pair->bot()->last_point() : fpoint_valid_t(false);
            bool insert_after = !insert_before && exit_pt.valid;

            if (insert_before || insert_after) {
                NewCutlineDialog* dialog = new NewCutlineDialog(this);

                if (!dialog)
                    return;

                if (insert_before) {
                    dialog->set(0, entry_pt.x, 0, entry_pt.y);
                    dialog->enable(0x5);
                }
                else if (insert_after) {
                    dialog->set(exit_pt.x, 0, exit_pt.y, 0);
                    dialog->enable(0xA);
                }

                dialog->exec();

                if (dialog->result() == QDialog::Accepted) {
                    double x0, x1, y0, y1;
                    dialog->get(x0, x1, y0, y1);

                    DxfLine line;
                    if (dialog->isRel()) {
                        if (insert_after) {
                            line.setX0(entry_pt.x);
                            line.setX1(entry_pt.x + x1);
                            line.setY0(entry_pt.y);
                            line.setY1(entry_pt.y + y1);
                        }
                        else { // before
                            line.setX0(entry_pt.x + x0);
                            line.setX1(entry_pt.x);
                            line.setY0(entry_pt.y + y0);
                            line.setY1(entry_pt.y);
                        }
                    }
                    else {
                        line.setX0(x0);
                        line.setX1(x1);
                        line.setY0(y0);
                        line.setY1(y1);
                    }

                    qDebug() << "Dialog OK";

                    ContourPair incut(CONTOUR_TYPE::CUTLINE_CONTOUR);
                    if (incut.bot())
                        incut.bot()->push_back(line);

                    if (insert_before) {
                        ContourPair outcut(incut);
                        outcut.reverse();

                        par.contours.push_front(incut);
                        par.contours.push_back(outcut);

                        m_row_num = 0;
                        m_col_num = 0;
                        par.contours.clearSelected();
                    }
                    else {
                        par.contours.insert_after(m_ctr_num, incut);
                        m_ctr_num++;
                    }

                    restoreViewPos(m_ctr_num, 0, 0);
                }

                delete dialog;
            }
        }
    }
}

void FormContour::on_actDeleteCtr_clicked() {
    updateCurrentViewPos();

    if (m_ctr_num < par.contours.size()) {
        m_viewState = VIEW_STATE::TABLE_VIEW_CONTOURS;

        saveUndo();
        par.contours.remove(m_ctr_num);

        if (m_ctr_num >= par.contours.size())
            m_ctr_num = par.contours.size() - 1;

        restoreViewPos(m_ctr_num, 0, 0);
    }
}

void FormContour::on_actDeleteSeg_clicked() {
    updateCurrentViewPos();
    ContourPair* const pair = par.contours.at(m_ctr_num);

    if (pair && !pair->empty()) {
        m_viewState = VIEW_STATE::TABLE_VIEW_SEGMENTS;

        saveUndo();
        pair->remove(m_row_num);

        if (m_row_num >= pair->count())
            m_row_num = pair->count() - 1;

        restoreViewPos(m_ctr_num, m_row_num, m_col_num);
    }
}

void FormContour::on_btnClear_clicked() {
    qDebug() << "Clear all contours:";
    qDebug() << par.contours.toString().c_str();

    saveUndo();

    setEmptyModel();
    par.contours.clear();

    qDebug() << par.contours.toString().c_str();

    init();
}

void FormContour::on_btnChangeDir_clicked() {
    updateCurrentViewPos();
    ContourPair* const pair = par.contours.at(m_ctr_num);

    if (!pair || pair->empty())
        return;

    saveUndo();

    pair->reverse();

    restoreViewPos(m_ctr_num, pair->count() - 1 - m_row_num, m_col_num);
}

void FormContour::on_actFirstCtr_clicked() {
    updateCurrentViewPos();

    if (par.contours.size() > 1 && m_ctr_num != 0 && m_ctr_num < par.contours.size()) {
        saveUndo();

        par.contours.shiftFirst(m_ctr_num);
        restoreViewPos(0);
    }
}

void FormContour::on_actLastCtr_clicked() {
    updateCurrentViewPos();

    if (par.contours.size() > 1 && m_ctr_num < par.contours.size() - 1) {
        saveUndo();

        par.contours.shiftLast(m_ctr_num);
        restoreViewPos(par.contours.size() - 1);
    }
}

void FormContour::on_actUpCtr_clicked() {
    updateCurrentViewPos();

    if (par.contours.size() > 1 && m_ctr_num != 0 && m_ctr_num < par.contours.size()) {
        saveUndo();

        par.contours.moveUp(m_ctr_num);

        if (m_ctr_num > 0)
            m_ctr_num--;

        restoreViewPos(m_ctr_num);
    }
}

void FormContour::on_actDownCtr_clicked() {
    updateCurrentViewPos();

    if (par.contours.size() > 1 && m_ctr_num < par.contours.size() - 1) {
        saveUndo();

        par.contours.moveDown(m_ctr_num);

        if (m_ctr_num < par.contours.size() - 1)
            m_ctr_num++;
        else
            m_ctr_num = par.contours.size() - 1;

        restoreViewPos(m_ctr_num);
    }
}

void FormContour::on_actFirstSeg_clicked() {
    updateCurrentViewPos();
    ContourPair* const pair = par.contours.at(m_ctr_num);

    if (!pair || pair->empty())
        return;

    saveUndo();

    qDebug() << pair->toStringShort().c_str();
    pair->setFirst(m_col_num, m_row_num);
    qDebug() << pair->toStringShort().c_str();

    restoreViewPos(m_ctr_num, 0, m_col_num);
}

void FormContour::on_actLastSeg_clicked() {
    updateCurrentViewPos();
    ContourPair* const pair = par.contours.at(m_ctr_num);

    if (!pair || pair->empty())
        return;

    saveUndo();

    qDebug() << pair->toStringShort().c_str();
    pair->setLast(m_col_num, m_row_num);
    qDebug() << pair->toStringShort().c_str();

    restoreViewPos(m_ctr_num, pair->count() - 1, m_col_num);
}

void FormContour::on_actUpSeg_clicked() {
    updateCurrentViewPos();
    ContourPair* const pair = par.contours.at(m_ctr_num);

    if (!pair || pair->empty())
        return;

    saveUndo();

    qDebug() << pair->toStringShort().c_str();
    pair->moveUp(m_col_num, m_row_num);
    qDebug() << pair->toStringShort().c_str();

    restoreViewPos(m_ctr_num, m_row_num > 0 ? --m_row_num : 0, m_col_num);
}

void FormContour::on_actDownSeg_clicked() {
    updateCurrentViewPos();
    ContourPair* const pair = par.contours.at(m_ctr_num);

    if (!pair || pair->empty())
        return;

    saveUndo();

    qDebug() << pair->toStringShort().c_str();
    pair->moveDown(m_col_num, m_row_num);
    qDebug() << pair->toStringShort().c_str();

    if (m_row_num < pair->bot()->count() - 1)
        m_row_num++;

    restoreViewPos(m_ctr_num, m_row_num, m_col_num);
}

void FormContour::on_btnSortClicked() {
    updateCurrentViewPos();
    ContourPair* const pair = par.contours.at(m_ctr_num);

    if (!pair || pair->empty())
        return;

    fpoint_valid_t prev_pt = fpoint_t(false), next_pt = fpoint_t(false);

    const ContourPair* const prev = par.contours.at(m_ctr_num - 1);
    const ContourPair* const next = par.contours.at(m_ctr_num + 1);

    if (prev) {
        switch (m_col_num) {
        case 0:
            prev_pt = prev->lastBot();
            break;
        case 1:
            prev_pt = prev->lastTop();
            break;
        default:
            break;
        }
    }

    if (next) {
        switch (m_col_num) {
        case 0:
            next_pt = next->firstBot();
            break;
        case 1:
            next_pt = next->firstTop();
            break;
        default:
            break;
        }
    }

    Dxf dxf, free, unused;

    switch (m_col_num) {
    case 0:
        dxf = *pair->bot();
        break;
    case 1:
        dxf = *pair->top();
        break;
    default:
        return;
    }

    if (dxf.empty())
        return;

    bool OK = dxf.sort(free, unused, prev_pt, next_pt);

    if (!free.empty()) {
       txtMsg->setText(txtMsg->toPlainText() + tr("Removed unconnected segments from DXF") + ":\n");

       for (list<DxfEntity*>::const_iterator it = free.entities().cbegin(); it != free.entities().cend(); ++it)
           if (*it)
               txtMsg->setText(txtMsg->toPlainText() + (*it)->toString().c_str() + "\n");
           else
               txtMsg->setText(txtMsg->toPlainText() + "Error at void FormContour::on_btnBot_clicked() (1): DxfEntity is NULL\n");
    }

    if (!unused.empty()) {
       txtMsg->setText(txtMsg->toPlainText() + tr("Removed extra tails from DXF") + ":\n");

       for (list<DxfEntity*>::const_iterator it = unused.entities().cbegin(); it != unused.entities().cend(); ++it)
           if (*it)
               txtMsg->setText(txtMsg->toPlainText() + (*it)->toString().c_str() + "\n");
           else
               txtMsg->setText(txtMsg->toPlainText() + "Error at void FormContour::on_btnBot_clicked() (2): DxfEntity is NULL\n");
    }

    if (!OK)
       txtMsg->setText(txtMsg->toPlainText() + tr("Bottom layer DXF segments sorting error") + ": " + dxf.lastError().c_str() + "\n");

    OK &= free.empty() && unused.empty();

    if (OK) {
        switch (m_col_num) {
        case 0:
            saveUndo();
            pair->setBot(dxf);
            break;
        case 1:
            saveUndo();
            pair->setTop(dxf);
            break;
        default:
            return;
        }

        restoreViewPos(m_ctr_num, 0, m_col_num);
    }
}

void FormContour::onViewContoursClicked(const QModelIndex &index) {
    setViewState(VIEW_STATE::TABLE_VIEW_CONTOURS);

    if (index.isValid()) {
        updateCurrentViewPos();
        initSegmentsView();

        const ContourPair* const pair = par.contours.at(m_ctr_num);

        if (pair && !pair->empty()) {
            if (pair->type() == CONTOUR_TYPE::MAIN_CONTOUR)
                lineContour->setText(QString("%1 Contour (%2)").arg(m_ctr_num + 1).arg(pair->count()));
            else
                lineContour->setText(QString("%1 Cutline (%2)").arg(m_ctr_num + 1).arg(pair->count()));

            par.contours.select(m_ctr_num);
            plot();
        }
        else
            lineContour->clear();

        txtMsg->setText(par.contours.toString().c_str());
    }
}

void FormContour::onViewSegmentsClicked(const QModelIndex& index) {
    setViewState(VIEW_STATE::TABLE_VIEW_SEGMENTS);

    if (index.isValid()) {
        updateCurrentViewPos();
        const ContourPair* const pair = par.contours.at(m_ctr_num);

        const Dxf* const bot = pair && !pair->botEmpty() ? pair->bot() : nullptr;
        const Dxf* const top = pair && !pair->topEmpty() ? pair->top() : nullptr;

        switch (m_col_num) {
        case 0:
            if (bot && m_row_num < bot->count()) {
                lineSegment->setText( QString("%1 %2 XY").arg(m_row_num + 1).arg( QString::fromStdString(bot->at(m_row_num)->typeString()) ) );
                txtMsg->setText( bot->at(m_row_num)->toString().c_str() );
                par.contours.select(m_ctr_num, m_row_num, m_col_num);
                plot();
            }

            break;
        case 1:
            if (top && m_row_num < top->count()) {
                lineSegment->setText(QString("%1 %2 UV").arg(m_row_num + 1).arg( QString::fromStdString(top->at(m_row_num)->typeString()) ) );
                txtMsg->setText( top->at(m_row_num)->toString().c_str() );
                par.contours.select(m_ctr_num, m_row_num, m_col_num);
                plot();
            }
            break;
        }
    }
}

// only for single contour
void FormContour::on_actEntryLine_clicked() {
    updateCurrentViewPos();

    if (par.contours.size() == 1) {
        ContourPair incut(CONTOUR_TYPE::CUTLINE_CONTOUR);        
        const ContourPair* const pair = par.contours.front();

        if (pair && !pair->empty()) {
            saveUndo();

            if (!pair->botEmpty() && pair->bot()->isSorted() && !pair->bot()->isLoop()) {
                if (m_row_num == 0)
                    incut.setBot( Dxf(pair->bot()->cut_front()) );
                else if (m_row_num == pair->countBot() - 1) {
                    incut.setBot( Dxf(pair->bot()->cut_back()) );
                    incut.bot()->reverse();
                }
            }

            if (!pair->topEmpty() && pair->top()->isSorted() && !pair->top()->isLoop()) {
                if (m_row_num == 0)
                    incut.setTop( Dxf(pair->top()->cut_front()) );
                else if (m_row_num == pair->countTop() - 1) {
                    incut.setTop( Dxf(pair->top()->cut_back()) );
                    incut.top()->reverse();
                }
            }

            if (pair->bot()->isLoop() && !incut.botEmpty()) {
                ContourPair outcut(incut);
                outcut.reverse();

                par.contours.push_front(incut);
                par.contours.push_back(outcut);

                restoreViewPos(1, 0, 0);
                return;
            }

            init();
        }
    }
    else {
        txtMsg->setText("It works only for a single loop contour");

//        ContourPair cutline(CONTOUR_TYPE::CUTLINE_CONTOUR);
////        const ContourPair* const pair = par.contours.front();
//        ContourPair* pair = par.contours.at(m_contour_num); // todo: check m_contour_num

//        if (pair && !pair->empty()) {
//            if (!pair->emptyBot()) {
//                if (m_row == 0)
//                    cutline.setBot( Dxf(pair->bot()->cut_front()) );
//                else if (m_row == pair->countBot() - 1) {
//                    cutline.setBot( Dxf(pair->bot()->cut_back()) );
//                    cutline.bot()->reverse();
//                }
//            }

//            if (!pair->emptyTop()) {
//                if (m_row == 0)
//                    cutline.setTop( Dxf(pair->top()->cut_front()) );
//                else if (m_row == pair->countTop() - 1) {
//                    cutline.setTop( Dxf(pair->top()->cut_back()) );
//                    cutline.top()->reverse();
//                }
//            }

//            if (!cutline.emptyBot() || !cutline.emptyTop()) {
//                ContourPair outcut(cutline);
//                outcut.reverse();

////                par.contours.insert(m_contour_num, incut);
//                par.contours.push_front(cutline);
//                par.contours.push_back(outcut);

//                m_row = 0;
//                m_column = 0;
//                par.contours.clearSelected();
//                init();
//            }
//        }
    }
}

void FormContour::restoreViewPos(size_t ctr_num, size_t row, size_t column) {    
    bool OK = restoreViewPos(ctr_num);

    if (OK) {
        const ContourPair* const pair = par.contours.at(ctr_num);

        if (!pair->empty() && row < pair->count() && column < 2) {
            QModelIndex seg_idx = viewSegments->model()->index(row, column);
            viewSegments->setCurrentIndex(seg_idx);

            onViewSegmentsClicked(seg_idx);
        }
    }
}

bool FormContour::restoreViewPos(size_t ctr_num) {
    par.contours.clearSelected();
    _init();

    QItemSelectionModel* m = viewSegments->selectionModel();
    viewSegments->setModel(new ContourTableModel(par.contours.at(ctr_num)));
    delete m;

    viewSegments->resizeColumnsToContents();
    lineSegment->clear();

    if (!par.contours.empty() && ctr_num < par.contours.size()) {
        QModelIndex ctr_idx = viewContours->model()->index(ctr_num, 0);
        viewContours->setCurrentIndex(ctr_idx);

        onViewContoursClicked(ctr_idx);

        return true;
    }

    return false;
}

void FormContour::saveUndo() {
    if (par.contours.saveUndo())
        btnUndo->setEnabled(true);
}

void FormContour::clearUndo() {
    par.contours.clearUndo();
    btnUndo->setEnabled(false);
}

void FormContour::setViewState(VIEW_STATE state) {
    m_viewState = state <= VIEW_STATE::TABLE_VIEW_SEGMENTS ? state : VIEW_STATE::TABLE_VIEW_UNDEFINED;

    if (state == VIEW_STATE::TABLE_VIEW_SEGMENTS)
        btnDelete->setStatusTip(tr("Delete selected segment"));
    else
        btnDelete->setStatusTip(tr("Delete selected contour"));
}

void FormContour::on_btnSegProp_clicked() {
    QStringList list;

    updateCurrentViewPos();
    const ContourPair* const pair = par.contours.at(m_ctr_num);

    if (!pair || pair->empty() || m_col_num >= 2 || (m_col_num == 0 && m_row_num >= pair->countBot()) || (m_col_num == 1 && m_row_num >= pair->countTop()))
        return;

    bool first  = m_ctr_num == 0 && pair->count() > 1 &&
            ((m_col_num == 0 && m_row_num == 0) || (m_col_num == 1 && m_row_num == 0));

    bool last   = m_ctr_num == par.contours.size() - 1 && pair->count() > 1 &&
            ((m_col_num == 0 && m_row_num == pair->countBot() - 1) || (m_col_num == 1 && m_row_num == pair->countTop() - 1));

    int i = 1;
    for (const ContourPair& ctr: par.contours.contours())
        list.append(QString::number(i++) + " " + QString::fromStdString(ctr.toStringShort()));

    SegPropertiesDialog* dialog = new SegPropertiesDialog(
                list,
                m_col_num ? pair->top()->at(m_row_num) : pair->bot()->at(m_row_num),
                m_ctr_num, m_row_num, m_col_num,
                first, last,
                this
            );

    if (!dialog)
        return;

    dialog->setFontPointSize(14);
    dialog->exec();

    if (dialog->result() == QDialog::Accepted) {
        if (dialog->isNewContour() && (dialog->isNewBefore() || dialog->isNewAfter())) {
            ContourPair ctr(CONTOUR_TYPE::CUTLINE_CONTOUR);
            ContourPair* const pair = par.contours.at(m_ctr_num);

            if (pair && !pair->empty()) {
                saveUndo();

                ctr.setBot( Dxf(pair->bot()->cut_at(m_row_num)) );

                if (dialog->isNewBefore()) {
                    par.contours.push_front(ctr);
                    m_ctr_num++;
                }
                else {
                    par.contours.push_back(ctr);                    
                }

                if (m_row_num >= pair->count())
                    m_row_num = pair->count() > 0 ? pair->count() - 1 : 0;

                restoreViewPos(m_ctr_num, m_row_num, 0);
                viewSegments->setFocus();
                setViewState(VIEW_STATE::TABLE_VIEW_SEGMENTS);
            }
        }
        else {
            // move to exist contour
            int i = dialog->contourNumber();

            if (i >= 0 && size_t(i) != m_ctr_num) { // move to the other contour
                ContourPair* const pair_out = par.contours.at(m_ctr_num);
                ContourPair* const pair_in = par.contours.at(i);

                if (pair_out && pair_in && !pair_out->empty() && m_row_num < pair_out->countBot()) {
                    saveUndo();

                    Dxf bot = pair_out->bot()->cut_at(m_row_num);
                    Dxf top = pair_out->top()->cut_at(m_row_num);
                    pair_in->move_back(bot, top);

                    restoreViewPos(m_ctr_num, m_row_num, 0);
                    viewSegments->setFocus();
                    setViewState(VIEW_STATE::TABLE_VIEW_SEGMENTS);
                }
            }
        }
    }

    delete dialog;
}

void FormContour::on_btnCtrProp_clicked() {
    updateCurrentViewPos();
    ContourPair* const pair = par.contours.at(m_ctr_num);

    if (!pair || pair->empty())
        return;

    ContourPropDialog* dialog = new ContourPropDialog(pair, m_ctr_num, this);

    if (!dialog)
        return;

    dialog->setFontPointSize(14);
    dialog->exec();

    if (dialog->result() == QDialog::Accepted) {
        saveUndo();

        if (dialog->isCutline())
            pair->setType(CONTOUR_TYPE::CUTLINE_CONTOUR);
        else
            pair->setType(CONTOUR_TYPE::MAIN_CONTOUR);

        restoreViewPos(m_ctr_num, m_row_num, m_col_num);
    }

    delete dialog;
}

void FormContour::on_actExitPoint_triggered() {

}

void FormContour::on_actRotate_triggered() {
    if (par.contours.empty())
        return;

    RotateDialog* dialog = new RotateDialog(this);

    if (!dialog)
        return;

    updateCurrentViewPos();

    dialog->exec();

    if (dialog->result() == QDialog::Accepted) {
        double angle = dialog->value();

        int data = ProgramParam::swapXY << 2 | ProgramParam::reverseX << 1 | ProgramParam::reverseY;

        if (data == 0b010 || data == 0b001 || data == 0b100 || data == 0b111)
            angle = -angle;

        saveUndo();

        qDebug() << "Dialog OK" << "Rotate: " << angle;
        par.contours.rotate(M_PI / 180 * angle);

        restoreViewPos(m_ctr_num, m_row_num, m_col_num);
    }

    delete dialog;
}

void FormContour::on_actFlipLeftRight_triggered() {
    if (par.contours.empty())
        return;

    updateCurrentViewPos();
    saveUndo();

    ProgramParam::swapXY ? par.contours.flipY() : par.contours.flipX();

    restoreViewPos(m_ctr_num, m_row_num, m_col_num);
}

void FormContour::on_actFlipUpDown_triggered() {
    if (par.contours.empty())
        return;

    updateCurrentViewPos();
    saveUndo();

    ProgramParam::swapXY ? par.contours.flipX() : par.contours.flipY();

    restoreViewPos(m_ctr_num, m_row_num, m_col_num);
}

void FormContour::on_plotClicked(const QPointF& pt) {
    size_t ctr_num = 0, row_num = 0, col_num = 0;

    bool valid = par.contours.find(fpoint_t(pt.x(), pt.y()), m_plotView.range(), ctr_num, row_num, col_num);

    if (valid) {
        qDebug("Clicked %s [%d, %d]", col_num == 0 ? "BOT" : "TOP", (int)ctr_num, (int)row_num);

        restoreViewPos(ctr_num, row_num, col_num);
        viewSegments->setFocus();
        setViewState(VIEW_STATE::TABLE_VIEW_SEGMENTS);
    }
}

//void FormContour::on_plotClicked(size_t ctr_num, size_t row_num, size_t col_num) {
//    restoreViewPos(ctr_num, row_num, col_num);
//    viewSegments->setFocus();
//    m_viewState = TABLE_VIEW_SEGMENTS;
//}

void FormContour::on_actResize_triggered() {
    if (par.contours.empty())
        return;

    updateCurrentViewPos();
//    ContourPair* const pair = par.contours.at(m_ctr_num); // ??

//    if (!pair || pair->empty()) // ??
//        return;

    ContourRange range = par.contours.range();
    center = fpoint_valid_t(range.center(), center.valid);

    ResizeDialog* dialog = new ResizeDialog(range.width(), range.height(), center, this);

    if (!dialog)
        return;

    dialog->exec();

    if (dialog->result() == QDialog::Accepted) {
        saveUndo();

        double pct = 1;
        if (dialog->hasPct()) {
            pct = dialog->pct() / 100.0;
            qDebug() << "Dialog OK" << "Rotate pct: " << pct;
        }
        else if (dialog->hasRect()) {
            qDebug() << "Dialog OK" << "Resize Width: " << dialog->rectWidth() << "Height: " << dialog->rectHeight();
            frect_t rect(fpoint_t(range.x_min, range.y_min), fpoint_t(range.x_max, range.y_max));
            pct = rect.resize(dialog->rectWidth(), dialog->rectHeight());
        }
        else if (dialog->hasRatio()) {
            pct = dialog->ratioNewSize() / dialog->ratioBaseSize();
            qDebug() << "Dialog OK" << "Ratio Old: " << dialog->ratioBaseSize() << "New: " << dialog->ratioNewSize() << "(" << pct << ")";
        }
        else
            qDebug() << "Dialog Error";

        center = dialog->center();
//        fpoint_t base(center.valid ? center.x : range.x_min, center.valid ? center.y : range.y_min);
        fpoint_t base(center.valid ? center.x : par.contours.firstBot().x, center.valid ? center.y : par.contours.firstBot().y);

        par.contours.scale(pct, base);

//        pair->scale(pct, base);

//        if (m_ctr_num) {
//            ContourPair* const incut = par.contours.at(m_ctr_num - 1);

//            if (incut && incut->type() == CONTOUR_TYPE::CUTLINE_CONTOUR) {
//                if (!incut->botEmpty())
//                    incut->bot()->back()->change_1( pair->firstBot() );

//                if (!incut->topEmpty())
//                    incut->top()->back()->change_1( pair->firstTop() );
//            }
//        }

//        if ((m_ctr_num + 1) < par.contours.size()) {
//            ContourPair* const outcut = par.contours.at(m_ctr_num + 1);

//            if (outcut && outcut->type() == CONTOUR_TYPE::CUTLINE_CONTOUR) {
//                if (!outcut->botEmpty())
//                    outcut->bot()->front()->change_0( pair->lastBot() );

//                if (!outcut->topEmpty())
//                    outcut->top()->front()->change_0( pair->lastTop() );
//            }
//        }

        restoreViewPos(m_ctr_num, m_row_num, m_col_num);
    }

    delete dialog;
}

void FormContour::initView() {
    qDebug() << "Init. " << "Contours: " << par.contours.size();

    QItemSelectionModel* m = viewContours->selectionModel();
    viewContours->setModel(new ContoursModel(&par.contours));
    delete m;

    viewContours->resizeColumnsToContents();
    lineContour->clear();

    initSegmentsView();
}

void FormContour::setEmptyModel() {
    QItemSelectionModel* m = viewSegments->selectionModel();
    viewSegments->setModel(new ContourTableModel());
    delete m;
}

void FormContour::initSegmentsView() {
    updateCurrentViewPos();

    qDebug() << "Init. " << "Contours: " << par.contours.size();

    QItemSelectionModel* m = viewSegments->selectionModel();
    viewSegments->setModel(new ContourTableModel(par.contours.at(m_ctr_num)));
    delete m;

    viewSegments->resizeColumnsToContents();
    lineSegment->clear();
}

void FormContour::plot() {
//    plotView.plot(par.contours, plotViewSize());
    m_plotView.setSwapXY(par.swapXY);
    m_plotView.setInverseX(par.reverseX);
    m_plotView.setInverseY(par.reverseY);
    m_plotView.setShowXY(par.showXY);
    m_plotView.plot(par.contours);

    // first point
    if (!par.contours.empty() && !par.contours.front()->botEmpty()) {
        fpoint_t pt = par.contours.front()->firstBot();
        m_plotView.addSymbolSquare(&pt, Qt::GlobalColor::red);
    }
}

void FormContour::plot(const Dxf& contour) {
    m_plotView.setSwapXY(par.swapXY);
    m_plotView.setInverseX(par.reverseX);
    m_plotView.setInverseY(par.reverseY);
    m_plotView.setShowXY(par.showXY);
    m_plotView.plot(contour);
}

QSize FormContour::plotViewSize() const {
    return plotViewSize(size());
}

QSize FormContour::plotViewSize(const QSize& formSize) const {
    QSize btnSize = gridButtons->sizeHint();
    QSize txtSize = txtMsg->size();

    int w = int(formSize.width() * m_plot_view_pct);
    int h = formSize.height() - btnSize.height() - txtSize.height();

    qDebug() << "Widget size: " << formSize << ", plot width:" << w << ", msg height: " << txtSize.height() << ", buttons height: " << btnSize.height();

    return QSize(w, h);
}

void FormContour::updateCurrentViewPos() {
    m_ctr_num = viewContours->currentIndex().row() < 0 ? 0 : viewContours->currentIndex().row();

    if (m_ctr_num >= par.contours.size())
        m_ctr_num = par.contours.size() - 1;

    m_row_num = viewSegments->currentIndex().row() < 0 ? 0 : viewSegments->currentIndex().row();

    const ContourPair* const pair = par.contours.at(m_ctr_num);
    size_t row_count = pair ? pair->count() : 0;

    if (m_row_num >= row_count)
        m_row_num = row_count ? row_count - 1 : 0;

    m_col_num = viewSegments->currentIndex().column() < 0 ? 0 : (viewSegments->currentIndex().column() & 1); // 0 or 1
}

void FormContour::resizeEvent(QResizeEvent* event) {
    if (event && !m_plotView.onResizeEvent( plotViewSize(event->size()) ))
        event->ignore();
//    updateGeometry();
//    update();
}

void FormContour::on_btnOpen_clicked() {
    emit fileNameChanged("");
    QDir dir(par.projDir);

    if (!dir.exists()) {
        par.projDir = QDir::homePath() + par.projDirDefault;
        dir = QDir(par.projDir);

        if (!dir.exists()) {
            bool OK = dir.mkpath(".");

            if (!OK)
                par.projDir.clear();
        }
    }

    QString selectedFilter = tr("Project files") + " (*.ncproj *.NCPROJ)";
    QString new_filePath = QFileDialog::getOpenFileName(this, tr("Open project"),
                                par.projDir + "/" + par.projFileName,
                                selectedFilter + ";;" + tr("All files") + " (*)",
                                &selectedFilter
                            );

    if (new_filePath.length()) {
        QFileInfo fInfo(new_filePath);
        par.projDir = fInfo.dir().path();
        par.projFileName = fInfo.fileName();

        QFile file(par.projDir + "/" + par.projFileName);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            QMessageBox::critical(this, tr("Contour: Open project file error"), file.errorString());
        else {
            QXmlStreamReader xml;
            xml.setDevice(&file);
            bool err = false;

            while (xml.tokenType() != QXmlStreamReader::StartDocument) {
                err = xml.atEnd() || xml.hasError();
                if (err) break;
                xml.readNext();
            }

            deque<ContourPair> contours = xml_ext::readContours(xml);
            par.contours = contours;

            // todo: add cut settings into the project
//            if (!err)
//                err = !par.cut.read(xml);

//            if (!err) {
//                GeneratorModeList list(par.genModesList);
//                err = !list.read(xml);
//                if (!err) {
////                    QItemSelectionModel* m = tableMode->selectionModel();

//                    par.genModesList = list.get();

////                    tableMode->setModel(new ModeTableModel(par.genModesList, this));
////                    delete m;
////                    tableMode->resizeColumnsToContents();
////                    initComboMode();
////                    bindModesFile();
//                }
//            }

            if (xml.hasError()) {
                QMessageBox::critical(this, tr("Contour: Parse project file error"), xml.errorString(), QMessageBox::Ok);
                return;
            }

            file.close();
            init();
            clearUndo();
            emit fileNameChanged(par.projFileName);
        }
    }
}

void FormContour::on_btnSave_clicked() {
    if (!par.projDir.length() || !par.projFileName.length())
        on_btnSaveAs_clicked();
    else {
        QFile file(par.projDir + "/" + par.projFileName);

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            QMessageBox::critical(this, tr("Contour: Save project file error"), file.errorString());
        else {
            QXmlStreamWriter xml(&file);
            xml.setAutoFormatting(true);
            xml.writeStartDocument();
                xml.writeStartElement("project");
                    xml_ext::writeContours(xml, par.contours.contours());
//                    par.cut.write(xml);
//                    GeneratorModeList(par.genModesList).write(xml);
                xml.writeEndElement();
            xml.writeEndDocument();

            file.close();
            emit fileNameChanged(par.projFileName);
        }
    }
}

void FormContour::on_btnSaveAs_clicked() {
    QDir dir(par.projDir);

    if (!dir.exists()) {
        par.projDir = QDir::homePath() + par.projDirDefault;
        dir = QDir(par.projDir);

        if (!dir.exists()) {
            bool OK = dir.mkpath(".");

            if (!OK)
                par.projDir.clear();
        }
    }

    QString new_filePath = QFileDialog::getSaveFileName(this, tr("Save project"),
                               par.projDir + "/" + par.projFileName,
                               tr("Project files") + " (*.ncproj *.NCPROJ)");

    if (new_filePath.length() != 0) {
        QFileInfo fInfo(new_filePath);
        par.projDir = fInfo.dir().path();
        par.projFileName = fInfo.completeBaseName() + ".ncproj";
        on_btnSave_clicked();
    }
}

void FormContour::setText(const QString& text) {
    txtMsg->setText(text);
}
