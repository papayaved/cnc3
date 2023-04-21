#include "form_center.h"
#include <QDebug>
#include <QTimer>

using namespace std;

FormCenter::FormCenter(ProgramParam& par, QWidget *parent) :
    QWidget(parent), par(par), cncReaderEna(false), adcEnable(false), wireSpeedMode(WireSpeed::Mode::MMM)
{
    this->setObjectName(tr("Centering Panel"));
    centerWidget = new CenterWidget(this);
    addButtons();

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(centerWidget);
//    mainLayout->addWidget(new QFrame);
    mainLayout->addLayout(gridButtons);

    this->setLayout(mainLayout);

    connect(centerWidget->btnResetPos, &QPushButton::clicked, this, [&]() {
        try {
            par.cnc.directSetPos(0, 0, 0, 0, 0, 0);
            centerWidget->emptyPlot();
        }
        catch (...) {}
    });

//    bindNumberButtons(numWidth, btnWidthInc, btnWidthDec);

//    connect(sender, &Sender::valueChanged,
//        [=]( const QString &newValue ) { receiver->updateValue( "senderValue", newValue ); }
//    );

    centerMode = CenterWidget::CenterMode::Mode1R;

    connect(centerWidget, &CenterWidget::modeChanged, this, [&](CenterWidget::CenterMode mode) {
        centerMode = mode;

        switch (centerMode) {
        case CenterWidget::CenterMode::Mode1R:
            btnXDec->setEnabled(true);
            btnXInc->setEnabled(true);
            btnYDec->setEnabled(true);
            btnYInc->setEnabled(true);
            btnCenter->setEnabled(false);
            break;
        case CenterWidget::CenterMode::Mode4R: case CenterWidget::CenterMode::Mode3Adj:
            btnXDec->setEnabled(false);
            btnXInc->setEnabled(false);
            btnYDec->setEnabled(false);
            btnYInc->setEnabled(false);
            btnCenter->setEnabled(true);
            break;
        default:
            btnXDec->setEnabled(false);
            btnXInc->setEnabled(false);
            btnYDec->setEnabled(false);
            btnYInc->setEnabled(false);
            btnCenter->setEnabled(false);
            break;
        }
    });

    centerWidget->radioMode1R->click();
}

FormCenter::~FormCenter() {
    stopCncReader();
    stopAdc();
}

