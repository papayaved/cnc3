#include "dxf_arc.h"
#include <cmath>

using namespace std;

const double DxfArc::ANGLE_STEP = M_PI / 180;

DxfArc::DxfArc() :
    DxfEntity(ENTITY_TYPE::ARC), flags({0,0,0,0,0,0,0,1}), R(0), alpha(0), beta(0), C(fpoint_t()) {}

DxfArc::DxfArc(const fpoint_t& C, double R, double alpha, double beta, bool ccw) :
    DxfEntity(ENTITY_TYPE::ARC), flags({0,0,0,0,0,1,0,ccw}), R(R > 0 ? R : 0), alpha(range360(alpha)), beta(range360(beta)), C(C) {}

DxfArc::DxfArc(const fpoint_t &A, const fpoint_t &B, const fpoint_t &C, bool ccw) :
    DxfEntity(ENTITY_TYPE::ARC), flags({0,0,0,0,0,1,0,1}), R(0), alpha(0), beta(0), C(fpoint_t())
{
    set(A, B, C, ccw);
}

DxfArc::DxfArc(const fpoint_t& A, const fpoint_t& B, double R, bool ccw) :
    DxfEntity(ENTITY_TYPE::ARC), flags({0,0,0,0,0,1,0,1})
{
    set(A, B, R, ccw);
}

DxfArc::DxfArc(const DxfArc& other) :
    DxfEntity(other, ENTITY_TYPE::ARC),
    flags(other.flags),
    R(other.R),
    alpha(other.alpha),
    beta(other.beta),
    C(other.C)
{
}

DxfArc::~DxfArc() {}

DxfEntity* DxfArc::clone() const { return new DxfArc(*this); }

bool DxfArc::operator==(const DxfEntity& other) const {
    return same(other) && point_0() == other.point_0() && point_1() == other.point_1();
}

bool DxfArc::operator!=(const DxfEntity& other) const {
    return !(*this == other);
}

bool DxfArc::same(const DxfEntity& other) const {
    return other.type() == ENTITY_TYPE::ARC && radius() > other.radius() - M_PRECISION && radius() < other.radius() + M_PRECISION;
}

bool DxfArc::inside(const DxfArc& other) const {
    double L = DxfEntity::length(C, other.C);
    return L < fabs(other.radius() - radius());
}

bool DxfArc::outside(const DxfArc& other) const {
    double L = DxfEntity::length(C, other.C);
    return L > other.radius() + radius();
}

bool DxfArc::insideCircle(const fpoint_t& pt) const {
    return DxfEntity::length(C, pt) <= radius();
}

bool DxfArc::outsideCircle(const fpoint_t& pt) const { return !insideCircle(pt); }

bool DxfArc::intersected(const DxfArc& other) const {
    double L = DxfEntity::length(C, other.C);
    return L >= fabs(other.radius() - radius()) && L <= other.radius() + radius();
}

void DxfArc::setCenterX(double value) {
    flags.valid = 0;
    flags.Cx = 1;
    C.x = value;
}
void DxfArc::setCenterY(double value) {
    flags.valid = 0;
    flags.Cy = 1;
    C.y = value;
}
void DxfArc::setRadius(double value) {
    flags.valid = 0;
    flags.R = 1;
    R = value > 0 ? value : 0;
}

void DxfArc::setStartAngle(double rad) {
    flags.valid = 0;
    flags.alpha = 1;
    alpha = range360(rad);
}
void DxfArc::setEndAngle(double rad) {
    flags.valid = 0;
    flags.beta = 1;
    beta = range360(rad);
}

void DxfArc::setExtrusionDirectionZ(double value) { flags.ccw = value >= 0 ? 1 : 0; }

bool DxfArc::isPoint() const { return R < M_PRECISION; }

bool DxfArc::isCircle() const { return point_0() == point_1() && R >= M_PRECISION; }

