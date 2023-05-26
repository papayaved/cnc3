#ifndef GFRAME_H
#define GFRAME_H

#include <string>
#include <vector>
#include <list>
#include "double_ext.h"
#include "fpoint_t.h"

enum class GEntityType : uint8_t { NONE, G, M, X, Y, U, V, I, J, R, F, P, Q };
enum class AXIS_ENA : uint8_t {NO_AXIS, XY_AXIS, UV_AXIS};

class GEntity {
    GEntityType m_type;
    double m_code;

public:
    GEntity(GEntityType type, double value);
    GEntity(char type, double value);

    void inc(double value) { m_code += value; }

    inline GEntityType type() const { return m_type; }
    inline double code() const { return m_code; }
    inline uint8_t codeUInt8() const { return m_code < 0 ? 0 : m_code > UINT8_MAX ? UINT8_MAX : static_cast<uint8_t>(m_code); }

    std::string toString() const;

    static std::string writeString(char ch, uint8_t value);
    static std::string writeString(char ch, double value);
//    static std::string writeString(std::string s, uint8_t value);
//    static std::string writeString(std::string s, double value);
};

enum class GCommandType : uint8_t { EMPTY, G, M, PRAGMA };

union GCommandValid {
    uint16_t data;
    struct {
        uint32_t G:1;
        uint32_t X:1; // P
        uint32_t Y:1; // Q
        uint32_t I:1; // R
        uint32_t J:1;
        uint32_t R:1;

        uint32_t G2:1;
        uint32_t U:1;
        uint32_t V:1;
        uint32_t I2:1; // R2
        uint32_t J2:1;
        uint32_t R2:1;

        uint32_t M:1;
        uint32_t P:1;
        uint32_t Q:1;
        uint32_t F:1;

        uint32_t PCT:1;
        uint32_t EoF:1; // End of Frame
    } flags;
};

struct GCommand {
    GCommandType m_type;
    uint8_t m_G, m_G2;
    GCommandValid m_valid;
    double m_X, m_Y, m_I, m_J, m_U, m_V, m_I2, m_J2, m_F;

public:
    GCommand() : m_type(GCommandType::EMPTY), m_valid({0}) {}

    void clear() {
        m_type = GCommandType::EMPTY;
        m_valid = {0};
    }
    inline bool empty() const { return m_valid.data == 0; }

    static bool isGMValid(double G) { return G >= 0 && G <= UINT8_MAX; }
    static bool hasGCoord(int G) { return (G >= 0 && G <= 3) || G == 92; }

    inline bool setG(double value) {
        if (isGMValid(value) && (m_type == GCommandType::EMPTY || m_type == GCommandType::G) && !m_valid.flags.G) {
            m_type = GCommandType::G;
            m_valid.flags.G = 1;
            m_G = static_cast<uint8_t>(value);
            return true;
        }
        return false;
    }

    inline bool setG2(double value) {
        if (isGMValid(value) && (m_type == GCommandType::EMPTY || m_type == GCommandType::G) && !m_valid.flags.G2) {
            m_type = GCommandType::G;
            m_valid.flags.G2 = 1;
            m_G2 = static_cast<uint8_t>(value);
            return true;
        }
        return false;
    }

    inline bool setM(double value) {
        if (isGMValid(value) && m_type == GCommandType::EMPTY) {
            m_type = GCommandType::M;
            m_valid.flags.M = 1;
            m_G = static_cast<uint8_t>(value);
            return true;
        }
        return false;
    }

    inline bool setX(double value) {
        if (m_type == GCommandType::EMPTY || m_type == GCommandType::G) {
            m_valid.flags.X = 1;
            m_X = value;
            return true;
        }
        return false;
    }
    inline bool setY(double value) {
        if (m_type == GCommandType::EMPTY || m_type == GCommandType::G) {
            m_valid.flags.Y = 1;
            m_Y = value;
            return true;
        }
        return false;
    }

