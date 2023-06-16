#include "segment_arc.h"
#include <cmath>

using namespace std;

const double SegmentArc::ANGLE_STEP = M_PI / 180;

SegmentArc::SegmentArc() :
    SegmentEntity(DXF_ENTITY_TYPE::ARC), m_flags({0,0,0,0,0,0,0,1}), m_R(0), m_alpha(0), m_beta(0), m_C(fpoint_t()) {}

SegmentArc::SegmentArc(const fpoint_t& C, double R, double alpha, double beta, bool ccw) :
    SegmentEntity(DXF_ENTITY_TYPE::ARC), m_flags({0,0,0,0,0,1,0,ccw}), m_R(R > 0 ? R : 0), m_alpha(range360(alpha)), m_beta(range360(beta)), m_C(C) {}

SegmentArc::SegmentArc(const fpoint_t &A, const fpoint_t &B, const fpoint_t &C, bool ccw) :
    SegmentEntity(DXF_ENTITY_TYPE::ARC), m_flags({0,0,0,0,0,1,0,1}), m_R(0), m_alpha(0), m_beta(0), m_C(fpoint_t())
{
    set(A, B, C, ccw);
}

SegmentArc::SegmentArc(const fpoint_t& A, const fpoint_t& B, double R, bool ccw) :
    SegmentEntity(DXF_ENTITY_TYPE::ARC), m_flags({0,0,0,0,0,1,0,1})
{
    set(A, B, R, ccw);
}

SegmentArc::SegmentArc(const SegmentArc& other) :
    SegmentEntity(other, DXF_ENTITY_TYPE::ARC),
    m_flags(other.m_flags),
    m_R(other.m_R),
    m_alpha(other.m_alpha),
    m_beta(other.m_beta),
    m_C(other.m_C)
{
}

SegmentArc::~SegmentArc() {}

SegmentEntity* SegmentArc::clone() const { return new SegmentArc(*this); }

bool SegmentArc::operator==(const SegmentEntity& other) const {
    return same(other) && point_0() == other.point_0() && point_1() == other.point_1();
}

bool SegmentArc::operator!=(const SegmentEntity& other) const {
    return !(*this == other);
}

bool SegmentArc::same(const SegmentEntity& other) const {
    return other.type() == DXF_ENTITY_TYPE::ARC && radius() > other.radius() - M_PRECISION && radius() < other.radius() + M_PRECISION;
}

bool SegmentArc::inside(const SegmentArc& other) const {
    double L = SegmentEntity::length(m_C, other.m_C);
    return L < fabs(other.radius() - radius());
}

bool SegmentArc::outside(const SegmentArc& other) const {
    double L = SegmentEntity::length(m_C, other.m_C);
    return L > other.radius() + radius();
}

bool SegmentArc::insideCircle(const fpoint_t& pt) const {
    return SegmentEntity::length(m_C, pt) <= radius();
}

bool SegmentArc::outsideCircle(const fpoint_t& pt) const { return !insideCircle(pt); }

bool SegmentArc::intersected(const SegmentArc& other) const {
    double L = SegmentEntity::length(m_C, other.m_C);
    return L >= fabs(other.radius() - radius()) && L <= other.radius() + radius();
}

void SegmentArc::setCenterX(double value) {
    m_flags.valid = 0;
    m_flags.Cx = 1;
    m_C.x = value;
}
void SegmentArc::setCenterY(double value) {
    m_flags.valid = 0;
    m_flags.Cy = 1;
    m_C.y = value;
}
void SegmentArc::setRadius(double value) {
    m_flags.valid = 0;
    m_flags.R = 1;
    m_R = value > 0 ? value : 0;
}

void SegmentArc::setStartAngle(double rad) {
    m_flags.valid = 0;
    m_flags.alpha = 1;
    m_alpha = range360(rad);
}
void SegmentArc::setEndAngle(double rad) {
    m_flags.valid = 0;
    m_flags.beta = 1;
    m_beta = range360(rad);
}

