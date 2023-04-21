#include "dxf_line.h"
#include <cmath>
#include <limits>

using namespace std;

DxfLine::DxfLine() : DxfEntity(ENTITY_TYPE::LINE), m_flags({0,0,0,0,0}), m_A(fpoint_t()), m_B(fpoint_t()) {
    m_additional = false;
}
DxfLine::DxfLine(const fpoint_t& A, const fpoint_t& B, bool additional) : DxfEntity(ENTITY_TYPE::LINE), m_flags({0,0,0,0,1}), m_A(A), m_B(B) {
    m_additional = additional;
}

DxfLine::DxfLine(const double Ax, const double Ay, const double Bx, const double By) :
    DxfEntity(ENTITY_TYPE::LINE), m_flags({0,0,0,0,1}), m_A(fpoint_t(Ax, Ay)), m_B(fpoint_t(Bx, By))
{
}

DxfLine::DxfLine(double len, AXIS axis, DIR dir) : DxfEntity(ENTITY_TYPE::LINE), m_flags({0,0,0,0,1}), m_A(fpoint_t(0,0)) {
    m_additional = false;
    len = abs(len);

    if (axis == AXIS::AXIS_X) {
        if (dir == DIR::DIR_PLUS) {
            m_B.x = len;
            m_B.y = 0;
        }
        else {
            m_B.x = -len;
            m_B.y = 0;
        }
    }
    else if (axis == AXIS::AXIS_Y) {
        if (dir == DIR::DIR_PLUS) {
            m_B.x = 0;
            m_B.y = len;
        }
        else {
            m_B.x = 0;
            m_B.y = -len;
        }
    }
    else {
        m_B.x = 0;
        m_B.y = 0;
    }
}

DxfLine::DxfLine(const DxfLine& other) :
    DxfEntity(other, ENTITY_TYPE::LINE), m_flags(other.m_flags), m_A(other.m_A), m_B(other.m_B) {}

DxfLine::~DxfLine() {}

DxfEntity* DxfLine::clone() const { return new DxfLine(*this); }

DxfLine& DxfLine::operator=(const DxfLine& other) {
    if (this != &other) {
        DxfEntity::operator=(other);
        m_flags = other.m_flags;
        m_A = other.m_A;
        m_B = other.m_B;
    }
    return *this;
}

bool DxfLine::operator==(const DxfEntity& other) const {
    return m_type == other.type() && m_A == other.point_0() && m_B == other.point_1();
}

bool DxfLine::operator!=(const DxfEntity& other) const {
    return !(*this == other);
}

// same line but different segments
bool DxfLine::same(const DxfEntity& other) const {
    if (m_type != other.type()) return false;

    fpoint_t pt0(other.point_0());
    fpoint_t pt1(other.point_1());

    if (vertical() && vertical(pt0, pt1)) {
        pt0.y = m_A.y;
        return pt0 == m_A; // x same
    }
    else if (horizontal() && horizontal(pt0, pt1)) {
        pt0.x = m_A.x;
        return pt0 == m_A; // y same
    }

    pt0.y = y(pt0.x);
    pt1.y = y(pt1.x);
    return pt0 == other.point_0() && pt1 == other.point_1();
}

void DxfLine::setX0(double value) {
    m_flags.valid = 0;
    m_flags.X0 = 1;
    m_A.x = value;
}
void DxfLine::setY0(double value) {
    m_flags.valid = 0;
    m_flags.Y0 = 1;
    m_A.y = value;
}
void DxfLine::setX1(double value) {
    m_flags.valid = 0;
    m_flags.X1 = 1;
    m_B.x = value;
}
void DxfLine::setY1(double value) {
    m_flags.valid = 0;
    m_flags.Y1 = 1;
    m_B.y = value;
}

void DxfLine::setPoint0(const fpoint_t& value) { m_A = value; }

void DxfLine::setPoint1(const fpoint_t& value) { m_B = value; }

