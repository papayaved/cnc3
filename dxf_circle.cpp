#include "dxf_circle.h"
#include <cmath>

using namespace std;

DxfCircle::DxfCircle() : DxfArc(DxfArc()) {}
DxfCircle::DxfCircle(const fpoint_t& C, double R, double alpha, bool ccw) : DxfArc(C, R, alpha, alpha, ccw) {}
DxfCircle::DxfCircle(const DxfCircle& other) : DxfArc(other) {}
DxfCircle::~DxfCircle() {}

DxfEntity* DxfCircle::clone() const { return new DxfCircle(*this); }

void DxfCircle::setStartAngle(double degree) { beta = alpha = range360(degree); }

bool DxfCircle::isPoint() const { return R < M_PRECISION; }
bool DxfCircle::isCircle() const { return true; }

vector<fpoint_t> DxfCircle::getPoints() const {
    vector<fpoint_t> res;

    for (double a = 0; a < 2 * M_PI; a += ANGLE_STEP) {
        fpoint_t pt;
        pt.x = C.x + R * cos(a);
        pt.y = C.y + R * sin(a);
        res.push_back(pt);
    }

    return res;
}

void DxfCircle::set(const fpoint_t& C, double R, double angle, bool ccw) {
    flags.valid = 1;
    flags.circle = 1;
    flags.ccw = ccw;    
    this->R = fabs(R);
    alpha = beta = range360(angle);
    this->C = C;
}

void DxfCircle::set(const fpoint_t& A, const fpoint_t& C, bool ccw) {
    flags.valid = 1;

    if (A == C) { // point
        flags.circle = 0;
        flags.ccw = ccw;
        R = 0;
        alpha = 0;
        beta = 0;
        this->C = C;
    }
    else { // circle
        flags.circle = 1;
        flags.ccw = ccw;
        polar(C, A, R, alpha);
        beta = alpha;
        this->C = C;
    }
}

double DxfCircle::length() const { return 2 * M_PI * R; }

void DxfCircle::offset(OFFSET_SIDE side, double offset, const DxfEntity* /*prev*/, const DxfEntity* /*next*/) {
    if ((flags.ccw && side == OFFSET_SIDE::RIGHT) || (!flags.ccw && side == OFFSET_SIDE::LEFT))
        R += offset;
    else {
        R -= offset;
        if (R < 0) R = 0;
    }
}

void DxfCircle::shift(const fpoint_t& value) { C.shift(value); }

void DxfCircle::reverse() { flags.ccw = !flags.ccw; }

string DxfCircle::toString() const {
    return flags.valid ? "Circle: Center " + C.toString() +
            ", Radius " + to_string(R) +
            ", Start Angle " + to_string(alpha / M_PI * 180) +
            ", CCW " + to_string(flags.ccw) : "Not valid";
}

string DxfCircle::toString2() const {
    return flags.valid ? "Circle: Center " + C.toString() +
            ", Radius " + to_string(R) +
            ", Start Angle " + to_string(alpha / M_PI * 180) +
            ", CCW " + to_string(flags.ccw) : "Not valid";
}

double DxfCircle::deltaAngle() const { return flags.ccw ? 2 * M_PI : -2 * M_PI; }

bool DxfCircle::check() {
    if (!flags.valid && flags.Cx && flags.Cy && flags.R) {
        flags.Cx = 0;
        flags.Cy = 0;
        flags.R = 0;        
        flags.valid = 1;
        flags.circle = 1;
    }
    return flags.valid;
}

void DxfCircle::clear() {
    DxfArc::clear();
    flags = {0,0,0,0,0,0,1,1};
    R = 0;
    alpha = 0;
    beta = 0;
    C = fpoint_t();
}