vector<fpoint_t> DxfArc::getPoints() const {
    vector<fpoint_t> res;    

    double a = alpha;
    double R = radius();
    fpoint_t pt;

    res.push_back(point_0());

    if (flags.ccw) {
        if (a >= beta) a -= 2 * M_PI;
        a += ANGLE_STEP;

        for (; a < beta; a += ANGLE_STEP) {
            pt.x = C.x + R * cos(a);
            pt.y = C.y + R * sin(a);
            res.push_back(pt);
        }
    }
    else {
        if (a <= beta) a += 2 * M_PI;
        a -= ANGLE_STEP;

        for (; a > beta; a -= ANGLE_STEP) {            
            pt.x = C.x + R * cos(a);
            pt.y = C.y + R * sin(a);
            res.push_back(pt);
        }
    }

    res.push_back(point_1());
    return res;
}

void DxfArc::set(const fpoint_t& C, double R, double alpha, double beta, bool ccw) {
    flags = {0, 0, 0, 0, 0, 1, 0, ccw};

    this->R = R > 0 ? R :0;
    this->alpha = range360(alpha);
    this->beta = range360(beta);
    this->C = C;
}

void DxfArc::set(const fpoint_t& A, const fpoint_t& B, const fpoint_t& C, bool ccw) {
    flags = {0, 0, 0, 0, 0, 1, 0, ccw};

    if (A == C && B == C) { // point
        flags.ccw = ccw;
        R = 0;
        alpha = 0;
        beta = 0;
        this->C = C;
    }
    else if (A == B) { // circle
        flags.circle = 1;
        flags.ccw = ccw;
        polar(C, A, R, alpha);
        beta = alpha;
        this->C = C;
    }
    else if (A == C || B == C) { // error, DxfLine
        polar(A, B, R, beta);
        R /= 2;
        this->C.x = (A.x + B.x) / 2;
        this->C.y = (A.y + B.y) / 2;
        alpha = range360(beta + M_PI);
        flags.ccw = true;
    }
    else {
        double R2;
        polar(C, A, R, alpha);
        polar(C, B, R2, beta);
        // todo: R
        this->C = C;
        flags.ccw  = ccw;
    }
}

void DxfArc::set(const fpoint_t& A, const fpoint_t& B, double R_in, bool ccw) {
    flags = {0, 0, 0, 0, 0, 1, 0, ccw};

    bool biggest = R_in < 0;
    R = fabs(R_in);

    if (R == 0) { // point
        flags.ccw = ccw;
        alpha = 0;
        beta = 0;
        C = A;
        return;
    }

    double AB, a;
    polar(A, B, AB, a);

    fpoint_t D;
    D.x = (A.x + B.x) / 2;
    D.y = (A.y + B.y) / 2;

    double AB_2 = AB / 2;

    if (R < AB_2)
        R = AB_2;

    double L = sqrt(R * R - AB_2 * AB_2);
    double dx, dy;

    if (ccw) a += M_PI;

    dx = L * sin(a);
    dy = - L * cos(a);

    if (biggest) {
        dx = -dx;
        dy = -dy;
    }

    C.x = D.x + dx;
    C.y = D.y + dy;

    double gamma = asin(AB_2 / R); // [0, 90]

    double h = M_PI_2 + a;

    if (ccw) {
        alpha = h - gamma;
        beta = h + gamma;
    }
    else {
        alpha = h + gamma;
        beta = h - gamma;
    }

    alpha = range360(alpha);
    beta = range360(beta);

    flags.circle = A == B; // circle
}

double DxfArc::point_X0() const { return C.x + radius() * cos(alpha); }
double DxfArc::point_Y0() const { return C.y + radius() * sin(alpha); }
double DxfArc::point_X1() const { return C.x + radius() * cos(beta); }
double DxfArc::point_Y1() const { return C.y + radius() * sin(beta); }

fpoint_t DxfArc::point_0() const { return fpoint_t(point_X0(), point_Y0()); }
fpoint_t DxfArc::point_1() const { return fpoint_t(point_X1(), point_Y1()); }

double DxfArc::radius() const { return R > 0 ? R : 0 ; }

