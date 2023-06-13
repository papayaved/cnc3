#include "cnc_context.h"
#include <cstdio>

void toDebug(const cnc_context_t * const ctx) {
    qDebug("pump:%x rollSt:%x wire:%x hv:%x hold:%x\n",
           ctx->field.pump_ena, ctx->field.roll_state, ctx->field.wire_ena, ctx->field.hv_enabled, ctx->field.hold_ena);
    qDebug("rollVel:%x\n",
           ctx->field.roll_vel);
    qDebug("uv:%x dia:%x tilted:%x rev:%x enc:%x\n",
           ctx->field.uv_ena, ctx->field.d_ena, ctx->field.tilted_ena, ctx->field.rev, ctx->field.enc_ena);
    qDebug("St:%x\n",
           ctx->field.state);
    qDebug("pW:%d pR:%d hvLvl:%x I:%x\n",
           ctx->field.pulse_width, ctx->field.pulse_ratio, ctx->field.low_hv_ena, ctx->field.current_index);
    qDebug("ID:%d X:%d Y:%d U:%d V:%d\n",
           (int)ctx->field.id, (int)ctx->field.x, (int)ctx->field.y, (int)ctx->field.u, (int)ctx->field.v);
    qDebug("encX:%d encY:%d\n",
           (int)ctx->field.enc_x, (int)ctx->field.enc_y);
    qDebug("T:%g\n",
           ctx->field.T);
    qDebug("T:%g\n",
           ctx->field.step);
    qDebug("bkp:%x read:%x\n",
           ctx->field.backup_valid, ctx->field.read_valid);
}

//const double cnc_adc_volt_t::coe[6] {
//    volt(1.0 / 61.04, 4.096, 10),
//    volt(1.0 / 111.08, 4.096, 10),
//    volt(1e6, 20e3, 1, 4.096, 10),
//    volt(1e6, 20e3, 1, 4.096, 10),
//    volt(1e6, 20e3, 1, 4.096, 10),
//    volt(10, 4.096, 10),
//};

//const double cnc_adc_volt_t::coe[6] {
//    volt(100e3, 3.01e3, 1, 4.096, 10), // diff
//    volt(10e3, 100e3, 10, 4.096, 10), // shunt
//    volt(100e3, 3.01e3, 1, 4.096, 10), // back
//    volt(100e3, 3.01e3, 1, 4.096, 10), // HV
//    volt(100e3, 3.01e3, 1, 4.096, 10), // Workpiece+
//    volt(100e3, 3.01e3, 1, 4.096, 10) // Wire-
//};

cnc_context_t CncContext::defaultContext() {
    cnc_context_t ctx;

    ctx.field.pump_ena = false;
    ctx.field.roll_state = static_cast<uint8_t>(roll_state_t::ROLL_DIS);
    ctx.field.wire_ena = false;
    ctx.field.hv_enabled = false;
    ctx.field.hold_ena = false;

    ctx.field.roll_vel = 0;
    ctx.field.tilted_ena = false;
    ctx.field.d_ena = false;

    ctx.field.uv_ena = false;
    ctx.field.enc_ena = false;
    ctx.field.rev = false;
    ctx.field.rollback = false;
    ctx.field.attempt = 0;
    ctx.field.acc_ena = false;

    ctx.field.state = static_cast<uint8_t>(cnc_state_t::ST_IDLE);

    ctx.field.pulse_width = 0;
    ctx.field.pulse_ratio = 0;
    ctx.field.low_hv_ena = 0;
    ctx.field.current_index = 0;

    ctx.field.id = -1;
    ctx.field.x = 0;
    ctx.field.y = 0;
    ctx.field.u = 0;
    ctx.field.v = 0;
    ctx.field.enc_x = 0;
    ctx.field.enc_y = 0;
    ctx.field.T = FLT_MAX;
    ctx.field.T_cur = FLT_MAX;
    ctx.field.step = STEP_MIN;

    ctx.field.limsw_fwd = 0;
    ctx.field.limsw_rev = 0;
    ctx.field.limsw_alm = 0;
    ctx.field.wire_break = 0;
    ctx.field.pwr = 0;
    ctx.field.fb_stop = 0;
    ctx.field.fb_to = 0;
    ctx.field.hv_enabled = 0;

    ctx.field.sem_ena = false;
    ctx.field.sem = 0;
    ctx.field.fb_ena = false;
    ctx.field.attempts = 0;

    ctx.field.backup_valid = false;

    return ctx;
}

