#include "cnc_types.h"

using namespace std;

// ST_IDLE, ST_DIRECT, ST_CENTER, ST_READ, ST_SEGMENT, ST_WAIT, ST_PAUSE, ST_STOP, ST_WAIT_BUTTON, ST_END, ST_ERROR
string stateToString(const cnc_state_t& state) {
    switch (state) {
    case cnc_state_t::ST_IDLE:
        return "ST_IDLE";
    case cnc_state_t::ST_DIRECT:
        return "ST_DIRECT";
    case cnc_state_t::ST_CENTER:
        return "ST_CENTER";
    case cnc_state_t::ST_READ:
        return "ST_READ";
    case cnc_state_t::ST_SEGMENT:
        return "ST_SEGMENT";
    case cnc_state_t::ST_WAIT:
        return "ST_WAIT";
    case cnc_state_t::ST_PAUSE:
        return "ST_PAUSE";
    case cnc_state_t::ST_STOP:
        return "ST_STOP";
    case cnc_state_t::ST_WAIT_BUTTON:
        return "ST_WAIT_BUTTON";
    case cnc_state_t::ST_END:
        return "ST_END";
    case cnc_state_t::ST_ERROR:
        return "ST_ERROR";
    default:
        return "ST_UNKNOWN";
    }
}

// ST_IDLE, ST_TOUCH, ST_CENTER, ST_BACK, ST_WAIT
string stateToString(const center_state_t& state) {
    switch (state) {
    case center_state_t::ST_IDLE:
        return "ST_IDLE";
    case center_state_t::ST_HV:
        return "ST_HV";
    case center_state_t::ST_TOUCH:
        return "ST_TOUCH";
    case center_state_t::ST_CENTER:
        return "ST_CENTER";
    case center_state_t::ST_BACK:
        return "ST_BACK";
    case center_state_t::ST_WAIT:
        return "ST_WAIT";
    default:
        return "ST_UNKNOWN";
    }
}

// ST_IDLE, ST_FWD, ST_FWD_FINE0, ST_FWD_FINE1, ST_RB
string stateToString(const touch_state_t& state) {
    switch (state) {
    case touch_state_t::ST_IDLE:
        return "ST_IDLE";
    case touch_state_t::ST_FWD:
        return "ST_FWD";
    case touch_state_t::ST_FWD_FINE0:
        return "ST_FWD_FINE0";
    case touch_state_t::ST_FWD_FINE1:
        return "ST_FWD_FINE1";
    case touch_state_t::ST_RB:
        return "ST_RB";
    case touch_state_t::ST_WAIT:
        return "ST_WAIT";
    default:
        return "ST_UNKNOWN";
    }
}

string stateToString(const FPGA_CENTER_MODE_T& state) {
    switch (state) {
    case FPGA_CENTER_MODE_T::FPGA_CENTER_MODE_OFF:
        return "OFF";
    case FPGA_CENTER_MODE_T::FPGA_CENTER_MODE_FORWARD:
        return "FORWARD";
    case FPGA_CENTER_MODE_T::FPGA_CENTER_MODE_REVERSE:
        return "REVERSE";
    default:
        return "UNKNOWN";
    }
}

uint8_t cut_t::getTimes() const {
    return times <= offsets.size() ? times : uint8_t(offsets.size());
}

vector<offset_t> cut_t::getOffsets() const {
    vector<offset_t> res(offsets);
    uint8_t times = getTimes();

    if (times <= res.size()) {
        res.resize(times);
        return res;
    }
    else
        return res;
}

vector<offset_t> cut_t::getTabOffsets() const {
    if (tab_multi_pass)
        return getOffsets();
    else {
        uint8_t times = getTimes();

        if (times & 1)
            return vector<offset_t>(1, tab_offset);
        else
            return vector<offset_t>(2, tab_offset);
    }
}

offset_t cut_t::getOvercutOffset() const {
    vector<offset_t> offsets = getOffsets();
    return !offsets.empty() ? getOffsets().back() : offset_t();
}

offset_t cut_t::getOutOffset() const {
    return getOvercutOffset();
}
