#include "dxf_point.h"

using namespace std;

DxfPoint::DxfPoint(bool additional) : DxfEntity(ENTITY_TYPE::POINT), m_flags({0,0,0}), m_pt(fpoint_t()) {
    m_additional = additional;
}
DxfPoint::DxfPoint(const fpoint_t &point, bool additional) :
    DxfEntity(ENTITY_TYPE::POINT),
    m_flags({0,0,1}),
    m_pt(point)
{
    m_additional = additional;
}
DxfPoint::DxfPoint(double x, double y, bool additional) :
    DxfEntity(ENTITY_TYPE::POINT),
    m_flags({0,0,1}),
    m_pt(fpoint_t(x, y))
{
    m_additional = additional;
}
DxfPoint::DxfPoint(const DxfPoint &other) : DxfEntity(other, ENTITY_TYPE::POINT), m_flags(other.m_flags), m_pt(other.m_pt) {}
DxfPoint::~DxfPoint() {}

DxfEntity *DxfPoint::clone() const { return new DxfPoint(*this); }

void DxfPoint::setX(double value) {
    m_flags.valid = 0;
    m_flags.X = 1;
    m_pt.x = value;
}

void DxfPoint::setY(double value) {
    m_flags.valid = 0;
    m_flags.Y = 1;
    m_pt.y = value;
}

void DxfPoint::set(const fpoint_t& point, bool additional) {
    m_flags = {0,0,1};
    m_additional = additional;
    m_pt = point;
}

double DxfPoint::X() const { return m_pt.x; }
double DxfPoint::Y() const { return m_pt.y; }
bool DxfPoint::isX() const { return m_flags.X; }
bool DxfPoint::isY() const { return m_flags.Y; }

bool DxfPoint::check() {
    if (!m_flags.valid && m_flags.X && m_flags.Y) {
        m_flags = {0,0,1};
        m_additional = false;
    }

    return m_flags.valid;
}

void DxfPoint::clear() {
    DxfEntity::clear();
    m_flags = {0,0,0};
    m_additional = false;
    m_pt = fpoint_t();
}

void DxfPoint::flipX(double x) {
    m_pt.flipX(x);
}

void DxfPoint::flipY(double y) {
    m_pt.flipY(y);
}

void DxfPoint::scale(double k) {
    m_pt.scale(k);
}

string DxfPoint::toString() const {
    return m_flags.valid ? "Point: " + m_pt.toString() : "Not valid";
}

string DxfPoint::toString2() const {
    return m_flags.valid ? "Point: " + m_pt.toString() : "Not valid";
}

ContourRange DxfPoint::range() const { return ContourRange(m_pt.x, m_pt.x, m_pt.y, m_pt.y); }
