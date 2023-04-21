#include "my_app.h"
#include <QMessageBox>
#include <iostream>

MyApp::MyApp(int &argc, char **argv) : QApplication(argc, argv) {}

bool MyApp::notify(QObject *rcv, QEvent *e) {
    try {
        return QApplication::notify(rcv, e);
    } catch(const std::exception& ex) {
        QMessageBox::critical(nullptr,
                              "The application is crashed (2)",
                              "Please remember your last action and sent its descriction and error message into Meatec company.\nError message: " + QString::number(e->type()) + ". " + ex.what()
        );

        std::cerr << "Event type: " + std::to_string(e->type()) + ". " + ex.what() << std::endl;
        assert( !"Forgot to catch exception" );
    } catch(...) {
        QMessageBox::critical(nullptr,
                              "The application is crashed (3)",
                              "Please remember your last action and sent its descriction and error message into Meatec company.\nError message: " + QString::number(e->type()) + ". " + std::exception().what()
        );

        std::cerr << "Event type: " + std::to_string(e->type()) + ". " + std::exception().what() << std::endl;
        assert( !"Forgot to catch exception" );
    }

    return false;
}