void SegmentArc::setExtrusionDirectionZ(double value) { m_flags.ccw = value >= 0 ? 1 : 0; }

bool SegmentArc::isPoint() const { return m_R < M_PRECISION; }

bool SegmentArc::isCircle() const { return point_0() == point_1() && m_R >= M_PRECISION; }

vector<fpoint_t> SegmentArc::getPoints() const {
    vector<fpoint_t> res;    

    double a = m_alpha;
    double R = radius();
    fpoint_t pt;

    res.push_back(point_0());

    if (m_flags.ccw) {
        if (a >= m_beta) a -= 2 * M_PI;
        a += ANGLE_STEP;

        for (; a < m_beta; a += ANGLE_STEP) {
            pt.x = m_C.x + R * cos(a);
            pt.y = m_C.y + R * sin(a);
            res.push_back(pt);
        }
    }
    else {
        if (a <= m_beta) a += 2 * M_PI;
        a -= ANGLE_STEP;

        for (; a > m_beta; a -= ANGLE_STEP) {
            pt.x = m_C.x + R * cos(a);
            pt.y = m_C.y + R * sin(a);
            res.push_back(pt);
        }
    }

    res.push_back(point_1());
    return res;
}

void SegmentArc::set(const fpoint_t& C, double R, double alpha, double beta, bool ccw) {
    m_flags = {0, 0, 0, 0, 0, 1, 0, ccw};

    this->m_R = R > 0 ? R :0;
    this->m_alpha = range360(alpha);
    this->m_beta = range360(beta);
    this->m_C = C;
}

void SegmentArc::set(const fpoint_t& A, const fpoint_t& B, const fpoint_t& C, bool ccw) {
    m_flags = {0, 0, 0, 0, 0, 1, 0, ccw};

    if (A == C && B == C) { // point
        m_flags.ccw = ccw;
        m_R = 0;
        m_alpha = 0;
        m_beta = 0;
        this->m_C = C;
    }
    else if (A == B) { // circle
        m_flags.circle = 1;
        m_flags.ccw = ccw;
        polar(C, A, m_R, m_alpha);
        m_beta = m_alpha;
        this->m_C = C;
    }
    else if (A == C || B == C) { // error, DxfLine
        polar(A, B, m_R, m_beta);
        m_R /= 2;
        this->m_C.x = (A.x + B.x) / 2;
        this->m_C.y = (A.y + B.y) / 2;
        m_alpha = range360(m_beta + M_PI);
        m_flags.ccw = true;
    }
    else {
        double R2;
        polar(C, A, m_R, m_alpha);
        polar(C, B, R2, m_beta);
        // todo: R
        this->m_C = C;
        m_flags.ccw  = ccw;
    }
}

void SegmentArc::set(const fpoint_t& A, const fpoint_t& B, double R_in, bool ccw) {
    m_flags = {0, 0, 0, 0, 0, 1, 0, ccw};

    bool biggest = R_in < 0;
    m_R = fabs(R_in);

    if (m_R == 0) { // point
        m_flags.ccw = ccw;
        m_alpha = 0;
        m_beta = 0;
        m_C = A;
        return;
    }

    double AB, a;
    polar(A, B, AB, a);

    fpoint_t D;
    D.x = (A.x + B.x) / 2;
    D.y = (A.y + B.y) / 2;

    double AB_2 = AB / 2;

    if (m_R < AB_2)
        m_R = AB_2;

    double L = sqrt(m_R * m_R - AB_2 * AB_2);
    double dx, dy;

    if (ccw) a += M_PI;

    dx = L * sin(a);
    dy = - L * cos(a);

    if (biggest) {
        dx = -dx;
        dy = -dy;
    }

    m_C.x = D.x + dx;
    m_C.y = D.y + dy;

    double gamma = asin(AB_2 / m_R); // [0, 90]

    double h = M_PI_2 + a;

    if (ccw) {
        m_alpha = h - gamma;
        m_beta = h + gamma;
    }
    else {
        m_alpha = h + gamma;
        m_beta = h - gamma;
    }

    m_alpha = range360(m_alpha);
    m_beta = range360(m_beta);

    m_flags.circle = A == B; // circle
}

