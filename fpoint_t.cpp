#include "fpoint_t.h"
#include <cmath>
#include <stdio.h>

using namespace std;

RotateMatrix::RotateMatrix(double phi) {
    set(phi);
}

void RotateMatrix::set(double phi) {
    mx[1][0] = sin(phi);
    mx[0][1] = -mx[1][0];
    mx[0][0] = mx[1][1] = cos(phi);
}

fpoint_t::fpoint_t(double x, double y) : x(x), y(y) {}
fpoint_t::fpoint_t(const fpoint_t &other) : x(other.x), y(other.y) {}

fpoint_t& fpoint_t::operator=(const fpoint_t& other) {
    if (this != &other) {
        x = other.x;
        y = other.y;
    }
    return *this;
}

bool fpoint_t::operator==(const fpoint_t& other) const {
    return x > other.x - CMP_PRECISION && x <= other.x + CMP_PRECISION && y > other.y - CMP_PRECISION && y <= other.y + CMP_PRECISION;
}

bool fpoint_t::operator!=(const fpoint_t& other) const {
    return !this->operator==(other);
}

fpoint_t fpoint_t::operator-() const {
    return fpoint_t(-x, -y);
}
fpoint_t fpoint_t::operator+(const fpoint_t &other) const {
    return fpoint_t(x + other.x, y + other.y);
}
fpoint_t fpoint_t::operator-(const fpoint_t &other) const {
    return fpoint_t(x - other.x, y - other.y);
}

void fpoint_t::shift(const fpoint_t& offset) {
    x += offset.x;
    y += offset.y;
}
void fpoint_t::shift(double dx, double dy) {
    x += dx;
    y += dy;
}

void fpoint_t::rotate(const RotateMatrix &mx) {
    double xx = x * mx.mx[0][0] + y * mx.mx[0][1];
    double yy = x * mx.mx[1][0] + y * mx.mx[1][1];
    x = xx;
    y = yy;
}

void fpoint_t::flipX(double X) {
    x = 2*X - x;
}

void fpoint_t::flipY(double Y) {
    y = 2*Y - y;
}

void fpoint_t::scale(double pct) {
    x *= pct;
    y *= pct;
}

string fpoint_t::toString() const {
//    s = "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
    char buf[128];
    snprintf(buf, sizeof(buf), "(%.3f, %.3f)", x, y);
    string s(buf);
    return s;
}

void fpoint_t::swap() {
    double t = x;
    x = y;
    y = t;
}

fpoint_valid_t& fpoint_valid_t::operator=(const fpoint_valid_t& other) {
    if (this != &other) {
        x = other.x;
        y = other.y;
        valid = other.valid;
    }
    return *this;
}

bool fpoint_valid_t::operator==(const fpoint_valid_t& other) const {
    return static_cast<fpoint_t>(*this) == static_cast<fpoint_t>(other) && valid && other.valid;
}

bool fpoint_valid_t::operator!=(const fpoint_valid_t &other) const {
    return !this->operator==(other);
}

//fpoint_valid_t fpoint_valid_t::operator-() const {
//    return static_cast<fpoint_t>(*this).operator-();
//}

string fpoint_valid_t::toString() const {
    return valid ? static_cast<fpoint_t>(*this).toString() : "Null";
}
