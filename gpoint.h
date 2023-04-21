#ifndef GPOINT_H
#define GPOINT_H

#include <QColor>
#include <QPointF>

struct GPoint {
    QPointF point;
    QColor color;
    GPoint(const QPointF& point = QPointF(), const QColor& color = Qt::GlobalColor::blue);
    GPoint(const GPoint& other);
    GPoint(GPoint&& other);

    qreal x() const;
    qreal y() const;

    qreal& rx();
    qreal& ry();

    void setX(qreal x);
    void setY(qreal y);
};

#endif // GPOINT_H