    inline bool setP(double value) {
        if (m_type == GCommandType::EMPTY || m_type == GCommandType::M) {
            m_valid.flags.P = 1;
            m_X = value;
            return true;
        }
        return false;
    }
    inline bool setQ(double value) {
        if (m_type == GCommandType::EMPTY || m_type == GCommandType::M) {
            m_valid.flags.Q = 1;
            m_Y = value;
            return true;
        }
        return false;
    }
    inline bool setF(double value) {
        if (m_type == GCommandType::EMPTY || m_type == GCommandType::G) {
            m_valid.flags.F = 1;
            m_F = value;
            return true;
        }
        return false;
    }

    inline bool setI(double value) {
        if (!m_valid.flags.R) {
            m_valid.flags.I = 1;
            m_I = value;
            return true;
        }
        return false;
    }
    inline bool setJ(double value) {
        if (!m_valid.flags.R) {
            m_valid.flags.J = 1;
            m_J = value;
            return true;
        }
        return false;
    }
    inline bool setR(double value) {
        if (!m_valid.flags.I && !m_valid.flags.J) {
            m_valid.flags.R = 1;
            m_I = value;
            return true;
        }
        return false;
    }

    inline void setU(double value) { m_valid.flags.U = 1; m_U = value; }
    inline void setV(double value) { m_valid.flags.V = 1; m_V = value; }

    inline bool setI2(double value) {
        if (!m_valid.flags.R2) {
            m_valid.flags.I2 = 1;
            m_I2 = value;
            return true;
        }
        return false;
    }
    inline bool setJ2(double value) {
        if (!m_valid.flags.R2) {
            m_valid.flags.J2 = 1;
            m_J2 = value;
            return true;
        }
        return false;
    }
    inline bool setR2(double value) {
        if (!m_valid.flags.I2 && !m_valid.flags.J2) {
            m_valid.flags.R2 = 1;
            m_I2 = value;
            return true;
        }
        return false;
    }

    inline void setEOF() {
        m_valid.flags.EoF = 1;
    }

    inline GCommandType type() const { return m_type; }

    inline uint8_t M() const { return m_G; }
    inline double P() const { return m_X; }
    inline double Q() const { return m_Y; }
    inline double F() const { return m_F; }

    inline uint8_t G() const { return m_G; }
    inline double X() const { return m_X; }
    inline double Y() const { return m_Y; }
    inline double I() const { return m_I; }    
    inline double J() const { return m_J; }
    inline double R() const { return m_I; }

    inline uint8_t G2() const { return m_G2; }
    inline double U() const { return m_U; }
    inline double V() const { return m_V; }
    inline double I2() const { return m_I2; }
    inline double J2() const { return m_J2; }
    inline double R2() const { return m_I2; }

    inline bool isG() const { return m_valid.flags.G; }
    inline bool isX() const { return m_valid.flags.X; }
    inline bool isY() const { return m_valid.flags.Y; }
    inline bool isI() const { return m_valid.flags.I; }
    inline bool isJ() const { return m_valid.flags.J; }
    inline bool isR() const { return m_valid.flags.R; }

    inline bool isG2() const { return m_valid.flags.G2; }
    inline bool isU() const { return m_valid.flags.U; }
    inline bool isV() const { return m_valid.flags.V; }    
    inline bool isI2() const { return m_valid.flags.I2; }    
    inline bool isJ2() const { return m_valid.flags.J2; }    
    inline bool isR2() const { return m_valid.flags.R2; }

    inline bool isM() const { return m_valid.flags.M; }
    inline bool isP() const { return m_valid.flags.P; }
    inline bool isQ() const { return m_valid.flags.Q; }
    inline bool isF() const { return m_valid.flags.F; }

    inline bool isEOF() const { return m_valid.flags.EoF; }