double DxfArc::tangent_0() const { return flags.ccw ? alpha + M_PI_2 : alpha - M_PI_2; }
double DxfArc::tangent_1() const { return flags.ccw ? beta + M_PI_2 : beta - M_PI_2; }

ContourRange DxfArc::range() const {
    ContourRange range;
    fpoint_t A  = point_0();
    fpoint_t B  = point_1();

    if (between(1.5 * M_PI, alpha, beta, flags.ccw))
        range.x_min = C.x - R;
    else
        range.x_min = B.x < A.x ? B.x : A.x;

    if (between(0.5 * M_PI, alpha, beta, flags.ccw))
        range.x_max = C.x + R;
    else
        range.x_max = B.x > A.x ? B.x : A.x;

    if (between(M_PI, alpha, beta, flags.ccw))
        range.y_min = C.y - R;
    else
        range.y_min = B.y < A.y ? B.y : A.y;

    if (between(0, alpha, beta, flags.ccw))
        range.y_max = C.y + R;
    else
        range.y_max = B.y > A.y ? B.y : A.y;

    range.valid = true;

    return range;
}

double DxfArc::length() const { return fabs(deltaAngle()) * radius(); }

double DxfArc::length(const fpoint_t& pt) const {
    double CP, gamma;
    polar(C, pt, CP, gamma);
    gamma = range360(gamma);

    double dGamma = deltaAngle(flags.ccw, alpha, gamma);

    bool less180 = fabs(deltaAngle()) < M_PI;

    if (dGamma <= 0 && less180)
        return 0;
    else {
        double dArc = fabs(deltaAngle());
        dGamma = fabs(dGamma);

        if (dGamma >= dArc)
            return length();
        else
            return dGamma * radius();
    }
}
double DxfArc::distance(const fpoint_t& pt) const {
    double CP, gamma;
    polar(C, pt, CP, gamma);
    gamma = range360(gamma);

    double dGamma = deltaAngle(flags.ccw, alpha, gamma);

    if (dGamma <= 0)
        return DxfEntity::length(point_0(), pt);
    else {
        double dArc = deltaAngle();

        if (dGamma >= dArc)
            return DxfEntity::length(point_1(), pt);
        else
            return DxfEntity::length(C, pt) - radius();
    }
}

DxfEntity* DxfArc::trim_front(double head, bool rem) {
    DxfArc* res = nullptr;
    double len = length();

    if (head >= M_PRECISION && head < (len - M_PRECISION)) {
        double pct = head / len;
        double da = deltaAngle();
        double gamma = range360(alpha + da * pct);

        if (rem) res = new DxfArc(C, R, alpha, gamma, flags.ccw);
        alpha = gamma;
    }
    return res;
}

DxfEntity *DxfArc::trim_front_rev(double tail, bool rem) {
    DxfArc* res = nullptr;
    double len = length();

    if (tail >= M_PRECISION && tail < len) {
        double pct = tail / len;
        double da = deltaAngle();
        double gamma = range360(beta - da * pct);

        if (rem) res = new DxfArc(C, R, alpha, gamma, flags.ccw);
        alpha = gamma;
    }
    return res;
}

DxfEntity *DxfArc::trim_back(double tail, bool rem) {
    DxfArc* res = nullptr;
    double len = length();

    if (tail > M_PRECISION && tail < len) {
        double pct = tail / len;
        double da = deltaAngle();
        double gamma = range360(beta - da * pct);

        if (rem) res = new DxfArc(C, R, gamma, beta, flags.ccw);
        beta = gamma;
    }
    return res;
}

DxfEntity* DxfArc::trim_back_rev(double head, bool rem) {
    DxfArc* res = nullptr;
    double len = length();

    if (head > M_PRECISION && head < len) {
        double pct = head / len;
        double da = deltaAngle();
        double gamma = range360(alpha + da * pct);

        if (rem) res = new DxfArc(C, R, gamma, beta, flags.ccw);
        beta = gamma;
    }
    return res;
}

