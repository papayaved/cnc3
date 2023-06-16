#ifndef SEGMENT_CIRCLE_H
#define SEGMENT_CIRCLE_H

#include "segment_arc.h"

class SegmentCircle : public SegmentArc {
public:
    SegmentCircle();
    SegmentCircle(const fpoint_t& center, double radius, double alpha = 0, bool ccw = true);
    SegmentCircle(const SegmentCircle& other);
    ~SegmentCircle() override;
    SegmentEntity* clone() const override;

    void setStartAngle(double degree) override;

    bool check() override;
    void clear() override;

    std::vector<fpoint_t> getPoints() const override;

    void set(const fpoint_t& center, double radius, double start_angle = 0, bool ccw = true);
    void set(const fpoint_t& A, const fpoint_t& center, bool ccw = true);

    double length() const override;
    void offset(OFFSET_SIDE side, double offset, const SegmentEntity* prev, const SegmentEntity* next) override;

    bool isPoint() const override;
    bool isCircle() const override;

    void shift(const fpoint_t& value) override;
    void reverse() override;

    std::string toString() const override;
    std::string toString2() const override;

    double deltaAngle() const override;
};

#endif // SEGMENT_CIRCLE_H