void FormCenter::addButtons() {
    btnHome = new QPushButton(tr("Home"));
    btnHome->setStatusTip(tr("Go to the Home panel") + "   Alt+H");
    btnHome->setShortcut(QKeySequence("Alt+H"));

    btnXDec = new QPushButton(tr("X-"));
    btnXDec->setStatusTip(tr("Decrease X"));

    btnXInc = new QPushButton(tr("X+"));
    btnXInc->setStatusTip(tr("Increase X"));

    btnYDec = new QPushButton(tr("Y-"));
    btnYDec->setStatusTip(tr("Decrease Y"));

    btnYInc = new QPushButton(tr("Y+"));
    btnYInc->setStatusTip(tr("Increase Y"));

    btnCenter = new QPushButton(tr("Center"));
    btnCenter->setStatusTip(tr("Find a center"));

    btn6 = new QPushButton;

    btn7 = new QPushButton;

    btn8 = new QPushButton;

    btn9 = new QPushButton;

    btnWeakHV = new QPushButton(tr("Weak Voltage"));
    btnWeakHV->setCheckable(true);
    btnWeakHV->setStatusTip(tr("Enable weak high voltage"));

    btnHold = new QPushButton(tr("Hold"));
    btnHold->setCheckable(true);

    btnCancel = new QPushButton(tr("Cancel"));

    btnHelp = new QPushButton(tr("Help"));
    btnHelp->setStatusTip(tr("Open Help") + "   F1");
    btnHelp->setShortcut(QKeySequence::HelpContents);

    buttons = {btnHome, btnXDec, btnXInc, btnYDec, btnYInc, btnCenter, btn6, btn7, btn8,\
               btn9, btnWeakHV, btnHold, btnCancel, btnHelp};

    gridButtons = new QGridLayout;

    gridButtons->addWidget(btnHome, 0, 0);
    gridButtons->addWidget(btnXInc, 0, 1);
    gridButtons->addWidget(btnXDec, 0, 2);
    gridButtons->addWidget(btnYInc, 0, 3);
    gridButtons->addWidget(btnYDec, 0, 4);
    gridButtons->addWidget(btnCenter, 0, 5);
    gridButtons->addWidget(btn6, 0, 6);
    gridButtons->addWidget(btn7, 0, 7);
    gridButtons->addWidget(btn8, 0, 8);
    gridButtons->addWidget(btn9, 0, 9);
    gridButtons->addWidget(btnWeakHV, 0, 10);
    gridButtons->addWidget(btnHold, 0, 11);
    gridButtons->addWidget(btnCancel, 0, 12);
    gridButtons->addWidget(btnHelp, 0, 13);

    connect(btnHome, &QPushButton::clicked, this, &FormCenter::on_btnHome_clicked);

    connect(btnXDec, &QPushButton::clicked, this, [&]() { on_btnXYForward_clicked(-1, AXIS::AXIS_X); });
    connect(btnXInc, &QPushButton::clicked, this, [&]() { on_btnXYForward_clicked(1, AXIS::AXIS_X); });
    connect(btnYDec, &QPushButton::clicked, this, [&]() { on_btnXYForward_clicked(-1, AXIS::AXIS_Y); });
    connect(btnYInc, &QPushButton::clicked, this, [&]() { on_btnXYForward_clicked(1, AXIS::AXIS_Y); });
    connect(btnCenter, &QPushButton::clicked, this, &FormCenter::on_btnCenter_clicked);

    connect(btnCancel, &QPushButton::clicked, this, &FormCenter::on_btnCancel_clicked);

    connect(btnWeakHV, &QPushButton::clicked, this, [&]() {
        try {
            par.cnc.writeWeakHVEnable( btnWeakHV->isChecked() );
        }
        catch (...) {}
    });

    connect(btnHold, &QPushButton::clicked, this, [&]() {
        try {
            par.cnc.writeHoldEnable( btnHold->isChecked() );
        }
        catch (...) {}
    });

    connect(btnHelp, &QPushButton::clicked, this, [&]() { emit helpPageClicked(help_file); });
}

void FormCenter::init() {
    initButtons();
    startCncReader();
    startAdc();
}

void FormCenter::on_btnHome_clicked() {
    stopCncReader();
    stopAdc();
    emit homePageClicked();
}

void FormCenter::on_btnXYForward_clicked(int dir, AXIS axis) {
    centerWidget->emptyPlot();

    par.cnc.centering(
        axis == AXIS::AXIS_Y ? CENTER_MODE_T::CENTER_Y : CENTER_MODE_T::CENTER_X,
        centerWidget->touches(), 1,
        centerWidget->rollVel(), centerWidget->threshold(),
        centerWidget->fineSharePct(), dir < 0 ? -centerWidget->radius() : centerWidget->radius(),
        dir < 0 ? centerWidget->rollback() : -centerWidget->rollback(),
        centerWidget->speedCoarse(), centerWidget->speedFine(),
        0, 0, 0
    );
}

void FormCenter::on_btnCenter_clicked() {
    centerWidget->emptyPlot();

    par.cnc.centering(
        centerMode == CenterWidget::CenterMode::Mode3Adj ? CENTER_MODE_T::CENTER_CIRCLE_3ADJ : CENTER_MODE_T::CENTER_CIRCLE_4R,
        centerWidget->touches(), centerWidget->attempts(),
        centerWidget->rollVel(), centerWidget->threshold(),
        centerWidget->fineSharePct(), fabs(centerWidget->radius()),
        0,
        centerWidget->speedCoarse(), centerWidget->speedFine(),
        centerWidget->angle(0), centerWidget->angle(1), centerWidget->angle(2)
    );
}

