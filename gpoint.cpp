#include "gpoint.h"

GPoint::GPoint(const QPointF &point, const QColor &color) : point(point), color(color) {}

GPoint::GPoint(const GPoint &other) : point(other.point), color(other.color) {}

GPoint::GPoint(GPoint &&other) {
    point = std::move(other.point);
    color = std::move(other.color);
}

qreal GPoint::x() const { return point.x(); }

qreal GPoint::y() const { return point.y(); }

qreal &GPoint::rx() { return point.rx(); }

qreal &GPoint::ry() { return point.ry(); }

void GPoint::setX(qreal x) { point.setX(x); }

void GPoint::setY(qreal y) { point.setY(y); }
