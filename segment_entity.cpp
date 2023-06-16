#include "segment_entity.h"
#include <cmath>

using namespace std;

ISegmentObject::ISegmentObject() {}
ISegmentObject::~ISegmentObject() {}

SegmentEntity::SegmentEntity(DXF_ENTITY_TYPE type) :
    m_type(type), m_additional(false), m_roll_vel {false, 0}, m_pause {false, 0}, m_speed {false, 0}, m_layer(std::string()) {}

SegmentEntity::SegmentEntity(const SegmentEntity& other, DXF_ENTITY_TYPE type) :
    m_type(type), m_additional(other.m_additional), m_roll_vel(other.m_roll_vel), m_pause(other.m_pause), m_speed(other.m_speed), m_layer(other.m_layer) {}

SegmentEntity &SegmentEntity::operator=(const SegmentEntity &other) {
    if (this != &other) {
        m_type = other.m_type;
        m_additional = other.m_additional;
        m_roll_vel = other.m_roll_vel;
        m_pause = other.m_pause;
        m_speed = other.m_speed;
        m_layer = other.m_layer;
    }
    return *this;
}

SegmentEntity::~SegmentEntity() {}

SegmentEntity *SegmentEntity::trim_front(double, bool) { return nullptr; }
SegmentEntity *SegmentEntity::trim_front_rev(double, bool) { return nullptr; }
SegmentEntity *SegmentEntity::trim_back(double, bool) { return nullptr; }
SegmentEntity *SegmentEntity::trim_back_rev(double, bool) { return nullptr; }

DXF_ENTITY_TYPE SegmentEntity::type() const { return m_type; }

string SegmentEntity::typeString() const {
    switch (m_type) {
    case DXF_ENTITY_TYPE::NONE: return "NONE";
    case DXF_ENTITY_TYPE::LINE: return "LINE";
    case DXF_ENTITY_TYPE::ARC: return "ARC";
    case DXF_ENTITY_TYPE::CIRCLE: return "CIRCLE";
    case DXF_ENTITY_TYPE::POINT: return "POINT";
    case DXF_ENTITY_TYPE::INSERT: return "INSERT";
    }

    return string();
}

string SegmentEntity::blockName() const { return string(); }

double SegmentEntity::lengthSquare(const fpoint_t& A, const fpoint_t& B) {
    double dx = B.x - A.x;
    double dy = B.y - A.y;
    return dx * dx + dy * dy;
}

double SegmentEntity::length(const fpoint_t& A, const fpoint_t& B) {
    return sqrt(lengthSquare(A, B));
}

DIRECTION SegmentEntity::dir(double dx, double dy) {
    if (dx >= 0)
        return dy >= 0 ? DIRECTION::Q1 : DIRECTION::Q4;
    else
        return dy >= 0 ? DIRECTION::Q2 : DIRECTION::Q3;
}

DIRECTION SegmentEntity::dir(const fpoint_t& A, const fpoint_t& B) {
    return dir(B.x - A.x, B.y - A.y);
}

bool almost_same(DIRECTION dir1, DIRECTION dir2) {
    return (dir1 == DIRECTION::Q1 && dir2 != DIRECTION::Q3) ||
           (dir1 == DIRECTION::Q2 && dir2 != DIRECTION::Q4) ||
           (dir1 == DIRECTION::Q3 && dir2 != DIRECTION::Q1) ||
           (dir1 == DIRECTION::Q4 && dir2 != DIRECTION::Q2);
}

bool SegmentEntity::almost_same_dir(const fpoint_t& base, const fpoint_t& A, const fpoint_t& B) {
    return almost_same(dir(base, A), dir(base, B));
}