void FormCenter::on_btnCancel_clicked() {
    par.cnc.cancelReq();
}

void FormCenter::startCncReader() {
    if (par.cnc.isOpen()) {
        qDebug()<<"CNC reader: Start";

        if (!cncReaderEna) {
            cncReaderEna = true;
            readCutState();
        }
    }
}

void FormCenter::stopCncReader() {
    qDebug()<<"CNC reader: Stop";
    cncReaderEna = false;
}

void FormCenter::readCutState() {
    static bool isWork_reg = false;
    static double dia = -1;

    if (cncReaderEna) {
//        qDebug()<<"CNC reader: Reading...";

        try {
            CncContext ctx = par.cnc.readCncContext();

            if (isWork_reg ^ ctx.isWork()) {
                isWork_reg = ctx.isWork();

                if (!isWork_reg)
                    dia = par.cnc.readDiameter();
                else
                    dia = -1;
            }

            centerWidget->txtMsg->setText(QString( ctx.toStringCenterDebug(dia).c_str() ));
//            qDebug() << "CNC reader: " + QString(cut_state.toString().c_str());


            centerWidget->setX(ctx.x());
            centerWidget->setY(ctx.y());

            double R = centerWidget->radius();
            ContourRange range(-R, R, -R, R);
            centerWidget->addToPlot(ctx.x(), ctx.y(), range);

            centerWidget->setSpeed(ctx.speed());

            centerWidget->setLimitSwitches(ctx.limitSwitches());

            btnWeakHV->blockSignals(true);
            btnHold->blockSignals(true);

            btnWeakHV->setChecked(ctx.weakHV());
            btnHold->setChecked(ctx.hold());

            btnWeakHV->blockSignals(false);
            btnHold->blockSignals(false);

            if (ctx.isWork()) {
//                btnHome->setEnabled(false);
                btnXDec->setEnabled(false);
                btnXInc->setEnabled(false);
                btnYDec->setEnabled(false);
                btnYInc->setEnabled(false);
                btnCenter->setEnabled(false);
                btnCancel->setEnabled(true);
                btnHelp->setEnabled(false);
            }
            else if (ctx.isError()) {
                par.cnc.cancelReq();
                initButtons();
            }
            else
                initButtons();

            if (btnWeakHV->isChecked() || ctx.isWork())
                btnHome->setEnabled(false);
            else
                btnHome->setEnabled(true);
        }
        catch (...) {
            initButtons();
        }

        QTimer::singleShot(POLLING_TIME, this, &FormCenter::readCutState);
    }
}

void FormCenter::initButtons() {
    bool line = centerWidget->radioMode1R->isChecked();
    bool center = centerWidget->radioMode4R->isChecked() || centerWidget->radioMode3Adj->isChecked();

    btnHome->setEnabled(line);
    btnXDec->setEnabled(line);
    btnXInc->setEnabled(line);
    btnYDec->setEnabled(line);
    btnYInc->setEnabled(line);
    btnCenter->setEnabled(center);
    btnCancel->setEnabled(false);
    btnHelp->setEnabled(true);    
}

void FormCenter::startAdc() { // on init
    if (par.cnc.isOpen()) {
        qDebug()<<"ADC: Start";

        if (!adcEnable) {
            adcEnable = true;
            readAdc();
        }
    }
    else
        adcEnable = false;
}

void FormCenter::stopAdc() { // on home
    qDebug()<<"ADC: Stop";
    adcEnable = false;
}

void FormCenter::readAdc() {
    if (adcEnable) {
        cnc_adc_volt_t adc = par.cnc.readADCVolt();
        centerWidget->setAdc(adc);
        QTimer::singleShot(ADC_POLLING_TIME, this, &FormCenter::readAdc);
    }
}

void FormCenter::setFontPointSize(int pointSize) {
    centerWidget->setFontPointSize(pointSize);

    for (QPushButton* b: buttons) {
        QFont font = b->font();
        font.setPointSize(pointSize);
        b->setFont(font);
//        b->setStyleSheet("font: bold");
    }
}
