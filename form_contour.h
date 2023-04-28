#ifndef FORM_DXF_H
#define FORM_DXF_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QGridLayout>
//#include <QSplitter>
#include <QSpacerItem>
#include <QTableView>
#include <QGroupBox>
#include <QMenu>
#include <QResizeEvent>

#include "program_param.h"
#include "contour_table_model.h"
#include "plot_view.h"
#include "qwt_plot_view.h"

class FormContour : public QWidget {
    Q_OBJECT

    const QString help_file = "contour.html";
    ProgramParam& par;

    QPushButton *btnHome, *btnOpen, *btnSave, *btnSaveAs, *btnLoadDxf, *btnMulti, *btn6, *btn7, *btn8, *btn9, *btn10, *btn11, *btnGenerate, *btnHelp;
    QGridLayout *gridButtons;
    QVBoxLayout *mainLayout;
//    QSplitter* splitter;

    QTextEdit* txtMsg {nullptr};

    QwtPlotView m_plotView {nullptr};
    double m_plot_view_pct {0};

    QGridLayout *gridPlot, *gridView;
    QLineEdit *lineContour, *lineSegment;

    QTableView *viewContours, *viewSegments;
    enum class VIEW_STATE : uint {TABLE_VIEW_UNDEFINED, TABLE_VIEW_CONTOURS, TABLE_VIEW_SEGMENTS} m_viewState = VIEW_STATE::TABLE_VIEW_UNDEFINED;

    QAction *actPropCtr {nullptr}, *actChangeDirCtr {nullptr}, *actDeleteCtr {nullptr},
        *actFirstCtr {nullptr}, *actUpCtr {nullptr}, *actDownCtr {nullptr}, *actLastCtr {nullptr},

        *actPropSeg {nullptr}, *actUseAsEntryLineSeg {nullptr},
        *actFirstSeg {nullptr}, *actUpSeg {nullptr}, *actDownSeg {nullptr}, *actLastSeg {nullptr}, *actSortCtr {nullptr},
        *actDeleleSeg {nullptr}, *actMoveSeg {nullptr};

    QGroupBox *groupContours, *groupContour;
    QPushButton *btnNewEmpty {nullptr}, *btnNewCutline {nullptr}, *btnMerge {nullptr}, *btnDelete {nullptr}, *btnClear {nullptr};

    QVBoxLayout *vboxLeft, *vboxRight_0, *vboxRight_1;

    QPushButton *btnFirst, *btnUp, *btnDown, *btnLast, *btnSort, *btnProperties, *btnEdit, *btnUndo;
    QMenu *menuUseAs, *menuEdit;
    QAction *actUseAsEntryLine, *actChangeDir, *actRotate, *actFlipLeftRight, *actFlipUpDown, *actResize;

    std::vector<QAction*> actions;

    size_t m_ctr_num, m_row_num, m_col_num;
    bool file_open;

    fpoint_valid_t center {false,0,0};

    std::vector<QPushButton*> buttons;
    int m_fontSize;

    void createButtons();
    void createGridView();
    void createViewControl();

    void initView();
    void setEmptyModel();
    void initSegmentsView();

    QSize plotViewSize() const;
    QSize plotViewSize(const QSize& newSize) const;

    void updateCurrentContourViewPos();
    void updateCurrentViewPos();
    void restoreViewPos(size_t ctr_num, size_t row, size_t column);
    bool restoreViewPos(size_t ctr_num);

    void saveUndo();
    void clearUndo();

    void setViewState(VIEW_STATE state);

signals:
    void homePageClicked();
    void muxPageClicked(size_t ctr);
    void passesPageClicked(bool one_pass = false);
    void editPageClicked(bool auto_plot);
    void helpPageClicked(const QString& file_name);
    void fileNameChanged(const QString& file_name);

private slots:
    void on_btnOpen_clicked();
    void on_btnSave_clicked();
    void on_btnSaveAs_clicked();

    void on_btnLoadDxf_clicked();

    void on_btnNewContour_clicked();
    void on_btnNewCutline_clicked();

    void on_btnMerge_clicked();

    void on_actDeleteCtr_clicked();
    void on_actDeleteSeg_clicked();

    void on_btnClear_clicked();

    void on_btnChangeDir_clicked();

    void on_actFirstCtr_clicked();
    void on_actLastCtr_clicked();
    void on_actUpCtr_clicked();
    void on_actDownCtr_clicked();

    void on_actFirstSeg_clicked();
    void on_actLastSeg_clicked();
    void on_actUpSeg_clicked();
    void on_actDownSeg_clicked();

    void on_btnSortClicked();

    void onViewContoursClicked(const QModelIndex&);
    void onViewSegmentsClicked(const QModelIndex&);

    void on_actUseAsEntryLine_clicked();
    void on_btnSegProp_clicked();
    void on_btnCtrProp_clicked();

    QModelIndexList selectedIndexes(bool sort_req = false, bool descent = false);
    void on_actMoveSeg_clicked();

    void on_actExitPoint_triggered();
    void on_actRotate_triggered();
    void on_actResize_triggered();
    void on_actFlipLeftRight_triggered();
    void on_actFlipUpDown_triggered();

    void on_plotClicked(const QPointF& pt);
    void on_plotControlClicked(const QPointF& pt);
//    void on_plotClicked(size_t ctr_num, size_t row_num, size_t col_num);

    static void setFontPointSize(QWidget* const w, int pointSize);
    static void setFontPointSize(QAction* const act, int pointSize);

protected:
    virtual void resizeEvent(QResizeEvent *event);

public:
//    explicit FormDxf(QWidget *parent = nullptr);
    FormContour(ProgramParam& par, QWidget *parent = nullptr);
    ~FormContour();

    void setFontPointSize(int pointSize);
    QString getFileName() const;
    void resetFileOpen();

private:
    void _init();
public:
    void init();


    void plot();
    void plot(const Dxf& contour);

    void setText(const QString& text);
};

#endif // FORM_DXF_H
