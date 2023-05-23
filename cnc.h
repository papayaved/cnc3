#ifndef CNC_H
#define CNC_H

#include <cfloat>
#include <string>
#include <deque>

#include "cnc_types.h"
#include "cnc_com.h"
#include "fpga.h"
#include "motor_record.h"
#include "aux_items.h"
#include "main.h"
#include "cnc_context.h"
#include "fpoint_t.h"
#include "cnc_param.h"
#include "gcode.h"

#define FPGA_CLOCK (72e6)

namespace ADDR {
    const uint32_t STATUS       = 0x00;
    const uint32_t CLEAR        = 0x01 << 2;

    const uint32_t CONTEXT      = 0x10 << 2;
    const uint32_t UV_ENABLE    = 0x10 << 2;
    const uint32_t GOTO         = 0x12 << 2;
    const uint32_t REC_X        = 0x13 << 2;
    const uint32_t REC_Y        = 0x14 << 2;
    const uint32_t REC_U        = 0x15 << 2;
    const uint32_t REC_V        = 0x16 << 2;
    const uint32_t REC_ENC_X    = 0x17 << 2;
    const uint32_t REC_ENC_Y    = 0x18 << 2;

    const uint32_t CONTROLS_ENABLE  = 0x20 << 2;
    const uint32_t ROLL_VEL         = 0x21 << 2;
    const uint32_t LOW_HV    = 0x22 << 2;
    const uint32_t CURRENT_INDEX    = 0x23 << 2;
    const uint32_t PULSE_WIDTH      = 0x24 << 2;
    const uint32_t PULSE_RATIO      = 0x25 << 2;
    const uint32_t SPEED            = 0x26 << 2;
    const uint32_t STEP             = 0x27 << 2;    
    const uint32_t SCALE_X          = 0x28 << 2;
    const uint32_t SCALE_Y          = 0x29 << 2;
    const uint32_t SCALE_U          = 0x2A << 2;
    const uint32_t SCALE_V          = 0x2B << 2;
    const uint32_t SCALE_ENC_X      = 0x2C << 2;
    const uint32_t SCALE_ENC_Y      = 0x2D << 2;
    const uint32_t BOOL_SET         = 0x2E << 2;

    const uint32_t INPUT_LEVEL  = 0x30 << 2;    
    const uint32_t MOTOR_DIR    = 0x31 << 2;
    const uint32_t SD_CTRL      = 0x32 << 2;
    const uint32_t ACC_ENA      = 0x33 << 2;
    const uint32_t ACC          = 0x34 << 2;
    const uint32_t DEC          = 0x35 << 2;

    const uint32_t FB_ENA       = 0x38 << 2;
    const uint32_t FB_ADC_THDL  = 0x39 << 2;
    const uint32_t RB_TIMEOUT   = 0x3A << 2;
    const uint32_t RB_ATTEMPTS  = 0x3B << 2;
    const uint32_t RB_LENGTH    = 0x3C << 2;
    const uint32_t RB_SPEED     = 0x3D << 2;
    const uint32_t FB_ACC       = 0x3E << 2;
    const uint32_t FB_DEC       = 0x3F << 2;

    const uint32_t SET_X        = 0x40 << 2;
    const uint32_t SET_Y        = 0x41 << 2;
    const uint32_t SET_U        = 0x42 << 2;
    const uint32_t SET_V        = 0x43 << 2;
    const uint32_t ENC_X        = 0x44 << 2;
    const uint32_t ENC_Y        = 0x45 << 2;

    const uint32_t MOVE_X       = 0x50 << 2;
    const uint32_t MOVE_Y       = 0x51 << 2;
    const uint32_t MOVE_U       = 0x52 << 2;
    const uint32_t MOVE_V       = 0x53 << 2;

    const uint32_t ADC          = 0x60 << 2;
    const size_t ADC_SIZE       = 6 * sizeof(uint16_t);

    const uint32_t CENTER_MODE      = 0x70 << 2;
    const uint32_t CENTER_THLD      = 0x71 << 2;
    const uint32_t CENTER_RADIUS    = 0x72 << 2;
    const uint32_t CENTER_ROLLBACK  = 0x73 << 2;
    const uint32_t COARSE_SPEED     = 0x74 << 2;
    const uint32_t FINE_SPEED       = 0x75 << 2;
    const uint32_t CENTER_ANGLE0    = 0x76 << 2;
    const uint32_t CENTER_ANGLE1    = 0x77 << 2;
    const uint32_t CENTER_ANGLE2    = 0x78 << 2;
//    const uint32_t CENTER_ANGLE3    = 0x79 << 2;
    const uint32_t DIA              = 0x7a << 2;
    const uint32_t D_Y              = 0x7b << 2;

