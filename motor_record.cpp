#include "motor_record.h"
#include "aux_items.h"
#include <string>

using namespace std;
using namespace auxItems;

MotorRecord::MotorRecord() {
    valid = false;
}

MotorRecord::~MotorRecord() {}

MotorRecord::MotorRecord(vector<uint8_t>& bytes, size_t* const pos) {
    if (*pos + MotorRecord::size > bytes.size())
        throw auxItems::string_format("Uncorrect data size %d pos %d", bytes.size(), *pos);

    uint32_t data =  BitConverter::toUInt32(bytes, *pos);
    *pos += sizeof(uint32_t);
    N = BitConverter::toInt32(bytes, *pos);
    *pos += sizeof(uint32_t);
    T = BitConverter::toUInt32(bytes, *pos);
    *pos += sizeof(uint32_t);

    valid = (data & 0x80000000) != 0;
    stop = (data & 0x40000000) != 0;
    mode = data >> 28 & 3;
    sync = (data & 0x08000000) != 0;
    axis = static_cast<AXIS>(data >> 24 & 7);
    cnt = data & 0xFFFFFF;
}

namespace local {
    string toString(const AXIS& axis) noexcept {
        return axis == AXIS::AXIS_X ? "X" : axis == AXIS::AXIS_Y ? "Y" : to_string(int(axis));
    }
}



string MotorRecord::toString() {
    switch (mode) {
        case 0:
            return string_format("%d %s N:%d T:%d (valid:%s, sync:%s, stop:%s)",
                cnt, local::toString(axis).c_str(), N, T, auxItems::toString(valid).c_str(), auxItems::toString(sync).c_str(), auxItems::toString(stop).c_str());
        default:
            return "Unknown mode";
    }
}
