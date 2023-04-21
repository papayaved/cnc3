#ifndef CONTOURRANGE_H
#define CONTOURRANGE_H

#include <QVector>
#include <QPointF>
#include "fpoint_t.h"

struct ContourRange {
    constexpr static const double CMP_PRECISION = CncParam::PRECISION;
    bool valid;
    double x_min, x_max, y_min, y_max;

    ContourRange();
    ContourRange(double x_min, double x_max, double y_min, double y_max);
    ContourRange(const std::vector<fpoint_t>& pts);
    ContourRange(const QVector<QPointF>& pts);
    ContourRange(const ContourRange& other);

    ContourRange& operator=(const ContourRange& other);

    void append(const std::vector<fpoint_t>& pts);
    void append(const QVector<QPointF>& pts);
    void append(const QVector<QVector<QPointF>>& pts_set);

    double width() const;
    double height() const;
    fpoint_t center() const;
//    double ratio() const;

    double meanX() const;
    double meanY() const;

    void expandWidth(double ratio);
    void expandHeight(double ratio);

    void scale(const ContourRange& range);

    static bool equal(double x, double y, double prec) { return x > y - prec && x < y + prec; }
    bool equal(const ContourRange& other, double prec = 0.001) const {
        return equal(x_min, other.x_min, prec) && equal(x_max, other.x_max, prec) && equal(y_min, other.y_min, prec) && equal(y_max, other.y_max, prec);
    }
};

#endif // CONTOURRANGE_H
