#ifndef FPGA_H
#define FPGA_H

#include "aux_items.h"
#include "cnc_com.h"

// CNC version type that is used for FPGA version and MCU version
union cnc_version_t {
public:
    struct {
        uint32_t rev:7; // revision of version
        uint32_t stone:1; // label of stone version
        uint32_t ver:8; // software version
        uint32_t fac_rev:8; // factory/board revision
        uint32_t fac_ver:6; // factory/board version
        uint32_t type:2; // type of version 0 - stable, 1 - alpha, 2 - beta
    } fields;
    uint32_t data;

    cnc_version_t(uint32_t data) {
        this->data = data;
    }

    // Print version
    std::string toString() {
        return auxItems::string_format("%d.%d.%d.%d%c%s",
            static_cast<int>(fields.fac_ver),
            static_cast<int>(fields.fac_rev),
            static_cast<int>(fields.ver),
            static_cast<int>(fields.rev),
            fields.type == 0 ? ' ' : fields.type == 1 ? 'a' : fields.type == 2 ? 'b' : 'u',
            fields.stone ? (" (" + QObject::tr("stone").toStdString() + ")").c_str() : ""
        );
    }
};

struct cnc_wdt_t {
    uint8_t wdt:1;
    uint8_t swdt:1;
};

// Class provides direct access to FPGA. It's for debugging only
namespace  Fpga {
    const uint32_t BAR = 0x04000000; // basa address FPGA in MCU
    const unsigned MTR_NUM = 4; // number of motors

    static constexpr double CLOCK = 72e6; // Hz. Work frequency of FPGA
    const double PERIOD = 1.0 / CLOCK; // s

    const uint32_t MTR_BAR = 0; // base address of motor controller
    const uint32_t ENC_BAR = 0x100; // of encoders
    const uint32_t CTRL_BAR = 0x180; // of control registors
    const uint32_t ADC_BAR = 0x1C0; // of ADC

    const uint32_t NT32 = MTR_BAR; // step number, time
    const uint32_t MAIN_DIR = MTR_BAR + 0x40; // moving directions
    const uint16_t SWAP_XY_BIT = 8; // swap axes X and Y
    const uint16_t SWAP_UV_BIT = 9; // swap axe U and V
    const uint32_t MTR_OE = MTR_BAR + 0x42; // enable outputs of motors
    const uint32_t TASK_ID = MTR_BAR + 0x44; // number of frame
    const uint32_t MTR_WRREQ = MTR_BAR + 0x48; // command write request
    const uint32_t MTR_CONTROL = MTR_BAR + 0x4A; // control and status bits
    const uint16_t MTR_SNAPSHOT_BIT = 1<<2; // snapshop of a position and task_id
    const uint32_t TIMEOUT32 = MTR_BAR + 0x4C; // shortcut timeout duration

    const uint32_t POS32_DIST32 = MTR_BAR + 0x80; // possition and distance

    const unsigned ENC_NUM = 3; // number of encoders
    const uint32_t ENC_CONTROL = ENC_BAR; // control register

    const uint32_t ENC_SNAPSHOT_BIT = ENC_BAR + 8;
    const uint16_t ENC_CLEAR_BITS = 0xFF; // clear encoders bit field

    const uint32_t POS32_Z32 = ENC_BAR + 0x40; // encoder counters

    const uint32_t IRQ_FLAGS = CTRL_BAR;
    const uint32_t IRQ_MASK = CTRL_BAR + 2;
    const uint16_t FLAG_LIMSW_MASK = 1<<0;
    const uint16_t FLAG_TIMEOUT_MASK = 1<<1;

    const uint32_t RESET = CTRL_BAR + 4;

    const uint32_t LIMSW_MASK = CTRL_BAR + 8;
//    const uint16_t LIMSW_LEVEL = CTRL_BAR + 0xA;
    const uint32_t LIMSW = CTRL_BAR + 0xC;
    const uint32_t LIMSW_FLAG = CTRL_BAR + 0xE;

