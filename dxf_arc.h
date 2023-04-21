#ifndef DXF_ARC_H
#define DXF_ARC_H

#include "dxf_entity.h"

class DxfArc : public DxfEntity {
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
    } flags;
    double R, alpha, beta; // start & end angles, radius
    fpoint_t C;

public:
    DxfArc();
    DxfArc(const fpoint_t& center, double radius, double start_angle, double end_angle, bool ccw = true);
    DxfArc(const fpoint_t& A, const fpoint_t& B, const fpoint_t& C, bool ccw);
    DxfArc(const fpoint_t& A, const fpoint_t& B, double R, bool ccw);
    DxfArc(const DxfArc& other);
    ~DxfArc() override;
    DxfEntity* clone() const override;

    bool operator==(const DxfEntity& other) const override;
    bool operator!=(const DxfEntity& other) const override;

    bool same(const DxfEntity& other) const override;
    bool inside(const DxfArc& other) const;
    bool outside(const DxfArc& other) const;
    bool insideCircle(const fpoint_t& pt) const;
    bool outsideCircle(const fpoint_t& pt) const;
    bool intersected(const DxfArc& other) const;

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

    virtual double startAngle() const { return alpha; }
    inline double endAngle() const { return beta; }
    virtual bool CCW() const override { return flags.ccw; }

    virtual double point_X0() const;
    virtual double point_Y0() const;
    virtual double point_X1() const;
    virtual double point_Y1() const;

    fpoint_t point_0() const override;
    fpoint_t point_1() const override;
    fpoint_t center() const override { return C; }
    double radius() const override;
    double tangent_0() const override;
    double tangent_1() const override;

    ContourRange range() const override;

    double length() const override;
    double length(const fpoint_t& pt) const override;
    double distance(const fpoint_t& pt) const;
    DxfEntity* trim_front(double length, bool rem = false) override;
    DxfEntity* trim_front_rev(double length, bool rem = false) override;
    DxfEntity* trim_back(double length, bool rem = false) override;
    DxfEntity* trim_back_rev(double length, bool rem = false) override;
    void offset(OFFSET_SIDE side, double offset, const DxfEntity* prev, const DxfEntity* next) override;
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

#endif // DXF_ARC_H