void DxfArc::offset(OFFSET_SIDE side, double offset, const DxfEntity* prev, const DxfEntity* next) {
    double a, a0, a1, g, b, R, dx, dy;

    fpoint_t A = point_0();
    fpoint_t B = point_1();
    fpoint_t C = this->C;
    bool ccw = flags.ccw;

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

void DxfArc::offset(OFFSET_SIDE side, double offset) {
    bool ccw = flags.ccw;
    if (offset <= 0)
        return;

    if ((ccw && side == OFFSET_SIDE::LEFT) || (!ccw && side == OFFSET_SIDE::RIGHT))
        R -= offset;
    else
        R += offset;
}

void DxfArc::rotate(const RotateMatrix &mx) {
    fpoint_t A = this->point_0();
    fpoint_t B = this->point_1();
    A.rotate(mx);
    B.rotate(mx);
    C.rotate(mx);
    set(A, B, C, flags.ccw);
}

void DxfArc::flipX(double x) {
    C.flipX(x);
    flags.ccw = !flags.ccw;
    alpha = range360(M_PI - alpha);
    beta = range360(M_PI - beta);
}

void DxfArc::flipY(double y) {
    C.flipY(y);
    flags.ccw = !flags.ccw;
    alpha = range360(2*M_PI - alpha);
    beta = range360(2*M_PI - beta);
}

void DxfArc::scale(double k) {
    C.scale(k);
    R *= k;
}

double DxfArc::dx() const { return radius() * (cos(beta) - cos(alpha)); }
double DxfArc::dy() const { return radius() * (sin(beta) - sin(alpha)); }

void DxfArc::shift(const fpoint_t& value) { C.shift(value); }

void DxfArc::reverse() {
    swap<double>(alpha, beta);
    flags.ccw = !flags.ccw;
}

string DxfArc::toString() const {
    return flags.valid ? "ARC: Center " + C.toString() +
            ", Radius " + to_string(R) +
            ", angles (" + to_string(alpha / M_PI * 180) + ", " + to_string(beta / M_PI * 180) + ")" +
                         ", CCW " + to_string(flags.ccw) : "Not valid";
}

string DxfArc::toString2() const {
    if (flags.valid) {
        char buf[128];
        sprintf_s(buf, sizeof(buf), ", R: %.3f, Length: %.3f mm, CCW: %s.", R, length(), flags.ccw ? "True" : "False");

        fpoint_t A = point_0();
        fpoint_t B = point_1();

        string s = "ARC. " + A.toString() + " - " + B.toString() + ", C: " + C.toString() + buf;

        return s;
    }

    return "Not valid";
}

bool DxfArc::between(double angle) const {
    return between(angle, alpha, beta, flags.ccw);
}

double DxfArc::range360(double value) {
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

bool DxfArc::between(double angle, double alpha, double beta, bool ccw) {
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

void DxfArc::polar(const fpoint_t& C, const fpoint_t& A, double& R, double& a) {
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

double DxfArc::deltaAngle(bool ccw, double alpha, double beta) {
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

double DxfArc::deltaAngle() const {
    if (R <= 0)
        return 0;

    if (flags.circle)
        return 2 * M_PI;

    double a = alpha;

    if (flags.ccw) {
        if (a >= beta) a -= 2 * M_PI;
    }
    else {
        if (a <= beta) a += 2 * M_PI;
    }

    return deltaAngle(flags.ccw, a, beta);
}

void DxfArc::clear() {
    DxfEntity::clear();
    flags = {0,0,0,0,0,0,0,1};
    R = 0;
    alpha = 0;
    beta = 0;
    C = fpoint_t();
}

bool DxfArc::check() {
    if (!flags.valid && flags.Cx && flags.Cy && flags.R && flags.alpha && flags.beta) {
        flags.Cx = 0;
        flags.Cy = 0;
        flags.R = 0;
        flags.alpha = 0;
        flags.beta = 0;
        flags.valid = 1;
    }
    return flags.valid;
}
