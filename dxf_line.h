#ifndef DXF_LINE_H
#define DXF_LINE_H

#include "dxf_entity.h"

class DxfLine : public DxfEntity {
    struct flags_t {
        uint8_t X0:1;
        uint8_t Y0:1;
        uint8_t X1:1;
        uint8_t Y1:1;
        uint8_t valid:1;
    } m_flags = {0,0,0,0,false};

    fpoint_t m_A, m_B;

public:
    DxfLine();
    DxfLine(const fpoint_t& A, const fpoint_t& B, bool additional = false);
    DxfLine(const double Ax, const double Ay, const double Bx, const double By);
    DxfLine(double len, AXIS axis, DIR dir);
    DxfLine(const DxfLine& other);
    ~DxfLine() override;
    DxfEntity* clone() const override;

    DxfLine& operator=(const DxfLine& other);
    virtual bool operator==(const DxfEntity& other) const override;
    virtual bool operator!=(const DxfEntity& other) const override;

    bool same(const DxfEntity& other) const override;

    void setX0(double value);
    void setY0(double value);
    void setX1(double value);
    void setY1(double value);

    void setPoint0(const fpoint_t& value);
    void setPoint1(const fpoint_t& value);
    void set(const fpoint_t& A, const fpoint_t& B, bool additional = false);

    fpoint_t point_0() const override;
    fpoint_t point_1() const override;
    static fpoint_t midpoint(const fpoint_t& A, const fpoint_t& B);
    fpoint_t center() const override;
    double tangent_0() const override;
    double tangent_1() const override;

    void change_0(const fpoint_t& A) override;
    void change_1(const fpoint_t& B) override;

    ContourRange range() const override;

    DIRECTION dir() const;
    bool otherDir180(const DxfLine& other) const;

    double length() const override;
    double length(const fpoint_t& pt) const override;
    double distance(const fpoint_t& pt) const;
    DxfEntity* trim_front(double head_length, bool rem = false) override;
    DxfEntity* trim_front_rev(double tail_length, bool rem = false) override;
    DxfEntity* trim_back(double tail_length, bool rem = false) override;
    DxfEntity* trim_back_rev(double head_length, bool rem = false) override;
    void offset(OFFSET_SIDE side, double offset, const DxfEntity* prev, const DxfEntity* next) override;
    void offset(OFFSET_SIDE side, double offset) override;

    double dx() const override;
    double dy() const override;

    double angle() const;
    double k() const;
    double b() const;
    static bool vertical(const fpoint_t& A, const fpoint_t& B);
    bool vertical() const;
    static bool horizontal(const fpoint_t& A, const fpoint_t& B);
    bool horizontal() const;
    void func(double& k, double& b) const;
    double y(double x) const;

    void clearWithoutLayer();
    void clear() override;
    void shift(const fpoint_t& value) override;
    void reverse() override;

    std::string toString() const override;
    std::string toString2() const override;

    virtual std::vector<fpoint_t> getPoints() const override;
    virtual bool isPoint() const override;
    bool check() override;
    bool empty();

    void rotate(const RotateMatrix& mx) override;
    void flipX(double x) override;
    void flipY(double y) override;
    void scale(double k) override;
};

#endif // DXF_LINE_H
