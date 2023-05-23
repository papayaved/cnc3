#ifndef CNCCONTEXT_H
#define CNCCONTEXT_H

#include <cstdint>
#include <cfloat>

#include <QVariant>
#include "cnc_types.h"
#include "aux_items.h"
#include "amp.h"
#include "fpoint_t.h"

#define CNC_CONTEX_SIZE32 (14)
#define CNC_BACKUP_SIZE32 (CNC_CONTEX_SIZE32)

union cnc_context_t {
    uint32_t data[CNC_BACKUP_SIZE32];
    uint8_t bytes[CNC_BACKUP_SIZE32 * sizeof(uint32_t)];

    struct {
        // 0
        uint32_t pump_ena:1;
        uint32_t roll_state:2;
        uint32_t wire_ena:1;
        uint32_t high_voltage_ena:1;
        uint32_t hold_ena:1;
        uint32_t center_ena:1;
        uint32_t is_init:1;

        uint32_t roll_vel:7;
        uint32_t dia_ena:1;

        uint32_t uv_ena:1;
        uint32_t enc_ena:1;
        uint32_t rev:1;
        uint32_t rollback:1;
        uint32_t attempt:3;
        uint32_t acc_ena:1;

        uint32_t state:8;
        // 1
        uint32_t pulse_width:8;
        uint32_t pulse_ratio:8;
        uint32_t low_high_voltage_ena:1;
        uint32_t :7;
        uint32_t current_index:8;
        // 2, 3, 4, 5, 6
        int32_t id, x, y, u, v;
        // 7, 8
        int32_t enc_x, enc_y;
        // 9, 10
        float T, T_cur; // clocks/mm
        // 11
        float step; // mm
        // 12
        uint32_t limsw_fwd:1;
        uint32_t limsw_rev:1;
        uint32_t limsw_alm:1;
        uint32_t wire_break:1;
        uint32_t pwr:1;
        uint32_t fb_stop:1;
        uint32_t fb_to:1;
        uint32_t hv_ena:1;

        uint32_t sem_ena:1;
        uint32_t sem:3;
        uint32_t fb_ena:1;
        uint32_t attempts:3;

        uint32_t center_state:3;
        uint32_t touch_state:3;
        uint32_t center_mode:2;
        uint32_t center_attempt:8;
        // 13
        uint32_t backup_valid:1; // only for backup
        uint32_t read_valid:1;
        uint32_t :6;
        uint32_t center_attempts:8;
        uint32_t touch:8;
        uint32_t touches:8;
    } field;

    cnc_context_t() : data{0} { }
};

Q_DECLARE_METATYPE(cnc_context_t) // for QVariant

void toDebug(const cnc_context_t* const ctx);

struct CncLimitSwitches {
    uint8_t limsw_fwd:1;
    uint8_t limsw_rev:1;
    uint8_t limsw_alm:1;
    uint8_t wire_break:1;
    uint8_t pwr:1;
    uint8_t fb:1;
    uint8_t fb_to:1;
    uint8_t res:1;
};

struct CncLedSemaphore {
    uint8_t sem:3;
    uint8_t sem_ena:1;
    uint8_t :4;
};

class CncContext {
    cnc_context_t m_context;

public:
    static constexpr size_t SIZE32 = CNC_CONTEX_SIZE32;
    static constexpr size_t BACKUP_SIZE32 = CNC_BACKUP_SIZE32;
    static constexpr size_t SIZE = CNC_CONTEX_SIZE32 * sizeof(uint32_t);
    static constexpr size_t BACKUP_SIZE = CNC_BACKUP_SIZE32 * sizeof(uint32_t);
    static constexpr float STEP_MIN  = 0.001f; // mm
    static constexpr float STEP_MAX  = 1.0f; // mm

    static cnc_context_t defaultContext();

    CncContext() { m_context = defaultContext(); }
    void setDefault() { m_context = defaultContext(); }

    static cnc_state_t toCncState(uint8_t value) {
        if (value > static_cast<uint8_t>(cnc_state_t::ST_ERROR))
            return cnc_state_t::ST_ERROR;

        return static_cast<cnc_state_t>(value);
    }

    static roll_state_t toRollState(uint8_t value) {
        if (value > static_cast<uint8_t>(roll_state_t::ROLL_ERROR))
            return roll_state_t::ROLL_ERROR;

        return static_cast<roll_state_t>(value);
    }

    static cnc_context_t parse(const std::vector<uint8_t>& v);

    static cnc_context_t parse(const QByteArray& ar);

    // init from backup
    void set(const QByteArray& ar) {
        m_context = parse(ar);
    }

