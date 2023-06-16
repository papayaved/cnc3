#ifndef SEGMENT_ENTITY_H
#define SEGMENT_ENTITY_H

#include <cstdint>
#include <string>
#include <deque>
#include <vector>
#include <QColor>
#include "fpoint_t.h"
#include "main.h"
#include "cnc_types.h"
#include "contour_range.h"

enum class DXF_ENTITY_TYPE : int8_t { NONE = -1, LINE, ARC, CIRCLE, POINT, INSERT };
enum class DIRECTION : uint8_t { Q1, Q2, Q3, Q4 };

bool almost_same(DIRECTION dir1, DIRECTION dir2);

class ISegmentObject {
protected:
    ISegmentObject();
public:
    virtual ~ISegmentObject() = 0;
    virtual fpoint_t point_0() const = 0;
    virtual fpoint_t point_1() const = 0;
    virtual fpoint_t center() const = 0;
    virtual double radius() const { return 0; }
    virtual double tangent_0() const { return 0; }
    virtual double tangent_1() const { return 0; }
    virtual double length() const { return 0; }
    virtual double length(const fpoint_t&) const { return 0; }
    virtual double dx() const { return 0; }
    virtual double dy() const { return 0; }
    virtual std::vector<fpoint_t> getPoints() const { return std::vector<fpoint_t>(); }
    virtual bool isPoint() const = 0;
    virtual bool valid() const { return false; }
//    virtual void clear() {}
    virtual void change_0(const fpoint_t&) {}
    virtual void change_1(const fpoint_t&) {}

    virtual ContourRange range() const = 0;
//    virtual DIRECTIONS direction_0() { return DIRECTIONS::Q1; }
//    virtual DIRECTIONS direction_1() { return DIRECTIONS::Q1; }
};

class SegmentEntity : public ISegmentObject {
protected:
    constexpr static const double M_PRECISION = CncParam::PRECISION; // mm
    DXF_ENTITY_TYPE m_type {DXF_ENTITY_TYPE::NONE};
    bool m_additional {false};

    struct {
        bool valid;
        uint8_t value;
    } m_roll_vel {false, 0};

    struct {
        bool valid;
        uint16_t value;
    } m_pause {false, 0};

    struct {
        bool valid;
        double value; // mm/min
    } m_speed {false, 0};

    std::string m_layer = std::string();

    SegmentEntity(DXF_ENTITY_TYPE type = DXF_ENTITY_TYPE::NONE);
    SegmentEntity(const SegmentEntity& other, DXF_ENTITY_TYPE type);

public:
//    CUT_TYPES cut_type;
//    bool highlight;

    virtual ~SegmentEntity() override = 0;

    virtual SegmentEntity* clone() const = 0; // parameters are lost

    virtual SegmentEntity& operator=(const SegmentEntity& other);

    virtual bool operator==(const SegmentEntity& /*other*/) const { return false; }
    virtual bool operator!=(const SegmentEntity& /*other*/) const { return false; }
    virtual bool same(const SegmentEntity& /*other*/) const { return false; }

    virtual bool check() { return false; }

    virtual void clear() {
        m_layer = std::string();
    }

    virtual SegmentEntity* trim_front(double head_length, bool rem = false);
    virtual SegmentEntity* trim_front_rev(double tail_length, bool rem = false);
    virtual SegmentEntity* trim_back(double tail_length, bool rem = false);
    virtual SegmentEntity* trim_back_rev(double head_length, bool rem = false);
    virtual void offset(OFFSET_SIDE /*side*/, double /*offset*/, const SegmentEntity* /*prev*/, const SegmentEntity* /*next*/) {}
    virtual void offset(OFFSET_SIDE /*side*/, double /*offset*/) {}

    virtual void rotate(const RotateMatrix& /*mx*/) {}
    virtual void flipX(double /*x*/) {}
    virtual void flipY(double /*y*/) {}
    virtual void scale(double /*k*/) {}

    DXF_ENTITY_TYPE type() const;
    std::string typeString() const;

    void setLayerName(const std::string& layer_name) {
        m_layer = layer_name;
    }
    std::string layerName() const { return m_layer; }

    virtual std::string blockName() const;

    virtual void shift(const fpoint_t& value) = 0;
    virtual void reverse() = 0;    

    virtual std::string toString() const = 0;
    virtual std::string toString2() const = 0;

    virtual double length() const override { return 0; }
    virtual double length(const fpoint_t&) const override { return 0; }

    virtual void markAdditional(bool value = true) { m_additional = value; }
    virtual bool isAdditional() const { return m_additional; }

    virtual bool CCW() const { return 0; }

    void setSpeed(double value) {
        m_speed.value = value;
        m_speed.valid = true;
    }
    double speed() const { return m_speed.valid ? m_speed.value : 0; }
    void invalidateSpeed() { m_speed.valid = false; }
    bool hasSpeed() const { return m_speed.valid; }

    void setRollVel(uint value) {
        m_roll_vel.value = value <= UINT8_MAX ? value : UINT8_MAX;
        m_roll_vel.valid = true;
    }
    uint rollVel() const { return m_roll_vel.valid ? m_roll_vel.value : 0; }
    void invalidateRollVel() { m_roll_vel.valid = false; }
    bool hasRollVel() const { return m_roll_vel.valid; }

    void setPause(uint value) {
        m_pause.value = value <= UINT16_MAX ? value : UINT16_MAX;
        m_pause.valid = m_pause.value != 0;
    }
    uint pause() const { return m_pause.valid ? m_pause.value : 0; }
    void invalidatePause() { m_pause.valid = false; }
    bool hasPause() const { return m_pause.valid; }

    static double lengthSquare(const fpoint_t& A, const fpoint_t& B);
    static double length(const fpoint_t& A, const fpoint_t& B);
    static DIRECTION dir(double dx, double dy);
    static DIRECTION dir(const fpoint_t& A, const fpoint_t& B);
    static bool almost_same_dir(const fpoint_t& base, const fpoint_t& A, const fpoint_t& B);
};

#endif // SEGMENT_ENTITY_H
