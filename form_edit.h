#ifndef FORMEDIT_H
#define FORMEDIT_H

#include <QWidget>
#include <QPushButton>
#include <QGridLayout>
#include <QSplitter>
#include <QMenu>
#include <QAction>

#include "code_editor.h"
#include "program_param.h"
//#include "plot_view.h"
#include "qwt_plot_view.h"

class FormEdit : public QWidget {
    Q_OBJECT

    const QString help_file = "editor.html";

    ProgramParam& par;
    CodeEditor* txtEditor;
    QwtPlotView plotView;
    QTextEdit* txtMsg;

    bool select_ena, file_open;

    QPushButton *btnHome, *btnNew, *btnOpen, *btnSave, *btnSaveAs, *btn8, *btnPlot, *btnPlay, *btnContour, *btn9, *btn10, *btn11, *btnRun, *btnHelp;
    std::vector<QPushButton*> buttons;

    QGridLayout *gridButtons;

    QSplitter *splitterTxtPlot, *splitterTxtMsg;

    QVBoxLayout* mainLayout;

//    QSpacerItem *hSpacer, *vSpacer;

    void createButtons();

    GCode compile();
    void showMessage(QString s);

    void showPlot();
    void hidePlot();
    void showMessage();
    void hideMessage();

    void plot();

    QSize plotViewSize() const;

    void buttonsDisable(QPushButton* except);
    void buttonsEnable();

private slots:
    void on_btnNew_clicked();
    void on_btnOpen_clicked();
    void on_btnSave_clicked();
    void on_btnSaveAs_clicked();
    void on_btnPlot_clicked();
    void on_btnPlay_clicked();
    void on_btnMsg_clicked();
    void on_btnContour_clicked();
    void on_btnRun_clicked();
    void on_btnImit_clicked();

    void onTxtPlotMoved(int pos, int index);
    void onTxtMsgMoved(int pos, int index);

    void onLineChanged(int line_num);

public:
//    explicit FormEdit(QWidget *parent = nullptr);
    FormEdit(ProgramParam& par, QWidget *parent = nullptr);
    ~FormEdit();

    void setFontPointSize(int pointSize);
    QString getFileName() const;

signals:
    void homePageClicked();
    void runPageClicked();
    void testPageClicked();
    void contourPageClicked();
    void helpPageClicked(const QString& file_name);
    void fileNameChanged(const QString& file_name);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent* e) override;

public slots:
    void init(bool plot = false);
};

#endif // FORMEDIT_H
