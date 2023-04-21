#ifndef DXF_POINT_H
#define DXF_POINT_H

#include "dxf_entity.h"

class DxfPoint : public DxfEntity {
    struct {
        uint8_t X:1;
        uint8_t Y:1;
        uint8_t valid:1;
    } m_flags;

    fpoint_t m_pt;

public:
    DxfPoint(bool additional = false);
    DxfPoint(const fpoint_t& pt, bool additional = false);
    DxfPoint(double x, double y, bool additional = false);
    DxfPoint(const DxfPoint& other);
    ~DxfPoint() override;

    DxfEntity* clone() const override;

    bool operator==(const DxfEntity &other) const override {
        return m_type == other.type() && m_pt == other.point_0();
    }

    bool operator!=(const DxfEntity &other) const override {
        return !(*this == other);
    }

    void setX(double value);
    void setY(double value);
    void set(const fpoint_t& point, bool additional = false);
    double X() const;
    double Y() const;
    bool isX() const;
    bool isY() const;

    bool check() override;
    bool valid() const override { return m_flags.valid; }
    void clear() override;

    fpoint_t point_0() const override { return m_pt; }
    fpoint_t point_1() const override { return m_pt; }
    fpoint_t center() const override { return m_pt; }
    std::vector<fpoint_t> getPoints() const override { return std::vector<fpoint_t>({m_pt}); }
    bool isPoint() const override { return true; }

    void shift(const fpoint_t& value) override { m_pt.shift(value); }
    void reverse() override {}

    void flipX(double x) override;
    void flipY(double y) override;
    void scale(double k) override;

    std::string toString() const override;
    std::string toString2() const override;

    ContourRange range() const override;
};

#endif // DXF_POINT_H