    QByteArray getByteArray() {
        QByteArray bytes(sizeof(cnc_context_t), 0);
        memcpy(bytes.data(), m_context.bytes, sizeof(cnc_context_t));
        return bytes;
    }

    void set(const cnc_context_t& ctx) { m_context = ctx; }
    CncContext(const cnc_context_t& ctx) { m_context = ctx; }
    CncContext(const std::vector<uint8_t>& v) { m_context = parse(v); }
    const cnc_context_t& get() const { return m_context; }

    bool pumpEnabled() const { return m_context.field.pump_ena; }
    roll_state_t rollState() const { return static_cast<roll_state_t>(m_context.field.roll_state); }
    bool wireControlEnabled() const { return m_context.field.wire_ena; }
    bool highVoltageEnabled() const { return m_context.field.high_voltage_ena; }
    bool hold() const { return m_context.field.hold_ena; }
    bool isInit() const { return m_context.field.is_init; }
    bool weakHV() const { return m_context.field.center_ena; }

    uint8_t rollVelocity() const { return m_context.field.roll_vel; }

    bool uvDiaEnabled() const { return m_context.field.dia_ena; }
    bool uvEnabled() const { return m_context.field.uv_ena; }
    bool reverse() const { return m_context.field.rev; }

    cnc_state_t cncState() const {
        return m_context.field.state > static_cast<uint8_t>(cnc_state_t::ST_ERROR) ? cnc_state_t::ST_ERROR : static_cast<cnc_state_t>(m_context.field.state);
    }

    uint8_t pulseWidth() const { return m_context.field.pulse_width; }
    uint8_t pulseRatio() const { return m_context.field.pulse_ratio; }
    bool lowHighVoltageEnabled() const { return m_context.field.low_high_voltage_ena; }
    uint8_t currentIndex() const { return m_context.field.current_index; }

    int32_t frameNum() const { return m_context.field.id; }
    int32_t x() const { return m_context.field.x; }
    int32_t y() const { return m_context.field.y; }
    int32_t u() const { return m_context.field.u; }
    int32_t v() const { return m_context.field.v; }
    int32_t encoderX() const { return m_context.field.enc_x; }
    int32_t encoderY() const { return m_context.field.enc_y; }

    bool isEncoderMode() const { return m_context.field.enc_ena; }

    CncLimitSwitches limitSwitches() const {
        CncLimitSwitches ls;
        ls.res = 0;

        ls.limsw_fwd = m_context.field.limsw_fwd;
        ls.limsw_rev = m_context.field.limsw_rev;
        ls.limsw_alm = m_context.field.limsw_alm;
        ls.wire_break = m_context.field.wire_break;
        ls.pwr = m_context.field.pwr;
        ls.fb = m_context.field.fb_stop;
        ls.fb_to = m_context.field.fb_to;

        return ls;
    }

    // mm/min
    double speed() const { return WireSpeed::TtoSpeed(m_context.field.T); }
    double step() const { return static_cast<double>(m_context.field.step); } // mm

    bool valid() const { return m_context.field.backup_valid; }
    void setValid(bool value) { m_context.field.backup_valid = value; }

    inline bool isWork() const {
        return m_context.field.state > static_cast<uint8_t>(cnc_state_t::ST_IDLE) && m_context.field.state < static_cast<uint8_t>(cnc_state_t::ST_ERROR);
    }
    inline bool isError() const {
        return m_context.field.state >= static_cast<uint8_t>(cnc_state_t::ST_ERROR);
    }
    inline bool isIdle() const {
        return m_context.field.state == static_cast<uint8_t>(cnc_state_t::ST_IDLE);
    }
    inline bool isDirect() const {
        return m_context.field.state == static_cast<uint8_t>(cnc_state_t::ST_DIRECT);
    }

    inline bool isRollEnable() const {
        return m_context.field.roll_state == static_cast<uint8_t>(roll_state_t::ROLL_FWD) || m_context.field.roll_state == static_cast<uint8_t>(roll_state_t::ROLL_REV);
    }

    inline void setRollEnable(bool value) {
        m_context.field.roll_state = value ? static_cast<uint8_t>(roll_state_t::ROLL_ANY) : static_cast<uint8_t>(roll_state_t::ROLL_DIS);
    }