double SegmentArc::point_X0() const { return m_C.x + radius() * cos(m_alpha); }
double SegmentArc::point_Y0() const { return m_C.y + radius() * sin(m_alpha); }
double SegmentArc::point_X1() const { return m_C.x + radius() * cos(m_beta); }
double SegmentArc::point_Y1() const { return m_C.y + radius() * sin(m_beta); }

fpoint_t SegmentArc::point_0() const { return fpoint_t(point_X0(), point_Y0()); }
fpoint_t SegmentArc::point_1() const { return fpoint_t(point_X1(), point_Y1()); }

double SegmentArc::radius() const { return m_R > 0 ? m_R : 0 ; }

double SegmentArc::tangent_0() const { return m_flags.ccw ? m_alpha + M_PI_2 : m_alpha - M_PI_2; }
double SegmentArc::tangent_1() const { return m_flags.ccw ? m_beta + M_PI_2 : m_beta - M_PI_2; }

ContourRange SegmentArc::range() const {
    ContourRange range;
    fpoint_t A  = point_0();
    fpoint_t B  = point_1();

    if (between(1.5 * M_PI, m_alpha, m_beta, m_flags.ccw))
        range.x_min = m_C.x - m_R;
    else
        range.x_min = B.x < A.x ? B.x : A.x;

    if (between(0.5 * M_PI, m_alpha, m_beta, m_flags.ccw))
        range.x_max = m_C.x + m_R;
    else
        range.x_max = B.x > A.x ? B.x : A.x;

    if (between(M_PI, m_alpha, m_beta, m_flags.ccw))
        range.y_min = m_C.y - m_R;
    else
        range.y_min = B.y < A.y ? B.y : A.y;

    if (between(0, m_alpha, m_beta, m_flags.ccw))
        range.y_max = m_C.y + m_R;
    else
        range.y_max = B.y > A.y ? B.y : A.y;

    range.valid = true;

    return range;
}

double SegmentArc::length() const { return fabs(deltaAngle()) * radius(); }

double SegmentArc::length(const fpoint_t& pt) const {
    double R, gamma;
    polar(m_C, pt, R, gamma);
    gamma = range360(gamma);

    double dGamma = deltaAngle(m_flags.ccw, m_alpha, gamma);
    dGamma = fabs(dGamma);

    double dArc = fabs(deltaAngle());

    if (dGamma >= dArc)
        return length();
    else
        return dGamma * radius();
}
double SegmentArc::distance(const fpoint_t& pt) const {
    double CP, gamma;
    polar(m_C, pt, CP, gamma);
    gamma = range360(gamma);

    double dGamma = deltaAngle(m_flags.ccw, m_alpha, gamma);

    if (dGamma <= 0)
        return SegmentEntity::length(point_0(), pt);
    else {
        double dArc = deltaAngle();

        if (dGamma >= dArc)
            return SegmentEntity::length(point_1(), pt);
        else
            return SegmentEntity::length(m_C, pt) - radius();
    }
}

SegmentEntity* SegmentArc::trim_front(double head, bool rem) {
    SegmentArc* res = nullptr;
    double len = length();

    if (head >= M_PRECISION && head < (len - M_PRECISION)) {
        double pct = head / len;
        double da = deltaAngle();
        double gamma = range360(m_alpha + da * pct);

        if (rem) res = new SegmentArc(m_C, m_R, m_alpha, gamma, m_flags.ccw);
        m_alpha = gamma;
    }
    return res;
}

SegmentEntity *SegmentArc::trim_front_rev(double tail, bool rem) {
    SegmentArc* res = nullptr;
    double len = length();

    if (tail >= M_PRECISION && tail < len) {
        double pct = tail / len;
        double da = deltaAngle();
        double gamma = range360(m_beta - da * pct);

        if (rem) res = new SegmentArc(m_C, m_R, m_alpha, gamma, m_flags.ccw);
        m_alpha = gamma;
    }
    return res;
}

