#ifndef MYAPP_H
#define MYAPP_H

#include <QApplication>

//#define NDEBUG
#include <assert.h>

class MyApp : public QApplication {
    Q_OBJECT

public:
    MyApp(int &argc, char **argv);

    virtual bool notify(QObject* rcv, QEvent* e) override;
};

#endif // MYAPP_H