    inline bool isXY() const { return m_valid.flags.X || m_valid.flags.Y || m_valid.flags.I || m_valid.flags.J || m_valid.flags.R; }
    inline bool isUV() const { return m_valid.flags.U || m_valid.flags.V || m_valid.flags.I2 || m_valid.flags.J2 || m_valid.flags.R2; }
    inline bool isXYUV() const { return isXY() || isUV(); }
};

class GFrame {
public:
    enum class PragmaType: uint8_t {NONE, DELIMITER, CUTLINE_BEGIN, CUTLINE_BODY, CUTLINE_END};

    PragmaType m_pragma;
    std::vector<GEntity> m_entities;
    std::string m_comment;

    GFrame();
    GFrame(PragmaType type) : m_pragma(type) {}
    GFrame(const GEntity& entity, std::string comment = "") :
        m_pragma(PragmaType::NONE),
        m_entities({entity}),
        m_comment(comment) {}
    GFrame(const GEntity& entity0, const GEntity& entity1, std::string comment = "") :
        m_pragma(PragmaType::NONE),
        m_entities({entity0, entity1}),
        m_comment(comment) {}
    GFrame(const GEntity& entity0, const GEntity& entity1, const GEntity& entity2, std::string comment = "") :
        m_pragma(PragmaType::NONE),
        m_entities({entity0, entity1, entity2}),
        m_comment(comment) {}
    GFrame(const GEntity& entity0, const GEntity& entity1, const GEntity& entity2, const GEntity& entity3, std::string comment = "") :
        m_pragma(PragmaType::NONE),
        m_entities({entity0, entity1, entity2, entity3}),
        m_comment(comment) {}

    ~GFrame(); // destructor prevents implicit move assignment
    GFrame(const GFrame& other);
    GFrame(GFrame&& other) noexcept;

    inline const std::vector<GEntity>& entities() const { return m_entities; }

    GFrame& operator=(const GFrame& other);
    GFrame& operator=(GFrame&& other) noexcept;    

    static GFrame XY(double x, double y) { return GFrame(GEntity('X', x), GEntity('Y', y)); }
    static GFrame UV(double u, double v) { return GFrame(GEntity('U', u), GEntity('V', v)); }
    static GFrame XYUV(double x, double y, double u, double v) { return GFrame(GEntity('X', x), GEntity('Y', y), GEntity('U', u), GEntity('V', v)); }

    static GFrame XYR(double x, double y, double r) { return GFrame(GEntity('X', x), GEntity('Y', y), GEntity('R', r)); }
    static GFrame UVR(double u, double v, double r) { return GFrame(GEntity('U', u), GEntity('V', v), GEntity('R', r)); }

    static GFrame XYIJ(double x, double y, double i, double j) { return GFrame(GEntity('X', x), GEntity('Y', y), GEntity('I', i), GEntity('J', j)); }
    static GFrame UVIJ(double u, double v, double i, double j) { return GFrame(GEntity('U', u), GEntity('V', v), GEntity('I', i), GEntity('J', j)); }

    static GFrame F(double f) { return GFrame(GEntity('F', f)); }

    static GFrame G0() { return GFrame(GEntity('G', uint8_t(0))); }
    static GFrame G0(double x, double y) {
        GFrame res = G0();
        res.append(XY(x,y));
        return res;
    }
    static GFrame G0(double x, double y, double u, double v) {
        GFrame res = G0();
        res.append(XYUV(x,y,u,v));
        return res;
    }

    static GFrame G1() { return GFrame(GEntity('G', uint8_t(1))); }
    static GFrame G1(double x, double y) {
        GFrame res = G1();
        res.append(XY(x,y));
        return res;
    }
    static GFrame G1UV(double u, double v) {
        GFrame res = G1();
        res.append(UV(u,v));
        return res;
    }
    static GFrame G1(double x, double y, double u, double v) {
        GFrame res = G1();
        res.append(XY(x,y));
        res.append(G1());
        res.append(UV(u,v));
        return res;
    }