SegmentEntity *SegmentArc::trim_back(double tail, bool rem) {
    SegmentArc* res = nullptr;
    double len = length();

    if (tail > M_PRECISION && tail < len) {
        double pct = tail / len;
        double da = deltaAngle();
        double gamma = range360(m_beta - da * pct);

        if (rem) res = new SegmentArc(m_C, m_R, gamma, m_beta, m_flags.ccw);
        m_beta = gamma;
    }
    return res;
}

SegmentEntity* SegmentArc::trim_back_rev(double head, bool rem) {
    SegmentArc* res = nullptr;
    double len = length();

    if (head > M_PRECISION && head < len) {
        double pct = head / len;
        double da = deltaAngle();
        double gamma = range360(m_alpha + da * pct);

        if (rem) res = new SegmentArc(m_C, m_R, gamma, m_beta, m_flags.ccw);
        m_beta = gamma;
    }
    return res;
}

void SegmentArc::offset(OFFSET_SIDE side, double offset, const SegmentEntity* prev, const SegmentEntity* next) {
    double a, a0, a1, g, b, R, dx, dy;

    fpoint_t A = point_0();
    fpoint_t B = point_1();
    fpoint_t C = this->m_C;
    bool ccw = m_flags.ccw;

    a = tangent_0();

    if (side == OFFSET_SIDE::LEFT) {
        a0 = (prev) ? prev->tangent_1() : a;
        g = (a0 - a) / 2;
        b = a + M_PI_2 + g;
        R = offset / cos(g);
        dx = R * cos(b);
        dy = R * sin(b);
        A.shift(dx, dy);

        a1 = (next) ? next->tangent_0() : a;
        g = (a - a1) / 2;
        b = a1 + M_PI_2 + g;
        R = offset / cos(g);
        dx = R * cos(b);
        dy = R * sin(b);
        B.shift(dx, dy);
    }
    else {
        a0 = (prev) ? prev->tangent_1() : a;
        g = (a0 - a) / 2;
        b = a + M_PI_2 + g;
        R = offset / cos(g);
        dx = R * cos(b);
        dy = R * sin(b);
        A.shift(dx, dy);

        a1 = (next) ? next->tangent_0() : a;
        g = (a - a1) / 2;
        b = M_PI_2 - (a1 + g);
        R = offset / cos(g);
        dx = R * cos(b);
        dy = R * sin(b);
        B.shift(dx, dy);
    }

    set(A, B, C, ccw);
}

void SegmentArc::offset(OFFSET_SIDE side, double offset) {
    bool ccw = m_flags.ccw;
    if (offset <= 0)
        return;

    if ((ccw && side == OFFSET_SIDE::LEFT) || (!ccw && side == OFFSET_SIDE::RIGHT))
        m_R -= offset;
    else
        m_R += offset;
}

void SegmentArc::rotate(const RotateMatrix &mx) {
    fpoint_t A = this->point_0();
    fpoint_t B = this->point_1();
    A.rotate(mx);
    B.rotate(mx);
    m_C.rotate(mx);
    set(A, B, m_C, m_flags.ccw);
}

void SegmentArc::flipX(double x) {
    m_C.flipX(x);
    m_flags.ccw = !m_flags.ccw;
    m_alpha = range360(M_PI - m_alpha);
    m_beta = range360(M_PI - m_beta);
}

void SegmentArc::flipY(double y) {
    m_C.flipY(y);
    m_flags.ccw = !m_flags.ccw;
    m_alpha = range360(2*M_PI - m_alpha);
    m_beta = range360(2*M_PI - m_beta);
}

void SegmentArc::scale(double k) {
    m_C.scale(k);
    m_R *= k;
}

double SegmentArc::dx() const { return radius() * (cos(m_beta) - cos(m_alpha)); }
double SegmentArc::dy() const { return radius() * (sin(m_beta) - sin(m_alpha)); }

void SegmentArc::shift(const fpoint_t& value) { m_C.shift(value); }