    inline std::string toString() const {
        cnc_state_t state = static_cast<cnc_state_t>(m_context.field.state);
        center_state_t center_state = static_cast<center_state_t>(m_context.field.center_state);
        FPGA_CENTER_MODE_T center_mode = static_cast<FPGA_CENTER_MODE_T>(m_context.field.center_mode);

        return  "State:\t"      + stateToString(state) + " Center:\t" + stateToString(center_state) + "\tMode: " + stateToString(center_mode) + "\n" +
                "Frame Number: " + std::to_string(m_context.field.id) + "\n" +
                "Position: (" +  std::to_string(m_context.field.x) + ", " + std::to_string(m_context.field.y) + ", " +
                            std::to_string(m_context.field.u) + ", " + std::to_string(m_context.field.v) + ") " + "\n" +
                "Encoder: (" +  std::to_string(m_context.field.enc_x) + ", " + std::to_string(m_context.field.enc_y) + ")" + "\n" +
                "Pump Enable: " + std::to_string(m_context.field.pump_ena) + "\n" +
                "Roll State: " + std::to_string(m_context.field.roll_state) + "\n" +
                "Wire Break Control: " + std::to_string(m_context.field.wire_ena) + "\n" +
                "Voltage Enable: " + std::to_string(m_context.field.high_voltage_ena) + "\n" +
                "Motor Hold Enable: " + std::to_string(m_context.field.hold_ena) + "\n" +
                "Roll Velocity: " + std::to_string(m_context.field.roll_vel) + "\n" +
                "Pulse Width: " + std::to_string(m_context.field.pulse_width) + " Ratio: " + std::to_string(m_context.field.pulse_ratio) + "\n" +
                "Low High Voltage: " + (m_context.field.low_high_voltage_ena ? "Y" : "n") + "\n" +
                "Current Index: " + std::to_string(m_context.field.current_index) + "\n" +
                "Period : " + std::to_string(m_context.field.T) + "clock/mm\n" +
                "UV Enable: " + std::to_string(m_context.field.uv_ena) + " Dia Enable: " + std::to_string(m_context.field.dia_ena) + "\n" +
                "Valid: " + std::to_string(m_context.field.backup_valid) + "\n";
    }

    inline std::string toStringRunDebug() const {
        cnc_state_t state = static_cast<cnc_state_t>(m_context.field.state);

        return  "State: "     + stateToString(state) + "\n" +
                "Encoder "    + (m_context.field.enc_ena ? "(Yes)" : "(no)") + "\t" +
                "UV "         + (m_context.field.uv_ena ? "(Yes)" : "(no)") + "\t" +
                "Dia "        + (m_context.field.dia_ena ? "(Yes)" : "(no)") + "\n" +
                "Feedback "   + (m_context.field.fb_ena ? "(Yes)" : "(no)") + "\t" +
                "Reverse "    + (m_context.field.rev ? "(Yes)" : "(no)") + "\t" +
                "Rollback "   + (m_context.field.rollback ? "(Yes)" : "(no)") + "\n" +
                "HV Enabled " + (m_context.field.hv_ena ? "(Yes)" : "(no)") + "\t" +
                "HV & FB "    + (m_context.field.hv_ena && m_context.field.fb_stop ? "(Yes)" : "(no)") + "\t" +
                "Attempt:\t"  + std::to_string(m_context.field.attempt) + " (" + std::to_string(m_context.field.attempts) + ")" + "\n" +
                "Cur. speed:\t" + std::to_string( int(round(WireSpeed::toUMS(WireSpeed::TtoSpeed(m_context.field.T_cur)))) ) + " um/sec\n";
    }

    inline std::string toStringRunStoneDebug() const {
        cnc_state_t state = static_cast<cnc_state_t>(m_context.field.state);

        return  "State: "       + stateToString(state) + "\n" +
                "Reverse "      + (m_context.field.rev ? "(Yes)" : "(no)") + "\t" +
                "Acceleration " + (m_context.field.acc_ena ? "(Yes)" : "(no)") + "\n" +
                "Cur. speed:\t" + std::to_string( int(round(WireSpeed::toUMS(WireSpeed::TtoSpeed(m_context.field.T_cur)))) ) + " um/sec\n";
    }

    std::string toStringCenterDebug(double dia = -1) const;
};

struct adc_t {
    uint16_t value:15;
    uint16_t valid:1;
};

struct cnc_adc_t {
    static constexpr size_t ADC_NUM = 6;
    static constexpr size_t SIZE = ADC_NUM * sizeof(uint16_t);

    adc_t diff {0, false}, shunt {0, false}, inv_diff {0, false}, hv {0, false}, workpiece {0, false}, wire {0, false};

