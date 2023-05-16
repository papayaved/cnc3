#include "cnc_param.h"
#include <cmath>

using namespace  std;

uint16_t CncParam::inputLevel = DEFAULT_INPUT_LEVEL;
bool CncParam::sdEnable = DEFAULT_SD_ENA;
bool CncParam::reverseX = false;
bool CncParam::reverseY = false;
bool CncParam::reverseU = false;
bool CncParam::reverseV = false;
bool CncParam::swapXY = false;
bool CncParam::swapUV = false;
bool CncParam::reverseEncX = false;
bool CncParam::reverseEncY = false;
double CncParam::step = DEFAULT_STEP; // mm
double CncParam::scaleX = DEFAULT_SCALE_XY, CncParam::scaleY = DEFAULT_SCALE_XY, CncParam::scaleU = DEFAULT_SCALE_UV, CncParam::scaleV = DEFAULT_SCALE_UV;
double CncParam::scaleEncX = DEFAULT_SCALE_ENC_XY, CncParam::scaleEncY = DEFAULT_SCALE_ENC_XY;
bool CncParam::encXY = false;

bool CncParam::fb_ena = false;
unsigned CncParam::rb_attempts = DEFAULT_RB_ATTEMPTS;
double CncParam::low_thld[2] = {0, 0}, CncParam::high_thld[2] = {200, 200}, CncParam::rb_to = DEFAULT_RB_TO, CncParam::rb_len = DEFAULT_RB_LEN, CncParam::rb_speed = DEFAULT_RB_SPEED;
double CncParam::fb_acc = DEFAULT_ACC, CncParam::fb_dec = DEFAULT_DEC;

bool CncParam::acc_ena = false;
double CncParam::acc = DEFAULT_ACC, CncParam::dec = DEFAULT_DEC;

// scale - steps/mm
int32_t CncParam::mm_to_steps(double mm, double scale) {
    double res = mm * scale;

    if (res > INT32_MAX)
        res = INT32_MAX;
    else if (res < INT32_MIN)
        res = INT32_MIN;

    return static_cast<int32_t>(round(res));
}

// scale - steps/mm
double CncParam::steps_to_mm(int32_t steps, double scale) {
    return steps / scale;
}

double CncParam::ums_to_mmm(double value) {
    return value * (60.0 / 1000.0);
}

double CncParam::scale(size_t i) {
    switch (i) {
    case 0: return scaleX;
    case 1: return scaleY;
    case 2: return scaleU;
    case 3: return scaleV;
    default: return CncParam::DEFAULT_SCALE_XY;
    }
}

double CncParam::scaleEncoder(size_t i) {
    switch (i) {
    case 0: return scaleEncX;
    case 1: return scaleEncY;
    default: return CncParam::DEFAULT_SCALE_ENC_XY;
    }
}

void CncParam::reset() {
    inputLevel = DEFAULT_INPUT_LEVEL;
    sdEnable = DEFAULT_SD_ENA;
    reverseX = reverseY = reverseU = reverseV = false;
    swapXY = swapUV = false;
    reverseEncX = reverseEncY = false;
    step = DEFAULT_STEP;
    scaleX =  scaleY = scaleU = scaleV = DEFAULT_SCALE_UV;
    scaleEncX = scaleEncY = DEFAULT_SCALE_ENC_XY;
    encXY = false;

    fb_ena = false;
    rb_attempts = DEFAULT_RB_ATTEMPTS;
    low_thld[0] = low_thld[1] = 0;
    high_thld[0] = high_thld[1] = 200;
    rb_to = DEFAULT_RB_TO;
    rb_len = DEFAULT_RB_LEN;
    rb_speed = DEFAULT_RB_SPEED;
    fb_acc = DEFAULT_ACC;
    fb_dec = DEFAULT_DEC;

    acc_ena = false;
    acc = DEFAULT_ACC;
    dec = DEFAULT_DEC;
}