    const uint32_t UV_L              = 0x80 << 2;
    const uint32_t UV_H              = 0x81 << 2;
    const uint32_t UV_T              = 0x82 << 2;
    const uint32_t UV_D              = 0x83 << 2;
    const uint32_t UV_AXIS_ENA       = 0x84 << 2;

    const uint32_t VER_DATE     = 0xF0 << 2;
    const uint32_t VER_TIME     = 0xF4 << 2;
    const uint32_t SYS_CLOCK    = 0xF8 << 2;
    const uint32_t VER          = 0xF9 << 2;
    const uint32_t WDT          = 0xFA << 2;

    const uint32_t TEST_REG     = 0xFF << 2;

    const uint32_t PA_RDADDR    = 0x100 << 2;
    const uint32_t PA_WRADDR    = 0x101 << 2;
    const uint32_t PA_SIZE      = 0x102 << 2;
    const uint32_t IMIT_FIFO_COUNT  = 0x103 << 2;

    const uint32_t IMIT_FIFO_Q  = 0x110 << 2; // 3 * 32 bits

    const uint32_t BACKUP       = 0x200 << 2;

    const uint32_t PA           = 0x08000000;

    const uint32_t RUN_MASK     = 1U << 0;
    const uint32_t STOPPED_MASK = 1U << 1;
    const uint32_t PAUSE_MASK   = 1U << 2;
    const uint32_t ERROR_MASK   = 1U << 3;

    const uint32_t REV_MASK     = 1U << 8;
    const uint32_t IMIT_MASK    = 1U << 16;

    const uint32_t STOP_MASK    = 1U << 0;
    const uint32_t CANCEL_MASK   = 1U << 1;

    const uint32_t RESET_MASK   = 1U << 3;
    const uint32_t STATE_RESET_MASK   = 1U << 4;
}

struct point_t {
    int32_t x, y, u, v;
    inline std::string toString() const { return "(" + std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(u) + "," + std::to_string(v) + ")"; }
};

struct path_t {
    uint32_t x, y, u, v;
    inline std::string toString() const { return "(" + std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(u) + "," + std::to_string(v) + ")"; }
};

enum class CENTER_MODE_T {CENTER_NO, CENTER_X, CENTER_Y, CENTER_CIRCLE_4R, CENTER_CIRCLE_3ADJ};

class Cnc : public QObject {
    Q_OBJECT

    static constexpr int CENTER_ATTEMPTS_MAX = 5;
    static constexpr float CENTER_RADIUS_MAX = 200; // mm
    static constexpr double COE_UMSEC2_TO_MMTICK2 = 1e-3 / (Fpga::CLOCK * Fpga::CLOCK);
private:
    static CncCom m_com;
    std::deque<MotorRecord> m_imit_list;
    auxItems::Reporter* m_msg;

    std::vector<uint8_t> m_wrbuf;

public:
    Fpga::FpgaBus m_fpga;

    explicit Cnc(QObject* parent = nullptr);
    ~Cnc();

    void bindReporter(auxItems::Reporter* const msg = nullptr);
    void unbindReporter();

    bool open();
    inline bool isOpen() const { return m_com.isOpen(); }
    void close();
    inline CncCom& com() { return m_com; }
    void reconnect();

    bool stateClear();
    bool reset();

    void clear();

    bool writeArray(uint32_t addr, const std::vector<uint32_t>& bytes);

    bool writeProgArray(const std::vector<uint8_t>& bytes);
    bool writeProgArrayBurst(const std::vector<uint8_t>& bytes);

    uint32_t readProgArraySize();

    std::vector<uint8_t> readProgArray(size_t len);
    std::vector<uint8_t> readProgArray();

    std::vector<uint8_t> readProgArrayBurst(size_t len);
    std::vector<uint8_t> readProgArrayBurst();

    bool write(const std::list<std::string>& frames, size_t* write_size = nullptr, size_t* pa_size = nullptr);
    bool writeFromFile(const std::string& fileName = "test.nc");
    void printGCode();
    std::list<std::string> read();

    bool directMoveOn(int32_t nx, double scale_x, int32_t ny, double scale_y, int32_t nu, double scale_u, int32_t nv, double scale_v, double speed);
    bool directSetPos(int32_t nx, int32_t ny, int32_t nu, int32_t nv, int32_t enc_x, int32_t enc_y);
    bool centering(
            CENTER_MODE_T mode,
            int touches, int attempts, int rollVel, int thld, int fineSharePct, float R, float rollback,
            float speedCoarse, float speedFine,
            float angle0, float angle1, float angle2
    );

    bool runReq();
    bool revReq();

    bool cancelReq();
    bool stopReq();

    bool isRun();

