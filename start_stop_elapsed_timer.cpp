#include "start_stop_elapsed_timer.h"

StartStopElapsedTimer::StartStopElapsedTimer() : ms(0) {
    timer = new QElapsedTimer;
}

StartStopElapsedTimer::~StartStopElapsedTimer() {
    if (timer) {
        delete timer;
        timer = nullptr;
    }
    ms = 0;
}

void StartStopElapsedTimer::clear() {
    if (timer) timer->invalidate();
    ms = 0;
}

void StartStopElapsedTimer::start() {
    if (!timer->isValid())
        timer->start();
}

void StartStopElapsedTimer::stop() {
    if (timer && timer->isValid()) {
        ms += timer->elapsed();
        timer->invalidate();
    }
}

qint64 StartStopElapsedTimer::elapsed() {
    return timer && timer->isValid() ? ms + timer->elapsed() : ms;
}
