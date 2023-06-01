#include "dxf_circle.h"
#include <cmath>

using namespace std;

DxfCircle::DxfCircle() : DxfArc(DxfArc()) {}
DxfCircle::DxfCircle(const fpoint_t& C, double R, double alpha, bool ccw) : DxfArc(C, R, alpha, alpha, ccw) {}
DxfCircle::DxfCircle(const DxfCircle& other) : DxfArc(other) {}
DxfCircle::~DxfCircle() {}

DxfEntity* DxfCircle::clone() const { return new DxfCircle(*this); }

void DxfCircle::setStartAngle(double degree) { m_beta = m_alpha = range360(degree); }

bool DxfCircle::isPoint() const { return m_R < M_PRECISION; }
bool DxfCircle::isCircle() const { return true; }

vector<fpoint_t> DxfCircle::getPoints() const {
    vector<fpoint_t> res;

    for (double a = 0; a < 2 * M_PI; a += ANGLE_STEP) {
        fpoint_t pt;
        pt.x = m_C.x + m_R * cos(a);
        pt.y = m_C.y + m_R * sin(a);
        res.push_back(pt);
    }

    return res;
}

void DxfCircle::set(const fpoint_t& C, double R, double angle, bool ccw) {
    m_flags.valid = 1;
    m_flags.circle = 1;
    m_flags.ccw = ccw;    
    this->m_R = fabs(R);
    m_alpha = m_beta = range360(angle);
    this->m_C = C;
}

void DxfCircle::set(const fpoint_t& A, const fpoint_t& C, bool ccw) {
    m_flags.valid = 1;

    if (A == C) { // point
        m_flags.circle = 0;
        m_flags.ccw = ccw;
        m_R = 0;
        m_alpha = 0;
        m_beta = 0;
        this->m_C = C;
    }
    else { // circle
        m_flags.circle = 1;
        m_flags.ccw = ccw;
        polar(C, A, m_R, m_alpha);
        m_beta = m_alpha;
        this->m_C = C;
    }
}

double DxfCircle::length() const { return 2 * M_PI * m_R; }

void DxfCircle::offset(OFFSET_SIDE side, double offset, const DxfEntity* /*prev*/, const DxfEntity* /*next*/) {
    if ((m_flags.ccw && side == OFFSET_SIDE::RIGHT) || (!m_flags.ccw && side == OFFSET_SIDE::LEFT))
        m_R += offset;
    else {
        m_R -= offset;
        if (m_R < 0) m_R = 0;
    }
}

void DxfCircle::shift(const fpoint_t& value) { m_C.shift(value); }

void DxfCircle::reverse() { m_flags.ccw = !m_flags.ccw; }

string DxfCircle::toString() const {
    return m_flags.valid ? "Circle: Center " + m_C.toString() +
            ", Radius " + to_string(m_R) +
            ", Start Angle " + to_string(m_alpha / M_PI * 180) +
            ", CCW " + to_string(m_flags.ccw) : "Not valid";
}

string DxfCircle::toString2() const {
    return m_flags.valid ? "Circle: Center " + m_C.toString() +
            ", Radius " + to_string(m_R) +
            ", Start Angle " + to_string(m_alpha / M_PI * 180) +
            ", CCW " + to_string(m_flags.ccw) : "Not valid";
}

double DxfCircle::deltaAngle() const { return m_flags.ccw ? 2 * M_PI : -2 * M_PI; }

bool DxfCircle::check() {
    if (!m_flags.valid && m_flags.Cx && m_flags.Cy && m_flags.R) {
        m_flags.Cx = 0;
        m_flags.Cy = 0;
        m_flags.R = 0;        
        m_flags.valid = 1;
        m_flags.circle = 1;
    }
    return m_flags.valid;
}

void DxfCircle::clear() {
    DxfArc::clear();
    m_flags = {0,0,0,0,0,0,1,1};
    m_R = 0;
    m_alpha = 0;
    m_beta = 0;
    m_C = fpoint_t();
}
