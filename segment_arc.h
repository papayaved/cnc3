#ifndef SEGMENT_ARC_H
#define SEGMENT_ARC_H

#include "segment_entity.h"

class SegmentArc : public SegmentEntity {
protected:
    static const double ANGLE_STEP;

    struct {
        uint8_t Cx:1;
        uint8_t Cy:1;
        uint8_t R:1;
        uint8_t alpha:1;
        uint8_t beta:1;
        uint8_t valid:1;
        uint8_t circle:1;
        uint8_t ccw:1;
    } m_flags;
    double m_R, m_alpha, m_beta; // start & end angles, radius
    fpoint_t m_C;

public:
    SegmentArc();
    SegmentArc(const fpoint_t& center, double radius, double start_angle, double end_angle, bool ccw = true);
    SegmentArc(const fpoint_t& A, const fpoint_t& B, const fpoint_t& C, bool ccw);
    SegmentArc(const fpoint_t& A, const fpoint_t& B, double R, bool ccw);
    SegmentArc(const SegmentArc& other);
    ~SegmentArc() override;
    SegmentEntity* clone() const override;

    bool operator==(const SegmentEntity& other) const override;
    bool operator!=(const SegmentEntity& other) const override;

    bool same(const SegmentEntity& other) const override;
    bool inside(const SegmentArc& other) const;
    bool outside(const SegmentArc& other) const;
    bool insideCircle(const fpoint_t& pt) const;
    bool outsideCircle(const fpoint_t& pt) const;
    bool intersected(const SegmentArc& other) const;

    virtual void setCenterX(double value);
    virtual void setCenterY(double value);
    virtual void setRadius(double value);
    virtual void setStartAngle(double degree);
    void setEndAngle(double degree);
    virtual void setExtrusionDirectionZ(double value);

    bool check() override;
    void clear() override;

    bool isPoint() const override;
    virtual bool isCircle() const;
    std::vector<fpoint_t> getPoints() const override;

    void set(const fpoint_t& center, double radius, double start_angle, double end_angle, bool ccw = true);
    void set(const fpoint_t& A, const fpoint_t& B, const fpoint_t& center, bool ccw = true);
    void set(const fpoint_t& A, const fpoint_t& B, double radius, bool ccw = true);

    virtual double startAngle() const { return m_alpha; }
    inline double endAngle() const { return m_beta; }
    virtual bool CCW() const override { return m_flags.ccw; }

    virtual double point_X0() const;
    virtual double point_Y0() const;
    virtual double point_X1() const;
    virtual double point_Y1() const;

    fpoint_t point_0() const override;
    fpoint_t point_1() const override;
    fpoint_t center() const override { return m_C; }
    double radius() const override;
    double tangent_0() const override;
    double tangent_1() const override;

    ContourRange range() const override;

    double length() const override;
    double length(const fpoint_t& pt) const override;
    double distance(const fpoint_t& pt) const;
    SegmentEntity* trim_front(double length, bool rem = false) override;
    SegmentEntity* trim_front_rev(double length, bool rem = false) override;
    SegmentEntity* trim_back(double length, bool rem = false) override;
    SegmentEntity* trim_back_rev(double length, bool rem = false) override;
    void offset(OFFSET_SIDE side, double offset, const SegmentEntity* prev, const SegmentEntity* next) override;
    void offset(OFFSET_SIDE side, double offset) override;

    void rotate(const RotateMatrix& mx) override;
    void flipX(double x) override;
    void flipY(double y) override;
    void scale(double k) override;

    double dx() const override;
    double dy() const override;

    void shift(const fpoint_t& value) override;
    void reverse() override;

    std::string toString() const override;
    std::string toString2() const override;

    bool between(double angle) const;

    static double range360(double value);
    static bool between(double angle, double alpha, double beta, bool ccw);
    static void polar(const fpoint_t& center, const fpoint_t& point, double& radius, double& angle);
    static double deltaAngle(bool ccw, double alpha, double beta);
    virtual double deltaAngle() const;
};

#endif // SEGMENT_ARC_H
