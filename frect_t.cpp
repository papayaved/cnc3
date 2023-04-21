#include <cmath>
#include "frect_t.h"

frect_t::frect_t() : m_min(0,0), m_max(0,0) {}
frect_t::frect_t(const fpoint_t& _min, const fpoint_t& _max) : m_min(_min), m_max(_max) {
    if (m_max.x < m_min.x) {
        double x = m_min.x;
        m_min.x = m_max.x;
        m_max.x = x;
    }
    if (m_max.y < m_min.y) {
        double y = m_min.y;
        m_min.y = m_max.y;
        m_max.y = y;
    }
}

const fpoint_t &frect_t::min() const { return m_min; }

const fpoint_t &frect_t::max() const { return m_max; }

double frect_t::width() const { return m_max.x - m_min.x; }
double frect_t::height() const { return m_max.y - m_min.y; }

fpoint_t frect_t::center() const {
    return fpoint_t( (m_min.x + m_max.x) / 2.0, (m_min.y + m_max.y) / 2.0 );
}

double frect_t::resize(double w_req, double h_req) {
    double k;
    double w = width();
    double h = height();

    bool w_ok = w_req >= CMP_PRECISION;
    bool h_ok = h_req >= CMP_PRECISION;

    if (w_ok && h_ok) {
        if (h < CMP_PRECISION) { // horizontal
            k = w < CMP_PRECISION ? 1 : w_req / w;
        }
        else if (w < CMP_PRECISION) { // vertical
            k = h < CMP_PRECISION ? 1 : h_req / h;
        }
        else if (w > h) {
            double r = w / h;
            double wh = r * h_req;

            if (wh < w_req)
                w_req = wh;

            k = w_req / w;
        }
        else { // h > w
            double r = h / w;
            double hw = r * w_req;

            if (hw < h_req)
                h_req = hw;

            k = h_req / h;
        }
    }
    else {
        if (w_ok)
            k = w < CMP_PRECISION ? 1 : w_req / w;
        else if (h_ok)
            k = h < CMP_PRECISION ? 1 : h_req / h;
        else
            k = 1;
    }

    h *= k;
    w *= k;
    m_max.x = m_min.x + w;
    m_max.y = m_min.y + h;
    return k;
}
