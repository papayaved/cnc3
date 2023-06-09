#ifndef MY_TYPES_H
#define MY_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include "main.h"

enum class AXIS : uint8_t {AXIS_X, AXIS_Y, AXIS_NUM};
enum class DIR : uint8_t {DIR_PLUS, DIR_MINUS};
enum class SLICING_PROFILE : uint8_t { RECT, ROUND };

DIR operator-(DIR dir);
AXIS swap(AXIS axis);

namespace cnc_param {
#ifdef STONE
    constexpr static const double WIRE_DIAMETER = 0.33; // 0.4
    constexpr static const double SPEED = 1.8; // mm / min
#else
    constexpr static const double WIRE_DIAMETER = 0.18; // 0.2
    constexpr static const double SPEED = 3.6; // mm / min
#endif
    constexpr static const double SPEED_MAX = 18; // mm / min

    constexpr static const uint8_t ROLL_MIN = 1;
    constexpr static const uint8_t ROLL_MAX = 7;
    constexpr static const uint8_t ROLL_DEFAULT = 7;

    constexpr static const uint8_t CURRENT_MIN = 0;
    constexpr static const uint8_t CURRENT_MAX = 11;
    constexpr static const uint8_t CURRENT_DEFAULT = 5;

    constexpr static const uint8_t PULSE_WIDTH_MIN = 2;
    constexpr static const uint8_t PULSE_WIDTH_MAX = 99;
    constexpr static const uint8_t PULSE_WIDTH_DEFAULT = 36;

    constexpr static const uint8_t PULSE_RATIO_MIN = 2;
    constexpr static const uint8_t PULSE_RATIO_MAX = 32;
    constexpr static const uint8_t PULSE_RATIO_DEFAULT = 8;

    constexpr static const double L_DEFAULT = 190;
    constexpr static const double H_DEFAULT = 50;
    constexpr static const double T_DEFAULT = 30;
    constexpr static const double D_DEFAULT = 29.5;
    constexpr static const AXIS D_AXIS_DEFAULT = AXIS::AXIS_Y;
    constexpr static const DIR D_WIRE_SIDE_DEFAULT = DIR::DIR_MINUS;
}

struct snake_t {
    SLICING_PROFILE profile {SLICING_PROFILE::RECT};
    AXIS axis {AXIS::AXIS_X};
    uint8_t roll_vel {cnc_param::ROLL_MAX}, roll_vel_idle {cnc_param::ROLL_MIN};
    bool pause_ena {false};
    unsigned slices_num {20}, sections_num {20}, pause {30};
    double wireD {cnc_param::WIRE_DIAMETER}, width {40.0}, spacing {2.0}, step {-1.0}, speed_avg {cnc_param::SPEED}, speed_idle {cnc_param::SPEED_MAX};
};

struct comb_t {
    AXIS axis {AXIS::AXIS_X};
    uint8_t roll_vel {cnc_param::ROLL_MAX}, roll_vel_idle {cnc_param::ROLL_MIN};
    bool pause_ena {false};
    unsigned slots_num {20}, pause {30};
    double wireD {cnc_param::WIRE_DIAMETER}, depth {40.0}, step {-1.0}, speed {cnc_param::SPEED}, speed_idle {cnc_param::SPEED_MAX};
};

enum class OFFSET_SIDE : uint8_t { LEFT, RIGHT };
OFFSET_SIDE operator~(const OFFSET_SIDE& side);

struct offset_t {
    uint8_t mode_id {0};
    double offset {0};

    offset_t(uint8_t mode_id = 0, double offset = 0);
};

template<typename T> struct data_valid_t {
    bool valid {false};
    T data {0};
};

#endif // MY_TYPES_H