    cnc_context_t readCncContext();
    cnc_context_t readBackup();
    bool clearBackup();
    void initialContext(const cnc_context_t& ctx);

    cnc_adc_t readADC();
    cnc_adc_volt_t readADCVolt();

    double readDiameter();

    void imitEna(bool value);
    bool isImitEna();
    void readImitFifo();
    void saveImitData(std::string fileName = "motor_xy.dat");

    std::string versionDate();
    uint32_t sysClock();
    cnc_version_t version();
    cnc_wdt_t wdt();
    std::string readVersion();

    bool testRegs();
    bool testProgArray();
    bool testFpga();

    bool writeUInt32(uint32_t addr, uint32_t data);    
    bool writeFloat(uint32_t addr, float data);
    bool writeSetBits(uint32_t addr, unsigned bit, unsigned width, uint16_t data);

    uint32_t readUInt32(uint32_t addr, uint32_t defaultValue = 0, bool* pOK = nullptr);
    uint32_t readBit(uint32_t addr, unsigned bit, unsigned width = 1, uint32_t defaultValue = 0, bool* pOK = nullptr);

    bool writeEnableUV(bool ena);
    bool writeGoto(int32_t frame_num);
    bool writeXYUVEnc(int x, int y, int u, int v, int enc_x, int enc_y);

    bool writePumpEnable(bool ena);
    bool writeRollEnable(bool ena);
    bool writeWireEnable(bool ena);
    bool writeEnableHighVoltage(bool ena);
    bool writeHoldEnable(bool ena);
    bool writeWeakHVEnable(bool ena);
    bool writeSemaphoreEnable(bool ena);
    bool writeSemaphoreCncEnable(bool ena);
    bool writeCncEnable(bool ena);

    bool writeEncXYEna(bool ena);
    bool writeUVEna(bool ena);

    bool writeRollVel(unsigned value);
    bool writeEnableLowHighVolt(bool value);
    bool writeCurrentIndex(size_t index);
    bool writePulseWidth(unsigned value);
    bool writePulseRatio(unsigned value);
    bool writeSpeed(float value);
    bool writeSpeed(const WireSpeed& value);

    bool writeStep(float value);

    bool writeSettings(
            uint16_t input_lvl,
            bool sd_oe, bool sd_ena,
            bool rev_x, bool rev_y, bool rev_u, bool rev_v, bool swap_xy, bool swap_uv,
            bool rev_enc_x, bool rev_enc_y,
            bool acc_ena, double acc, double dec
        );
    bool writeStep(float step, float scale_x, float scale_y, float scale_u, float scale_v, float scale_enc_x, float scale_enc_y, bool encXY);

    bool readSettings(
            uint16_t& input_lvl,
            bool& sd_oe, bool& sd_ena,
            bool& rev_x, bool& rev_y, bool& rev_u, bool& rev_v,
            bool& swap_xy, bool& swap_uv,
            bool& rev_enc_x, bool& rev_enc_y,
            bool& acc_ena, double& acc, double& dec
        );
    bool readStep(float& step, float& scaleX, float& scaleY, float& scaleU, float& scaleV, float& scaleEncX, float& scaleEncY, bool&encXY);

//    bool writeAdcThreshold(bool enable, uint16_t low, uint16_t high);
//    bool writeAdcThresholdVolt(bool enable, double Vlow, double Vhigh);
//    bool readAdcThreshold(bool& enable, uint16_t& low, uint16_t& high);
//    bool readAdcThresholdVolt(bool& enable, double& Vlow, double& Vhigh);

//    bool writeRollbackTimeout(double sec);
//    bool readRollbackTimeout(double& sec);

//    bool writeRollbackAttempts(uint32_t times);
//    bool readRollbackAttempts(uint32_t& times);

//    bool writeRollbackLength(double mm);
//    bool readRollbackLength(double& mm);

//    bool writeRollbackSpeed(double mmm);
//    bool readRollbackSpeed(double& mmm);

    bool writeFeedback(bool enable, double Vlow, double Vhigh, double to_sec, uint32_t attempts, double length, double speed, double fb_acc, double fb_dec);
    bool writeFeedback(bool enable, double Vlow, double Vhigh);
    bool readFeedback(bool& enable, double& Vlow, double& Vhigh, double& to_sec, uint32_t& attempts, double& length, double& speed, double& fb_acc, double& fb_dec);

    static double umsec2_to_mmtick2(double value) { return COE_UMSEC2_TO_MMTICK2 * value; }
    static double mmtick2_to_umsec2(double value) { return (1.0 / COE_UMSEC2_TO_MMTICK2) * value; }

    void recoveryUV(const GCodeSettings& s);

signals:
    void writeBurstProcessing(int pct);
    void readBurstProcessing(int pct);
    void error(const std::string& s);
};

#endif // CNC_H
