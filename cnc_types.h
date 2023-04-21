#ifndef CNC_TYPES_H
#define CNC_TYPES_H

#include <cstdint>
#include <vector>
#include <QObject>

#include "wire_cut.h"
#include "generator_mode_list.h"

enum class TAB_MODE : uint8_t { TAB_NONE, TAB_SINGLE, TAB_MULTI };

enum class cnc_state_t: uint8_t {ST_IDLE, ST_DIRECT, ST_CENTER, ST_READ, ST_SEGMENT, ST_WAIT, ST_PAUSE, ST_STOP, ST_WAIT_BUTTON, ST_END, ST_ERROR};
enum class center_state_t: uint8_t {ST_IDLE, ST_HV, ST_TOUCH, ST_CENTER, ST_BACK, ST_WAIT};
enum class touch_state_t:uint8_t {ST_IDLE, ST_FWD, ST_FWD_FINE0, ST_FWD_FINE1, ST_RB, ST_WAIT};
enum class FPGA_CENTER_MODE_T : uint8_t {FPGA_CENTER_MODE_OFF, FPGA_CENTER_MODE_FORWARD, FPGA_CENTER_MODE_REVERSE};
enum class roll_state_t: uint8_t {ROLL_DIS, ROLL_FWD, ROLL_REV, ROLL_ANY, ROLL_ERROR};

std::string stateToString(const cnc_state_t& state);
std::string stateToString(const center_state_t& state);
std::string stateToString(const touch_state_t& state);
std::string stateToString(const FPGA_CENTER_MODE_T& state);

class WireSpeed {
public:
    enum class Mode: bool {UMS, MMM}; // mm/min, um/sec

    static constexpr double FPGA_FREQ = 72e6; // Hz
    static constexpr double MIN = 0.01; // mm/min
    static constexpr double MAX = 18; // mm/min

private:
    double m_value; // mm/min
    Mode m_mode;

public:
    WireSpeed(double value = MAX, WireSpeed::Mode mode = WireSpeed::Mode::MMM) {
        m_mode = mode;
        set(value);
    }

    WireSpeed(const WireSpeed& other) {
        *this = other;
    }

    WireSpeed& operator=(const WireSpeed& other) {
        if (this == &other)
            return *this;

        m_value = other.m_value;
        m_mode = other.m_mode;
        return *this;
    }

    void changeMode(WireSpeed::Mode value) { m_mode = value; }
    WireSpeed::Mode mode() { return m_mode; }

    void set(double value) { m_value = (m_mode == Mode::MMM) ? value : toMMM(value); }
    void set(double value, WireSpeed::Mode mode) {
        m_mode = mode;
        set(value);
    }
    double get() const {
        double res = (m_mode == Mode::MMM) ? m_value : toUMS(m_value);

        if (res < min())
            return min();
        else if (res > max())
            return max();

        return res;
    }

    void setMMM(double value) { m_value = value; }
    double getMMM() const {
        if (m_value < MIN)
            return MIN;
        else if (m_value > MAX)
            return MAX;

        return m_value;
    }

    // result - mm/min
    // T - clock/mm
    static double TtoSpeed(float T) { return (FPGA_FREQ * 60.0) / double(T); }
    void setClockPerMM(float T) { m_value = TtoSpeed(T); }

    float getClockPerMM() const { return float( (FPGA_FREQ * 60.0) / m_value ); }

    double min() const { return m_mode == Mode::MMM ? MIN : toUMS(MIN); }
    double max() const { return m_mode == Mode::MMM ? MAX : toUMS(MAX); }

    static double toUMS(double value) { return value * (1000.0 / 60.0); }
    static double toMMM(double value) { return value * (60.0 / 1000.0); }
    static float toMMM(float value) { return value * (60.0f / 1000.0f); }
};

#endif // CNC_TYPES_H