cnc_context_t CncContext::parse(const std::vector<uint8_t> &v) {
    using namespace auxItems;
    cnc_context_t ctx;

    if (v.size() >= SIZE) {
        ctx.field.pump_ena = (v[0] & 1) != 0;
        ctx.field.roll_state = static_cast<uint8_t>(toRollState(v[0]>>1 & 3));
        ctx.field.wire_ena = (v[0] & 8) != 0;

        ctx.field.voltage_ena = (v[0] & 0x10) != 0;
        ctx.field.hold_ena = (v[0] & 0x20) != 0;
        ctx.field.center_ena = (v[0] & 0x40) != 0;
        ctx.field.is_init = (v[0] & 0x80) != 0;

        ctx.field.roll_vel  = v[1] & 0x3F;
        ctx.field.tilted_ena = (v[1] & 0x40) != 0;
        ctx.field.d_ena     = (v[1] & 0x80) != 0;

        ctx.field.uv_ena    = (v[2] & 1) != 0;
        ctx.field.enc_ena  = (v[2] & 2) != 0;
        ctx.field.rev       = (v[2] & 4) != 0;
        ctx.field.rollback  = (v[2] & 8) != 0;
        ctx.field.attempt   = v[2]>>4 & 7;
        ctx.field.acc_ena   = (v[2] & 0x80) != 0;

        ctx.field.state = static_cast<uint8_t>(toCncState(v[3]));

        ctx.field.pulse_width = v[4];
        ctx.field.pulse_ratio = v[5];
        ctx.field.low_hv_ena = v[6] & 1;
        ctx.field.current_index = v[7];

        ctx.field.id = BitConverter::toInt32(v, 2 * sizeof(uint32_t));
        ctx.field.x = BitConverter::toInt32(v, 3 * sizeof(uint32_t));
        ctx.field.y = BitConverter::toInt32(v, 4 * sizeof(uint32_t));
        ctx.field.u = BitConverter::toInt32(v, 5 * sizeof(uint32_t));
        ctx.field.v = BitConverter::toInt32(v, 6 * sizeof(uint32_t));
        ctx.field.enc_x = BitConverter::toInt32(v, 7 * sizeof(uint32_t));
        ctx.field.enc_y = BitConverter::toInt32(v, 8 * sizeof(uint32_t));
        ctx.field.T = BitConverter::toFloat(v, 9 * sizeof(uint32_t));
        ctx.field.T_cur = BitConverter::toFloat(v, 10 * sizeof(uint32_t));
        ctx.field.step = BitConverter::toFloat(v, 11 * sizeof(uint32_t));

        ctx.field.limsw_fwd     = (v[48] & 1) != 0;
        ctx.field.limsw_rev     = (v[48] & 2) != 0;
        ctx.field.limsw_alm     = (v[48] & 4) != 0;
        ctx.field.wire_break    = (v[48] & 8) != 0;
        ctx.field.pwr           = (v[48] & 0x10) != 0;
        ctx.field.fb_stop       = (v[48] & 0x20) != 0;
        ctx.field.fb_to         = (v[48] & 0x40) != 0;
        ctx.field.hv_enabled    = (v[48] & 0x80) != 0;

        ctx.field.sem_ena       = (v[49] & 1) != 0;
        ctx.field.sem           = v[49]>>1 & 7;
        ctx.field.fb_ena        = (v[49] & 0x10) != 0;
        ctx.field.attempts      = v[49]>>5 & 7;

        ctx.field.center_state  = v[50] & 7;
        ctx.field.touch_state   = v[50]>>3 & 7;
        ctx.field.center_mode   = v[50]>>6 & 3;

        ctx.field.center_attempt    = v[51];

        ctx.field.backup_valid      = (v[52] & 1) != 0;
        ctx.field.center_attempts   = v[53];
        ctx.field.touch             = v[54];
        ctx.field.touches           = v[55];

//        if (v.size() >= BACKUP_SIZE)
//            ctx.field.backup_valid = (v[BACKUP_SIZE - sizeof(uint32_t)] & 1) != 0;
//        else
//            ctx.field.backup_valid = false;
    }
    else
        ctx.field.backup_valid = false;

    return ctx;
}

cnc_context_t CncContext::parse(const QByteArray &ar) {
    cnc_context_t ctx;

    if (ar.size() == sizeof(cnc_context_t))
        memcpy(ctx.bytes, ar.data(), sizeof(cnc_context_t));
    else
        ctx = CncContext::defaultContext();

    return ctx;
}

std::string CncContext::toStringCenterDebug(double dia) const {
    cnc_state_t state = static_cast<cnc_state_t>(m_context.field.state);
    center_state_t center_state = static_cast<center_state_t>(m_context.field.center_state);
    touch_state_t touch_state = static_cast<touch_state_t>(m_context.field.touch_state);
    FPGA_CENTER_MODE_T center_mode = static_cast<FPGA_CENTER_MODE_T>(m_context.field.center_mode);

    std::string s = "State: "       + stateToString(state) + "\t Center: " + stateToString(center_state) + "\n" +
            "Touch: " + stateToString(touch_state) + "\t Mode: " + stateToString(center_mode) + "\n" +
            "Feedback: "    + (m_context.field.fb_ena ? "(Yes)" : "(no) ") + " " +
            "Weak HV: "     + (m_context.field.center_ena ? "(Yes)" : "(no) ") + " " +
            "Attempt: "     + std::to_string(m_context.field.center_attempt) + " (" + std::to_string(m_context.field.center_attempts) + ")" + " " +
            "Touch: "       + std::to_string(m_context.field.touch) + " (" + std::to_string(m_context.field.touches) + ")";
    if (dia >= 0) {
        char buf[100];

        dia = round(dia * 1e3) / 1e3;
        snprintf(buf, sizeof(buf), "Diameter: %.3f mm", dia);

        s += "\n";
        s += buf;
    }

    return s;
}