    static GFrame G2() { return GFrame(GEntity('G', uint8_t(2))); }
    static GFrame G2(double x, double y, double i, double j) {
        GFrame res = G2();
        res.append(XYIJ(x,y,i,j));
        return res;
    }
    static GFrame G2UV(double u, double v, double i, double j) {
        GFrame res = G2();
        res.append(UVIJ(u,v,i,j));
        return res;
    }
    static GFrame G2(double x, double y, double i, double j, double u, double v, double i2, double j2) {
        GFrame res = G2();
        res.append(XYIJ(x,y,i,j));
        res.append(G2());
        res.append(UVIJ(u,v,i2,j2));
        return res;
    }
    static GFrame G2(double x, double y, double r) {
        GFrame res = G2();
        res.append(XYR(x,y,r));
        return res;
    }
    static GFrame G2UV(double u, double v, double r) {
        GFrame res = G2();
        res.append(UVR(u,v,r));
        return res;
    }

    static GFrame G3() { return GFrame(GEntity('G', uint8_t(3))); }
    static GFrame G3(double x, double y, double i, double j) {
        GFrame res = G3();
        res.append(XYIJ(x,y,i,j));
        return res;
    }
    static GFrame G3UV(double u, double v, double i, double j) {
        GFrame res = G3();
        res.append(UVIJ(u,v,i,j));
        return res;
    }
    static GFrame G3(double x, double y, double r) {
        GFrame res = G3();
        res.append(XYR(x,y,r));
        return res;
    }
    static GFrame G3UV(double u, double v, double r) {
        GFrame res = G3();
        res.append(UVR(u,v,r));
        return res;
    }

    // delay, ms
    static GFrame G4(uint32_t delay) { return GFrame(GEntity('G', uint8_t(4)), GEntity('P', delay)); }

    // pause, wait when button press
    static GFrame M0() { return GFrame(GEntity('M', uint8_t(0)), "Pause"); }

    // end of program
    static GFrame M2() { return GFrame(GEntity('M', uint8_t(2))); }

    // pump enable
    static GFrame M40() { return GFrame(GEntity('M', uint8_t(40)), "Pump On"); }

    // pump disable
    static GFrame M41() { return GFrame(GEntity('M', uint8_t(41)), "Pump Off"); }

    // roll enable
    static GFrame M82() { return GFrame(GEntity('M', uint8_t(82)), "Roll On"); }

    // roll disable
    static GFrame M83() { return GFrame(GEntity('M', uint8_t(83)), "Roll Off"); }

    // voltage enable
    static GFrame M84() { return GFrame(GEntity('M', uint8_t(84)), "HV On"); }

    // voltage disable
    static GFrame M85() { return GFrame(GEntity('M', uint8_t(85)), "HV Off"); }

    // set position
    static GFrame G92() { return GFrame(GEntity('G', uint8_t(92))); }
    static GFrame G92(double x, double y) {
        GFrame gframe = G92();
        gframe.m_entities.push_back(GEntity('X', x));
        gframe.m_entities.push_back(GEntity('Y', y));
        return gframe;
    }
    static GFrame G92(double x, double y, double u, double v) {
        GFrame gframe = G92();
        gframe.m_entities.push_back(GEntity('X', x));
        gframe.m_entities.push_back(GEntity('Y', y));
        gframe.m_entities.push_back(GEntity('U', u));
        gframe.m_entities.push_back(GEntity('V', v));
        return gframe;
    }

    // apply
//    static GFrame M100() { return GFrame(GEntity('M', uint8_t(100)), "Apply"); }

    // hold off
//    static GFrame M101() { return GFrame(GEntity('M', uint8_t(101)), "Hold off"); }

    // Second contour settings
    static GFrame M100(float L_rollers, float H_bot) {
        return GFrame(GEntity('M', uint8_t(100)), GEntity('P', double(L_rollers)), GEntity('Q', double(H_bot)), "L, H");
    }