void SegmentArc::reverse() {
    swap<double>(m_alpha, m_beta);
    m_flags.ccw = !m_flags.ccw;
}

string SegmentArc::toString() const {
    return m_flags.valid ? "ARC: Center " + m_C.toString() +
            ", Radius " + to_string(m_R) +
            ", angles (" + to_string(m_alpha / M_PI * 180) + ", " + to_string(m_beta / M_PI * 180) + ")" +
                         ", CCW " + to_string(m_flags.ccw) : "Not valid";
}

string SegmentArc::toString2() const {
    if (m_flags.valid) {
        char buf[128];
        snprintf(buf, sizeof(buf), ", R: %.3f, Length: %.3f mm, CCW: %s.", m_R, length(), m_flags.ccw ? "True" : "False");

        fpoint_t A = point_0();
        fpoint_t B = point_1();

        string s = "ARC. " + A.toString() + " - " + B.toString() + ", C: " + m_C.toString() + buf;

        return s;
    }

    return "Not valid";
}

bool SegmentArc::between(double angle) const {
    return between(angle, m_alpha, m_beta, m_flags.ccw);
}

double SegmentArc::range360(double value) {
    const double M_360 = 2 * M_PI;

    if (value >= M_360) {
        while (value > M_360)
            value -= M_360;

        if (value < 0) value = 0;
    }
    else if (value < 0) {
        while (value < 0)
            value += M_360;

        if (value >= M_360) value = 0;
    }
    return value;
}

bool SegmentArc::between(double angle, double alpha, double beta, bool ccw) {
    angle = range360(angle);
    alpha = range360(alpha);
    beta = range360(beta);

    if (ccw)
        if (alpha < beta)
            return angle >= alpha && angle <= beta;
        else
            return angle >= alpha || angle <= beta;
    else
        if (alpha < beta)
            return !(angle > alpha && angle < beta);
        else
            return !(angle > alpha || angle < beta);
}

void SegmentArc::polar(const fpoint_t& C, const fpoint_t& A, double& R, double& a) {
    if (A != C) {
        double dx = A.x - C.x;
        double dy = A.y - C.y;

        R = sqrt(dx * dx + dy * dy);
        a = asin(dy / R); // [-90, +90]

        if (a < 0)
            if (dx < 0)
                a = M_PI - a;
            else
                a = 2 * M_PI + a;
        else if (dx < 0)
            a = M_PI - a;

        if (a <= 0 || a >= 2 * M_PI) a = 0;
    }
    else {
        R = 0;
        a = 0;
    }
}

double SegmentArc::deltaAngle(bool ccw, double alpha, double beta) {
    double delta;
    if (ccw)
        if (beta > alpha)
            delta = beta - alpha;
        else
            delta = 2 * M_PI - (alpha - beta);
    else
        if (beta > alpha)
            delta = -(2 * M_PI - (beta - alpha));
        else
            delta = -(alpha - beta);

    return delta;
}

double SegmentArc::deltaAngle() const {
    if (m_R <= 0)
        return 0;

    if (m_flags.circle)
        return 2 * M_PI;

    double a = m_alpha;

    if (m_flags.ccw) {
        if (a >= m_beta) a -= 2 * M_PI;
    }
    else {
        if (a <= m_beta) a += 2 * M_PI;
    }

    return deltaAngle(m_flags.ccw, a, m_beta);
}

void SegmentArc::clear() {
    SegmentEntity::clear();
    m_flags = {0,0,0,0,0,0,0,1};
    m_R = 0;
    m_alpha = 0;
    m_beta = 0;
    m_C = fpoint_t();
}

bool SegmentArc::check() {
    if (!m_flags.valid && m_flags.Cx && m_flags.Cy && m_flags.R && m_flags.alpha && m_flags.beta) {
        m_flags.Cx = 0;
        m_flags.Cy = 0;
        m_flags.R = 0;
        m_flags.alpha = 0;
        m_flags.beta = 0;
        m_flags.valid = 1;
    }
    return m_flags.valid;
}