    static cnc_adc_t parse(const std::vector<uint8_t>& v) {
        using namespace auxItems;
        cnc_adc_t adc = cnc_adc_t();

        if (v.size() >= SIZE) {
            adc.diff.value = (uint16_t)(v[1] & 0x3)<<8 | v[0];
            adc.diff.valid = (v[1] & 4) == 0;

            adc.shunt.value = (uint16_t)(v[3] & 0x3)<<8 | v[2];
            adc.shunt.valid = (v[3] & 4) == 0;

            adc.inv_diff.value = (uint16_t)(v[5] & 0x3)<<8 | v[4];
            adc.inv_diff.valid = (v[5] & 4) == 0;

            adc.hv.value = (uint16_t)(v[7] & 0x3)<<8 | v[6];
            adc.hv.valid = (v[7] & 4) == 0;

            adc.workpiece.value = (uint16_t)(v[9] & 0x3)<<8 | v[8];
            adc.workpiece.valid = (v[9] & 4) == 0;

            adc.wire.value = (uint16_t)(v[11] & 0x3)<<8 | v[10];
            adc.wire.valid = (v[11] & 4) == 0;
        }

        return adc;
    }
};

struct double_valid_t {
    bool valid {false};
    double value {0};
};

struct cnc_adc_volt_t {
    constexpr static unsigned _ADC_WIDTH = 10; // bits
    constexpr static double Vref = 4.096; // V
    constexpr static double k_adc = Vref / (1<<_ADC_WIDTH);
    constexpr static double k_diff = 1.638 / 100;
    constexpr static double k_diff_inv = 1.0 / k_diff;

    constexpr static double Vdiod = 0.167; // V

    static constexpr double volt(double RH, double RL, double gain, double Vref, unsigned ADC_WIDTH = _ADC_WIDTH) {
        return Vref * (RH + RL) / ((1<<ADC_WIDTH) * RL * gain);
    }

    static constexpr double volt(double gain, double Vref, unsigned ADC_WIDTH = _ADC_WIDTH) {
        return Vref / ((1<<ADC_WIDTH) * gain);
    }

//    static const double coe[6];

    double_valid_t gap, rev_gap, workpiece, wire, hv, shunt;


    static double toVolt(size_t index, uint16_t code) {
        switch (index) {
        case 0: case 2: {
            double gain = amp::GainResDiv();
            return (code * k_adc + Vdiod) / gain;
        }
        case 1: {
            double gain = amp::GainResDiv(10e3, 100e3, 10e3, 100e3);
            return (code * k_adc + Vdiod) / gain;
        }
        case 3: case 4: case 5: {
            double gain = amp::GainResDiv();
            return code * k_adc / gain;
        }
        default:
            return 0;
        }
    }

    static double maxVolt(size_t index) {
        return toVolt(index, (1<<_ADC_WIDTH) - 1);
    }

    static uint16_t toCode(size_t index, double volt, size_t ADC_WIDTH = _ADC_WIDTH) {
        double value;

        if (volt > 0) {
            switch (index) {
            case 0: case 2:
                value = (volt * amp::GainResDiv() - Vdiod) / k_adc;
                break;
            case 1:
                value = (volt * amp::GainResDiv(10e3, 100e3, 10e3, 100e3) - Vdiod) / k_adc;
                break;
            case 3: case 4: case 5:
                value = volt * amp::GainResDiv() / k_adc;
                break;
            default:
                value = 0;
                break;
            }
        }
        else
            value = 0;

        uint16_t max_value;
        if (ADC_WIDTH >= 16)
            max_value = 0xffff;
        else
            max_value = static_cast<uint16_t>((1U<<ADC_WIDTH) - 1);

        value = round(value);

        if (value < 0)
            value = 0;
        else if (value > max_value)
            return max_value;

        return static_cast<uint16_t>(value);
    }

    cnc_adc_volt_t(const cnc_adc_t& adc) {
        gap.valid = adc.diff.valid;
        gap.value = gap.valid ? toVolt(0, adc.diff.value) : 0;

        shunt.valid = adc.shunt.valid;
        shunt.value = shunt.valid ? toVolt(1, adc.shunt.value) : 0;

        rev_gap.valid = adc.inv_diff.valid;
        rev_gap.value = rev_gap.valid ? toVolt(2, adc.inv_diff.value) : 0;

        hv.valid = adc.hv.valid;
        hv.value = hv.valid ? toVolt(3, adc.hv.value) : 0;

        workpiece.valid = adc.workpiece.valid;
        workpiece.value = workpiece.valid ? toVolt(4, adc.workpiece.value) : 0;

        wire.valid = adc.wire.valid;
        wire.value = wire.valid ? toVolt(5, adc.wire.value) : 0;
    }
};

#endif // CNCCONTEXT_H