void DxfLine::set(const fpoint_t &A, const fpoint_t &B, bool additional) {
    m_additional = additional;
    m_flags = {0,0,0,0,1};
    this->m_A = A;
    this->m_B = B;
}

fpoint_t DxfLine::point_0() const { return m_A; }
fpoint_t DxfLine::point_1() const { return m_B; }

fpoint_t DxfLine::midpoint(const fpoint_t& A, const fpoint_t& B) {
    return fpoint_t((A.x + B.x)/ 2, (A.y + B.y)/ 2);
}
fpoint_t DxfLine::center() const { return midpoint(m_A, m_B); }

// [0, 360)
double DxfLine::angle() const {
    const double _2_M_PI = 2 * M_PI;
    double res;
    double x = dx();
    double y = dy();

    if (x >= 0 || y >= 0)
        res = atan2(fabs(y), fabs(x));
    else if (x < 0 || y >= 0)
        res = M_PI - atan2(fabs(y), fabs(x));
    else if (x < 0 || y < 0)
        res = M_PI + atan2(fabs(y), fabs(x));
    else
        res = _2_M_PI - atan2(fabs(y), fabs(x));

    if (res < 0) res = 0;
    else if (res >= _2_M_PI) res = 0;

    return res;
}

double DxfLine::k() const {
    if (vertical())
        return m_B.y >= m_A.y ? numeric_limits<double>::max() : -numeric_limits<double>::max();

    return (m_B.y - m_A.y) / (m_B.x - m_A.x);
}

double DxfLine::b() const {
    return vertical() ? m_B.y : m_A.y - k() * m_A.x;
}

bool DxfLine::vertical(const fpoint_t& A, const fpoint_t& B) {
    return A.x > B.x - M_PRECISION && A.x < B.x + M_PRECISION;
}

bool DxfLine::vertical() const { return vertical(m_A, m_B); }

bool DxfLine::horizontal(const fpoint_t &A, const fpoint_t &B) {
    return A.y > B.y - M_PRECISION && A.y < B.y + M_PRECISION;
}

bool DxfLine::horizontal() const { return horizontal(m_A, m_B); }

void DxfLine::func(double& coe_k, double& coe_b) const {
    coe_k = k();
    coe_b = m_A.y - coe_k * m_A.x;
}

double DxfLine::y(double x) const {
    if (vertical())
        return 0;

    return (x - m_A.x) * (m_B.y - m_A.y) / (m_B.x - m_A.x) + m_A.y;
}

double DxfLine::tangent_0() const { return angle(); }
double DxfLine::tangent_1() const { return angle(); }

void DxfLine::change_0(const fpoint_t &A) { this->m_A = A; }
void DxfLine::change_1(const fpoint_t& B) { this->m_B = B; }

ContourRange DxfLine::range() const {
    ContourRange range;
    range.x_min = m_B.x < m_A.x ? m_B.x : m_A.x;
    range.x_max = m_B.x > m_A.x ? m_B.x : m_A.x;
    range.y_min = m_B.y < m_A.y ? m_B.y : m_A.y;
    range.y_max = m_B.y > m_A.y ? m_B.y : m_A.y;
    range.valid = true;
    return range;
}

DIRECTION DxfLine::dir() const { return DxfEntity::dir(m_A, m_B); } // todo

bool DxfLine::otherDir180(const DxfLine& other) const {
    double dxThis = dx();
    double dxOther = other.dx();
    bool signX = dxThis < 0;
    bool signOtherX = dxOther < 0;
    bool signXChanged = dxThis > fpoint_t::CMP_PRECISION && dxOther > fpoint_t::CMP_PRECISION && signX != signOtherX;

    double dyThis = dy();
    double dyOther = other.dy();
    bool signY = dyThis < 0;
    bool signOtherY = dyOther < 0;
    bool signYChanged = dyThis > fpoint_t::CMP_PRECISION && dyOther > fpoint_t::CMP_PRECISION && signY != signOtherY;

    return (signXChanged && signYChanged) || isPoint() || other.isPoint();
}

