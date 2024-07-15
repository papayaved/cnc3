#include <limits>
#include "contour_range.h"

using namespace std;

ContourRange::ContourRange() :
    valid(false),
    x_min( numeric_limits<double>::max() ),
    x_max( -numeric_limits<double>::max() ),
    y_min( numeric_limits<double>::max() ),
    y_max( -numeric_limits<double>::max() )
{
}

ContourRange::ContourRange(double x_min, double x_max, double y_min, double y_max) :
    valid(true),
    x_min( x_min ), x_max( x_max ),
    y_min( y_min ), y_max( y_max )
{
}

ContourRange::ContourRange(const std::vector<fpoint_t>& pts) :
    x_min( numeric_limits<double>::max() ),
    x_max( -numeric_limits<double>::max() ),
    y_min( numeric_limits<double>::max() ),
    y_max( -numeric_limits<double>::max() )
{
    append(pts);
}

ContourRange::ContourRange(const QVector<QPointF>& pts) :
    x_min( numeric_limits<double>::max() ),
    x_max( -numeric_limits<double>::max() ),
    y_min( numeric_limits<double>::max() ),
    y_max( -numeric_limits<double>::max() )
{
    append(pts);
}

ContourRange::ContourRange(const ContourRange& other) :
    valid( other.valid ),
    x_min( other.x_min ), x_max( other.x_max ),
    y_min( other.y_min ), y_max( other.y_max )
{
}

ContourRange &ContourRange::operator=(const ContourRange &other) {
    memcpy(this, &other, sizeof(ContourRange));
    return *this;
}

void ContourRange::append(const std::vector<fpoint_t> &pts) {
    valid = !pts.empty();

    for (const fpoint_t& pt: pts) {
        if (pt.x < x_min) x_min = pt.x;
        if (pt.x > x_max) x_max = pt.x;

        if (pt.y < y_min) y_min = pt.y;
        if (pt.y > y_max) y_max = pt.y;
    }
}

void ContourRange::append(const QVector<QPointF>& pts) {
    valid = !pts.empty();

    for (const QPointF& pt: pts) {
        if (pt.x() < x_min) x_min = pt.x();
        if (pt.x() > x_max) x_max = pt.x();

        if (pt.y() < y_min) y_min = pt.y();
        if (pt.y() > y_max) y_max = pt.y();
    }
}

void ContourRange::append(const QVector<QVector<QPointF>>& pts_set) {
    for (const QVector<QPointF>& pts: pts_set) {
        append(pts);
    }
}

double ContourRange::width() const { return valid && x_max > x_min ? x_max - x_min : 0; }
double ContourRange::height() const { return valid && y_max > y_min ? y_max - y_min : 0; }
fpoint_t ContourRange::center() const { return fpoint_t(meanX(), meanY()); }

double ContourRange::meanX() const { return valid ? (x_min + x_max) / 2.0 : 0; }
double ContourRange::meanY() const { return valid ? (y_min + y_max) / 2.0 : 0; }

//double ContourRange::ratio() const { return valid ? width() / height() : 1.0; }

void ContourRange::expandWidth(double value) {
    if (x_max > x_min) { // valid
        x_max += value;
        x_min -= value;
    }
    else {
        x_max = value;
        x_min = -value;
    }
}

void ContourRange::expandHeight(double value) {
    if (y_max > y_min) { // valid
        y_max += value;
        y_min -= value;
    }
    else {
        y_max = value;
        y_min = -value;
    }
}

void ContourRange::scale(const ContourRange &range) {
    if (range.valid) {
        if (x_max > x_min) { // valid
            if (range.x_min < x_min) x_min = range.x_min;
            if (range.x_max > x_max) x_max = range.x_max;
        }
        else {
            x_min = range.x_min;
            x_max = range.x_max;
        }

        if (y_max > y_min) { // valid
            if (range.y_min < y_min) y_min = range.y_min;
            if (range.y_max > y_max) y_max = range.y_max;
        }
        else {
            y_min = range.y_min;
            y_max = range.y_max;
        }

        valid = true;
    }
}
