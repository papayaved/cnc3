#ifndef DXF_CIRCLE_H
#define DXF_CIRCLE_H

#include "dxf_arc.h"

class DxfCircle : public DxfArc {
public:
    DxfCircle();
    DxfCircle(const fpoint_t& center, double radius, double alpha = 0, bool ccw = true);
    DxfCircle(const DxfCircle& other);
    ~DxfCircle() override;
    DxfEntity* clone() const override;

    void setStartAngle(double degree) override;

    bool check() override;
    void clear() override;

    std::vector<fpoint_t> getPoints() const override;

    void set(const fpoint_t& center, double radius, double start_angle = 0, bool ccw = true);
    void set(const fpoint_t& A, const fpoint_t& center, bool ccw = true);

    double length() const override;
    void offset(OFFSET_SIDE side, double offset, const DxfEntity* prev, const DxfEntity* next) override;

    bool isPoint() const override;
    bool isCircle() const override;

    void shift(const fpoint_t& value) override;
    void reverse() override;

    std::string toString() const override;
    std::string toString2() const override;

    double deltaAngle() const override;
};

#endif // DXF_CIRCLE_H