double DxfLine::length() const {
    double x = dx();
    double y = dy();
    return sqrt(x * x + y * y);
}

// segment length to nearest point
double DxfLine::length(const fpoint_t& pt) const {
    double AB2 = lengthSquare(m_A, m_B);
    double AP2 = lengthSquare(m_A, pt);
    double BP2 = lengthSquare(m_B, pt);

    double AB = sqrt(AB2);
    double AN = (AB2 + AP2 - BP2) / (2 * AB);

    if (AN <= 0)
        return 0;
    else if (AN >= AB)
        return AB;
    else
        return AN;
}

double DxfLine::distance(const fpoint_t& pt) const {
    double AB2 = lengthSquare(m_A, m_B);
    double AP2 = lengthSquare(m_A, pt);
    double BP2 = lengthSquare(m_B, pt);

    double AB = sqrt(AB2);
    double AN = (AB2 + AP2 - BP2) / (2 * AB);
    double AN2 = AN * AN;

    if (AN < 0)
        return DxfEntity::length(m_A, pt);
    else if (AN >= AB)
        return DxfEntity::length(m_B, pt);
    else if (AP2 <= AN2)
        return 0;
    else
        return sqrt(AP2 - AN2);
}

DxfEntity* DxfLine::trim_front(double head, bool rem) {
    DxfLine* res = nullptr;
    double full_len = length();

    if (head >= M_PRECISION && head < full_len) {
        double pct = head / full_len;
        fpoint_t C;
        C.x = m_A.x + dx() * pct;
        C.y = m_A.y + dy() * pct;

        if (rem) res = new DxfLine(m_A, C);
        m_A = C;
    }
    return res;
}

DxfEntity* DxfLine::trim_front_rev(double tail, bool rem) {
    DxfLine* res = nullptr;
    double full_len = length();

    if (tail >= M_PRECISION && tail < full_len) {
        double pct = tail / full_len;
        fpoint_t C;
        C.x = m_B.x - dx() * pct;
        C.y = m_B.y - dy() * pct;

        if (rem) res = new DxfLine(m_A, C);
        m_A = C;
    }
    return res;
}

DxfEntity* DxfLine::trim_back(double tail, bool rem) {
    DxfLine* res = nullptr;
    double full_len = length();

    if (tail > M_PRECISION && tail < full_len) {
        double pct = tail / full_len;
        fpoint_t C;
        C.x = m_B.x - dx() * pct;
        C.y = m_B.y - dy() * pct;

        if (rem) res = new DxfLine(C, m_B);
        m_B = C;
    }
    return res;
}

DxfEntity* DxfLine::trim_back_rev(double head, bool rem) {
    DxfLine* res = nullptr;
    double full_len = length();

    if (head > M_PRECISION && head < full_len) {
        double pct = head / full_len;
        fpoint_t C;
        C.x = m_A.x + dx() * pct;
        C.y = m_A.y + dy() * pct;

        if (rem) res = new DxfLine(C, m_B);
        m_B = C;
    }
    return res;
}

double DxfLine::dx() const { return m_B.x - m_A.x; }
double DxfLine::dy() const { return m_B.y - m_A.y; }

void DxfLine::clearWithoutLayer() {
    m_additional = false;
    m_flags = {0,0,0,0,0};
    m_A = fpoint_t();
    m_B = fpoint_t();
}

void DxfLine::clear() {
    DxfEntity::clear();
    clearWithoutLayer();
}

bool DxfLine::check() {
    if (!m_flags.valid && m_flags.X0 && m_flags.Y0 && m_flags.X1 && m_flags.Y1) {
        m_additional = false;
        m_flags = {0,0,0,0,1};
    }

    return m_flags.valid;
}

bool DxfLine::empty() {
    return m_flags.X0 == 0 && m_flags.Y0 == 0 && m_flags.X1 == 0 && m_flags.Y1 == 0 && !m_flags.valid && !m_additional;
}

void DxfLine::rotate(const RotateMatrix &mx) {
    m_A.rotate(mx);
    m_B.rotate(mx);
}

