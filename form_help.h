#ifndef FORMHELP_H
#define FORMHELP_H

#include <QWidget>
#include <QTextBrowser>
#include <QPushButton>
#include <QGridLayout>

#include "program_param.h"

enum class HelpPageEnum: uint8_t { EMPTY, END };

class FormHelp : public QWidget
{
    Q_OBJECT

    QTextBrowser* browser;
    const ProgramParam& par;

    QVBoxLayout *mainLayout;
    QPushButton *btnBack, *btn1, *btn2, *btn3, *btn4, *btn5, *btn6, *btn7, *btn8, *btn9, *btn10, *btn11, *btn12, *btn13;
    QGridLayout* gridButtons;
    std::vector<QPushButton*> buttons;

    void createButtons();

public:
    explicit FormHelp(const ProgramParam& par, QWidget *parent = nullptr);
    void init(const QString& helpHtmlFile);

    void setFontPointSize(int pointSize);

signals:
    void backClicked();

};

#endif // FORMHELP_H
