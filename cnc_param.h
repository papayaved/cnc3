#ifndef CNC_PARAM_H
#define CNC_PARAM_H

#include "main.h"
#include "my_types.h"

struct CncParam {
    static constexpr size_t AXES_NUM = 4;
    static constexpr size_t ENC_NUM = 2;

    static constexpr uint16_t INPUT_LEVEL_METAL = 0x001;
    static constexpr uint16_t INPUT_LEVEL_STONE = 0x010;
    static constexpr uint16_t INPUT_LEVEL_DEBUG = 0x300;

#ifdef STONE
    static constexpr uint16_t DEFAULT_INPUT_LEVEL = INPUT_LEVEL_STONE;
#else
    static constexpr uint16_t DEFAULT_INPUT_LEVEL = INPUT_LEVEL_METAL;
#endif

    static constexpr bool DEFAULT_SD_ENA = false;

    static constexpr double DEFAULT_STEP = 0.001; // mm
    static constexpr double PRECISION = DEFAULT_STEP / 2; // +-0.5 um
    static constexpr double STEP_MIN = 0.001; // mm
    static constexpr double STEP_MAX = 0.01; // mm

    static constexpr double DEFAULT_SCALE_XY = 1000; // steps/mm    
    static constexpr double DEFAULT_SCALE_UV = 1000; // steps/mm
    static constexpr double SCALE_MIN = 1000; // steps/mm
    static constexpr double SCALE_MAX = 1e6; // steps/mm

    static constexpr double ENCODERS_NUM = 2;
    static constexpr double DEFAULT_SCALE_ENC_XY = 200; // steps/mm
    static constexpr double SCALE_ENC_MIN = 200; // steps/mm
    static constexpr double SCALE_ENC_MAX = 1000; // steps/mm
    static constexpr bool DEFAULT_ENC_XY = false; // encoder enable

    static constexpr double DEFAULT_RB_TO = 30; // sec
    static constexpr unsigned DEFAULT_RB_ATTEMPTS = 3; // times
    static constexpr double DEFAULT_RB_LEN = 0.3; // mm
    static constexpr double DEFAULT_RB_SPEED = 1; // mm/min

    static constexpr bool DEFAULT_ACC_ENA = false;
    static constexpr double DEFAULT_ACC = 100;
    static constexpr double DEFAULT_DEC = 100;

    static uint16_t inputLevel;
    static bool sdEnable;
    static bool reverseX, reverseY, reverseU, reverseV, swapXY, swapUV, reverseEncX, reverseEncY;
    static double step; // mm
    static double scaleX, scaleY, scaleU, scaleV;
    static double scaleEncX, scaleEncY;
    static bool encXY;

    static bool fb_ena;
    static unsigned rb_attempts;
    static double low_thld[2], high_thld[2];
    static double rb_to, rb_len, rb_speed;

    static double fb_acc, fb_dec;

    static bool acc_ena;
    static double acc, dec;

    static int32_t mm_to_steps(double mm, double scale);
    static double steps_to_mm(int32_t steps, double scale);
    static double ums_to_mmm(double value);

    static double scale(size_t i);
    static double scaleEncoder(size_t i);

    static void reset();
};

#endif // CNC_PARAM_H