    const uint16_t LIMSW_FORWARD = 1<<0;
    const uint16_t LIMSW_REVERSE = 1<<1;
    const uint16_t LIMSW_ALARM = 1<<2;

    const uint32_t INPUT_LEVEL = CTRL_BAR + 0x10;

    const uint32_t DATO = CTRL_BAR + 0x10;
    const uint32_t DATO_OLD = CTRL_BAR + 0x18;
    const uint32_t DATI = CTRL_BAR + 0x20;

    const uint32_t LED16 = CTRL_BAR + 0x3A;
    const uint32_t VER32 = CTRL_BAR + 0x3C;

    const unsigned ADC_NUM = 5;
    const unsigned ADC_BITS = 10;
    const uint32_t ADC_SNAPSHOT = ADC_BAR;
    const uint32_t ADC = ADC_BAR;
    const uint32_t LOW_THLD = ADC_BAR + 0x10;
    const uint32_t HIGH_THLD = ADC_BAR + 0x12;
    const uint32_t ADC_ENA = ADC_BAR + 0x14;
    const uint32_t RUN_ENA = ADC_BAR + 0x16;
    const uint32_t PERMIT = ADC_BAR + 0x18;

    struct point_t { int32_t x, y; };

    struct status_t{
        uint32_t alarm:1;
        uint32_t roolback_req:1;
    };

    class FpgaBus: public QObject {
        Q_OBJECT

        CncCom& m_com;

    public:
        FpgaBus(CncCom& com);

        uint16_t read_u16(uint16_t addr);
        int16_t read_s16(uint16_t addr);
        uint32_t read_u32(uint16_t addr);
        int32_t read_s32(uint16_t addr);
        uint64_t read_u48(uint16_t addr);
        uint64_t read_u64(uint16_t addr);
        int64_t read_s64(uint16_t addr);

        void write_u16(uint16_t addr, uint16_t data);
        void write_s16(uint16_t addr, int16_t data);
        void write_u32(uint16_t addr, uint32_t data);
        void write_s32(uint16_t addr, int32_t data);
        void write_u48(uint16_t addr, uint64_t data);
        void write_u64(uint16_t addr, uint64_t data);
        void write_s64(uint16_t addr, int64_t data);

        // Motors
        void step(uint8_t i, int N, unsigned T);
        int32_t getN(uint8_t i);
        uint32_t getT(uint8_t i);

        void setOE(bool oe);
        bool getOE();
        uint16_t getRun();
        uint8_t getWrreq();
        bool getReady();
        bool getStop();

        void motorSnapshot();

        void setTaskID(int32_t value);
        int32_t getTaskID();

        void setPos(uint8_t index, int32_t value);
        void setDist(uint8_t index, uint32_t value);

        int32_t getPos(uint8_t index);
        uint32_t getDist(uint8_t index);

        // Control module
        uint16_t getIrqFlags();
        void clearIrqFlags(uint16_t value);
        void setIrqMask(uint16_t value);
        uint16_t getIrqMask();

        void reset();

        uint16_t getLimitSwitches();
        bool getAlarm();

        void setDato(uint64_t value);
        uint64_t getDatoOld();
        uint16_t getDati();

        // ADCs
        void adcSnapshot();
        bool getADC(uint8_t i, uint16_t* const value);
        void setThld(uint16_t low, uint16_t high);
        uint16_t getLowThld();
        uint16_t getHighThld();
        void setAdcEnable(bool ena);
        bool getAdcEnable();
        void setRunEnable(bool ena);
        bool getRunEnable();
        bool getPermit();

        // Control
        cnc_version_t version();
        std::string readVersion();

//        void setKeyLevel(uint32_t value);
//        uint32_t getKeyLevel();

        // Aux
        static uint32_t toTicks(double ms);

    signals:
        void error(const std::string& s);
    };
}

#endif // FPGA_H
