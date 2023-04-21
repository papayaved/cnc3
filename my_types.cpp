#include "my_types.h"

OFFSET_SIDE operator~(const OFFSET_SIDE &side) {
    return side == OFFSET_SIDE::LEFT ? OFFSET_SIDE::RIGHT : OFFSET_SIDE::LEFT;
}

offset_t::offset_t(uint8_t mode_id, double offset) : mode_id(mode_id), offset(offset)  {}

DIR operator-(DIR dir) {
    return dir == DIR::DIR_MINUS ? DIR::DIR_PLUS : DIR::DIR_MINUS;
}

AXIS swap(AXIS axis) {
    return axis == AXIS::AXIS_Y ? AXIS::AXIS_X : AXIS::AXIS_Y;
}