    // Workpeace thickness
    static GFrame M101(float Thickness) {
        return GFrame(GEntity('M', uint8_t(101)), GEntity('P', double(Thickness)), "T");
    }

    // Use the roller diameter
    static GFrame M102(float rollerD, AXIS rollerAxis) {
        return GFrame(GEntity('M', uint8_t(102)), GEntity('P', double(rollerD)), GEntity('Q', uint8_t(rollerAxis == AXIS::AXIS_Y ? 1 : 0)), "D, Axis");
    }

    // roll velocity
    static GFrame M105(float value) { return GFrame(GEntity('M', uint8_t(105)), GEntity('P', double(value)), "Velocity"); }

    // current
    static GFrame M106(float value) { return GFrame(GEntity('M', uint8_t(106)), GEntity('P', double(value)), "I"); }

    // pulse
    static GFrame M107(float width, float ratio) { return GFrame(GEntity('M', uint8_t(107)), GEntity('P', double(width)), GEntity('Q', double(ratio)), "W, R"); }

    // Low High-Voltage Enable
    static GFrame M108() { return GFrame( GEntity('M', uint8_t(108)), "Low HV On" ); }

    // Low High-Voltage Disable
    static GFrame M109() { return GFrame( GEntity('M', uint8_t(109)), "Low HV Off" ); }

    // Voltage thresholds, FB enable
    static GFrame M110(float low_thld, float high_thld) {
        return GFrame(GEntity('M', uint8_t(110)), GEntity('P', double(low_thld)), GEntity('Q', double(high_thld)), "FB On");
    }

    // programs delimiter: symbol %
    static GFrame delimiter() { return GFrame(PragmaType::DELIMITER); }

    // comment with tag <cutline>
    static GFrame cutlineBegin() { return GFrame(PragmaType::CUTLINE_BEGIN); }

    // comment with tag </cutline>
    static GFrame cutlineEnd() { return GFrame(PragmaType::CUTLINE_END); }

    // disable voltage, roll, pump
    static GFrame disable() {
#if defined(STONE)
        return GFrame(GEntity('M', uint8_t(83)), GEntity('M', uint8_t(41)), "disable");
#else
        return GFrame(GEntity('M', uint8_t(83)), GEntity('M', uint8_t(85)), GEntity('M', uint8_t(41)), "disable");
#endif
    }

    // enable voltage, roll, pump
    static GFrame enable() {
#if defined(STONE)
        return GFrame(GEntity('M', uint8_t(82)), GEntity('M', uint8_t(40)), "enable");
#else
        return GFrame(GEntity('M', uint8_t(82)), GEntity('M', uint8_t(84)), GEntity('M', uint8_t(40)), "enable");
#endif
    }

    void push_back(const GEntity& entity);
    void append(const GFrame& frame);

    void clear();
    bool empty() const;
    std::string toString() const;

    const GFrame& removeComment();
//    GFrame noComment() const;

    void offset(double X, double Y, double U, double V);
    void removeG(uint8_t value);

    void replaceRtoIJ(bool XY, const fpoint_t& center);

    static GFrame parse(std::string line, bool& err, const GFrame& prev = GFrame());

    static std::string cutComment(std::string& line);
    static void removeAllSpaces(std::string& str);
    static PragmaType toPragma(std::string& m_comment);

//    const double* find(GEntity::CommandType type) const;

//    int G() const;
//    const double* X() const;
//    const double* Y() const;
//    const double* U() const;
//    const double* V() const;

    std::list<GCommand> get(int& G, AXIS_ENA& axis_ena) const;

    bool any(const GEntityType type, const std::list<uint8_t>& codes) const;

    enum class PLANE_T{PLANE_UNKNOWN, PLANE_XY, PLANE_XYUV};
    PLANE_T getPlane() const;
};

#endif // GFRAME_H
