#ifndef FRECT_T_H
#define FRECT_T_H

#include "fpoint_t.h"

class frect_t {
    constexpr static const double CMP_PRECISION = CncParam::PRECISION;

    fpoint_t m_min{0,0}, m_max{0,0};

public:
    frect_t();
    frect_t(const fpoint_t& m_min, const fpoint_t& m_max);

    const fpoint_t& min() const;
    const fpoint_t& max() const;
    double width() const;
    double height() const;
    fpoint_t center() const;
    double resize(double width_req, double height_req);
};

#endif // FRECT_T_H
