#include "segment_point.h"

using namespace std;

SegmentPoint::SegmentPoint(bool additional) : SegmentEntity(DXF_ENTITY_TYPE::POINT), m_flags({0,0,0}), m_pt(fpoint_t()) {
    m_additional = additional;
}
SegmentPoint::SegmentPoint(const fpoint_t &point, bool additional) :
    SegmentEntity(DXF_ENTITY_TYPE::POINT),
    m_flags({0,0,1}),
    m_pt(point)
{
    m_additional = additional;
}
SegmentPoint::SegmentPoint(double x, double y, bool additional) :
    SegmentEntity(DXF_ENTITY_TYPE::POINT),
    m_flags({0,0,1}),
    m_pt(fpoint_t(x, y))
{
    m_additional = additional;
}
SegmentPoint::SegmentPoint(const SegmentPoint &other) : SegmentEntity(other, DXF_ENTITY_TYPE::POINT), m_flags(other.m_flags), m_pt(other.m_pt) {}
SegmentPoint::~SegmentPoint() {}

SegmentEntity *SegmentPoint::clone() const { return new SegmentPoint(*this); }

void SegmentPoint::setX(double value) {
    m_flags.valid = 0;
    m_flags.X = 1;
    m_pt.x = value;
}

void SegmentPoint::setY(double value) {
    m_flags.valid = 0;
    m_flags.Y = 1;
    m_pt.y = value;
}

void SegmentPoint::set(const fpoint_t& point, bool additional) {
    m_flags = {0,0,1};
    m_additional = additional;
    m_pt = point;
}

double SegmentPoint::X() const { return m_pt.x; }
double SegmentPoint::Y() const { return m_pt.y; }
bool SegmentPoint::isX() const { return m_flags.X; }
bool SegmentPoint::isY() const { return m_flags.Y; }

bool SegmentPoint::check() {
    if (!m_flags.valid && m_flags.X && m_flags.Y) {
        m_flags = {0,0,1};
        m_additional = false;
    }

    return m_flags.valid;
}

void SegmentPoint::clear() {
    SegmentEntity::clear();
    m_flags = {0,0,0};
    m_additional = false;
    m_pt = fpoint_t();
}

void SegmentPoint::flipX(double x) {
    m_pt.flipX(x);
}

void SegmentPoint::flipY(double y) {
    m_pt.flipY(y);
}

void SegmentPoint::scale(double k) {
    m_pt.scale(k);
}

string SegmentPoint::toString() const {
    return m_flags.valid ? "Point: " + m_pt.toString() : "Point is not valid";
}

string SegmentPoint::toString2() const {
    return m_flags.valid ? "Point: " + m_pt.toString() : "Point is not valid";
}

ContourRange SegmentPoint::range() const { return ContourRange(m_pt.x, m_pt.x, m_pt.y, m_pt.y); }
