#include "my_app.h"
#include "mainwindow.h"

#include <QDebug>
#include <QThread>
#include <QProcess>
#include <QTranslator>
#include <QMessageBox>
#include <QStyleFactory>

#include "program_param.h"

//#include <QDBusConnection>

int main(int argc, char *argv[]) {
    MyApp app(argc, argv);

    QTranslator translator;
    {
        ProgramParam::loadParam();

        switch (ProgramParam::lang) {
        case InterfaceLanguage::RUSSIAN:
            translator.load("icnc_ru", ".");
            break;
        default: break;
        }
    }
    qApp->installTranslator(&translator);
    qDebug("Hello world!");

    QIcon icon("icnc.png");
//    w.setWindowIcon(icon);
//    w.setWindowIconText(QObject::tr("Meatec CNC"));
    app.setWindowIcon(icon);
    app.setApplicationDisplayName(QObject::tr("Meatec CNC"));
//    app.setDesktopFileName(QObject::tr("Meatec CNC (Run)"));

//    int fontSize = ProgramParam::loadFontSize();

    qDebug()<<QStyleFactory::keys();
    qApp->setStyle(QStyleFactory::create("Fusion"));
//    qApp->setStyle(QStyleFactory::create("Windows"));
//    qApp->setStyle(QStyleFactory::create("windowsvista"));

#ifdef DARK_GUI_THEME
    QPalette darkPalette;
    QColor darkColor = QColor(45,45,45);
    QColor disabledColor = QColor(127,127,127);
    darkPalette.setColor(QPalette::Window, darkColor);
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(18,18,18));
    darkPalette.setColor(QPalette::AlternateBase, darkColor);
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
    darkPalette.setColor(QPalette::Button, darkColor);
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));

    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledColor);

    qApp->setPalette(darkPalette);

    qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
#endif

    MainWindow w;
    w.show();

//    QTimer::singleShot(0, &w, &MainWindow::onLoad);

    QThread::currentThread()->setPriority(QThread::Priority::HighPriority);

//    QDBusConnection bus = QDBusConnection::sessionBus();
//    if(bus.isConnected())
//    {
//        QString services[MAX_SERVICES] = {
//            "org.freedesktop.ScreenSaver",
//            "org.gnome.SessionManager"
//        };
//        QString paths[MAX_SERVICES] = {
//            "/org/freedesktop/ScreenSaver",
//            "/org/gnome/SessionManager"
//        };

//        for(int i = 0; i < MAX_SERVICES ; i++)
//        {
//            QDBusInterface screenSaverInterface(
//                services[i], paths[i],services[i], bus, this);
//            if (!screenSaverInterface.isValid())
//                continue;

//            QDBusReply<uint> reply = screenSaverInterface.call(
//                "Inhibit", "YOUR_APP_NAME", "REASON");
//            if (reply.isValid())
//            {
//                cookieID = reply.value();
//                qDebug()<<"succesful"
//            } else {
//                QDBusError error =reply.error();
//                qDebug()<<error.message()<<error.name();
//            }
//        }
//    }

    try { // I don't think that it works
        return app.exec();
    } catch(...) {
        QMessageBox::critical(nullptr, "Sorry! App crashed (1)", std::exception().what());
        return 0;
    }
}
