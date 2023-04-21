#ifndef FPOINT_T_H
#define FPOINT_T_H

#include "cnc_param.h"
#include <string>
#include <QPointF>

struct RotateMatrix {
    double mx[2][2] {{1,0}, {0,1}};

    RotateMatrix(double phi);
    void set(double phi);
};

struct fpoint_t {
    constexpr static const double CMP_PRECISION = CncParam::PRECISION; // 0.5 um
    double x{0}, y{0};

    explicit fpoint_t(double x = 0, double y = 0);
    fpoint_t(const fpoint_t& other);

    fpoint_t& operator=(const fpoint_t& other);
    bool operator==(const fpoint_t& other) const;
    bool operator!=(const fpoint_t& other) const;
    fpoint_t operator-() const;
    fpoint_t operator+(const fpoint_t& other) const;
    fpoint_t operator-(const fpoint_t& other) const;

    void shift(const fpoint_t& offset);
    void shift(double dx, double dy);

    void rotate(const RotateMatrix& mx);
    void flipX(double x);
    void flipY(double y);
    void scale(double pct);

    std::string toString() const;

    operator QPointF() { return QPointF(x, y); }

    void swap();
};

struct fpoint_valid_t : public fpoint_t {
    bool valid {false};

    fpoint_valid_t(double _x = 0, double _y = 0, bool valid = true) : fpoint_t(_x, _y), valid(valid) {}
    fpoint_valid_t(const fpoint_t& point, bool valid = true) : fpoint_t(point), valid(valid) {}
    explicit fpoint_valid_t(bool valid) : fpoint_t(fpoint_t()), valid(valid) {}
    fpoint_valid_t(const fpoint_valid_t& other) : fpoint_t(other.x, other.y), valid(other.valid) {}

    static fpoint_valid_t null() { return fpoint_valid_t(0,0,false); }

    fpoint_valid_t& operator=(const fpoint_valid_t& other);
    bool operator==(const fpoint_valid_t& other) const;
    bool operator!=(const fpoint_valid_t& other) const;
//    fpoint_valid_t operator-() const;

    std::string toString() const;
};

#endif // FPOINT_T_H