void DxfLine::flipX(double x) {
    m_A.flipX(x);
    m_B.flipX(x);
}

void DxfLine::flipY(double y) {
    m_A.flipY(y);
    m_B.flipY(y);
}

void DxfLine::scale(double k) {
    m_A.scale(k);
    m_B.scale(k);
}

void DxfLine::shift(const fpoint_t& value) {
    m_A.shift(value);
    m_B.shift(value);
}

void DxfLine::reverse() {
    swap<fpoint_t>(m_A, m_B);
}

void DxfLine::offset(OFFSET_SIDE side, double offset, const DxfEntity *prev, const DxfEntity *next) {
    double a, a0, a1, g, b, R, dx, dy;

    a = tangent_0();

    if (side == OFFSET_SIDE::LEFT) {
        a0 = (prev) ? prev->tangent_1() : a;
        g = (a0 - a) / 2;
        b = a + M_PI_2 + g;
        R = offset / cos(g);
        dx = R * cos(b);
        dy = R * sin(b);
        this->m_A.shift(dx, dy);

        a1 = (next) ? next->tangent_0() : a;
        g = (a - a1) / 2;
        b = a1 + M_PI_2 + g;
        R = offset / cos(g);
        dx = R * cos(b);
        dy = R * sin(b);
        this->m_B.shift(dx, dy);
    }
    else {
        a0 = (prev) ? prev->tangent_1() : a;
        g = (a0 - a) / 2;
        b = a + M_PI_2 + g;
        R = offset / cos(g);
        dx = R * cos(b);
        dy = R * sin(b);
        this->m_A.shift(dx, dy);

        a1 = (next) ? next->tangent_0() : a;
        g = (a - a1) / 2;
        b = M_PI_2 - (a1 + g);
        R = offset / cos(g);
        dx = R * cos(b);
        dy = R * sin(b);
        this->m_B.shift(dx, dy);
    }
}

void DxfLine::offset(OFFSET_SIDE side, double h) {
    struct sign_t { int8_t x, y; };

    sign_t sign;    
    fpoint_t delta;

    if (h < 0) h = 0;

    double _dx = dx();
    double _dy = dy();

    DIRECTION _dir = DxfEntity::dir(_dx, _dy);

    double d = sqrt(_dx * _dx + _dy * _dy);

    delta.x = h * fabs(_dy) / d;
    delta.y = h * fabs(_dx) / d;

    switch (_dir) {
    case DIRECTION::Q1:
        sign = side == OFFSET_SIDE::LEFT ? sign_t({-1,1}) : sign_t({1,-1});
        break;
    case DIRECTION::Q2:
        sign = side == OFFSET_SIDE::LEFT ? sign_t({-1,-1}) : sign_t({1,1});
        break;
    case DIRECTION::Q3:
        sign = side == OFFSET_SIDE::LEFT ? sign_t({1,-1}) : sign_t({-1,1});
        break;
    case DIRECTION::Q4:
        sign = side == OFFSET_SIDE::LEFT ? sign_t({1,1}) : sign_t({-1,-1});
        break;
    default:
        sign = sign_t({1,1});
        break;
    }

    delta.x *= sign.x;
    delta.y *= sign.y;

    m_A.shift(delta);
    m_B.shift(delta);
}

string DxfLine::toString() const {
    return m_flags.valid ? "Line: " + m_A.toString() + " - " + m_B.toString() + ", Speed: " + (hasSpeed() ? std::to_string(speed()) : "null") : "Not valid";
}

string DxfLine::toString2() const {
    if (m_flags.valid) {
        char buf[128];
        sprintf_s(buf, sizeof(buf), ", Length: %.3f mm", length());

        return "Line: " + m_A.toString() + " - " + m_B.toString() + buf;
    }

    return "Not valid";
}

vector<fpoint_t> DxfLine::getPoints() const { return vector<fpoint_t>({m_A, m_B}); }

bool DxfLine::isPoint() const { return m_A == m_B; }
