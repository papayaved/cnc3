#include "fpga.h"
#include <cmath>

using namespace std;
using namespace Fpga;

FpgaBus::FpgaBus(CncCom& com) : m_com(com) {
    connect(&m_com, &CncCom::error, this, [=](const string& s) { emit error(s); });
}

uint16_t FpgaBus::read_u16(uint16_t addr) {
    uint16_t data;
    m_com.read16(BAR + addr, data);
    return data;
}
int16_t FpgaBus::read_s16(uint16_t addr) {
    uint16_t value = read_u16(addr);
    return *reinterpret_cast<int16_t*>(&value);
}
uint32_t FpgaBus::read_u32(uint16_t addr) {
    uint32_t data;
    m_com.read32(BAR + addr, data);
    return data;
}
int32_t FpgaBus::read_s32(uint16_t addr) {
    uint32_t value = read_u32(addr);
    return *reinterpret_cast<int32_t*>(&value);
}
uint64_t FpgaBus::read_u64(uint16_t addr) {
    uint64_t data;
    m_com.read64(BAR + addr, data);
    return data;
}
int64_t FpgaBus::read_s64(uint16_t addr) {
    uint64_t value = read_u64(addr);
    return *reinterpret_cast<int64_t*>(&value);
}
uint64_t FpgaBus::read_u48(uint16_t addr) {
    uint64_t data;
    m_com.read48(BAR + addr, data);
    return data;
}

void FpgaBus::write_u16(uint16_t addr, uint16_t data) {
    m_com.write16(BAR + addr, data);
}
void FpgaBus::write_s16(uint16_t addr, int16_t data) {
    m_com.write16(BAR + addr, *reinterpret_cast<uint16_t*>(data));
}
void FpgaBus::write_u32(uint16_t addr, uint32_t data) {
    m_com.write32(BAR + addr, data);
}
void FpgaBus::write_s32(uint16_t addr, int32_t data) {
    m_com.write32(BAR + addr, *reinterpret_cast<uint32_t*>(data));
}
void FpgaBus::write_u64(uint16_t addr, uint64_t data) {
    m_com.write64(BAR + addr, data);
}
void FpgaBus::write_s64(uint16_t addr, int64_t data) {
    m_com.write64(BAR + addr, *reinterpret_cast<uint64_t*>(data));
}
void FpgaBus::write_u48(uint16_t addr, uint64_t data) {
    m_com.write48(BAR + addr, data);
}

// MOTORS
void FpgaBus::step(uint8_t i, int N, unsigned T) {
    if (i < MTR_NUM) {
        write_s32(NT32 + i * sizeof(uint64_t), N);
        write_u32(NT32 + i * sizeof(uint64_t) + sizeof(uint32_t), T);
        write_u16(MTR_WRREQ, 1 << 3);
    }
}

int32_t FpgaBus::getN(uint8_t i) {
    if (i < MTR_NUM) return read_s32(NT32 + i * sizeof(uint64_t));
    return 0;
}

uint32_t FpgaBus::getT(uint8_t i) {
    if (i < MTR_NUM) return read_u32(NT32 + i * sizeof(uint64_t)) + sizeof(uint32_t);
    return 0;
}

void FpgaBus::setOE(bool oe) { write_u16(MTR_OE, oe ? 1 : 0); }
bool FpgaBus::getOE() { return read_u16(MTR_OE) == 1; }

uint16_t FpgaBus::getRun() { return read_u16(MTR_WRREQ); }
bool FpgaBus::getStop() { return getRun() == 0; }

void FpgaBus::motorSnapshot() { write_u16(MTR_CONTROL, MTR_SNAPSHOT_BIT); }

uint8_t FpgaBus::getWrreq() { return static_cast<uint8_t>(getRun() & 0xFF); }
bool FpgaBus::getReady() { return getWrreq() == 0; }

void FpgaBus::setTaskID(int32_t value) { write_s32(TASK_ID, value); }
int32_t FpgaBus::getTaskID() { return read_s32(TASK_ID); }

void FpgaBus::setPos(uint8_t index, int32_t value) { write_s32(POS32_DIST32 + index * sizeof(uint64_t), value); }
void FpgaBus::setDist(uint8_t index, uint32_t value) { write_u32(POS32_DIST32 + index * sizeof(uint64_t) + sizeof(uint32_t), value); }

int32_t FpgaBus::getPos(uint8_t index) { return read_s32(POS32_DIST32 + index * sizeof(uint64_t)); }
uint32_t FpgaBus::getDist(uint8_t index) { return read_u32(POS32_DIST32 + index * sizeof(uint64_t) + sizeof(uint32_t)); }

// Control module
uint16_t FpgaBus::getIrqFlags() { return read_u16(IRQ_FLAGS); }
void FpgaBus::clearIrqFlags(uint16_t value) { write_u16(IRQ_FLAGS, value); }
void FpgaBus::setIrqMask(uint16_t value) { write_u16(IRQ_MASK, value); }
uint16_t FpgaBus::getIrqMask() { return read_u16(IRQ_MASK); }

void FpgaBus::reset() { write_u16(RESET, 1); }

uint16_t FpgaBus::getLimitSwitches() { return read_u16(LIMSW); }
bool FpgaBus::getAlarm() { return getLimitSwitches() != 0; }

void FpgaBus::setDato(uint64_t dato) { write_u48(DATO, dato); }
uint64_t FpgaBus::getDatoOld() { return read_u48(DATO_OLD); }
uint16_t FpgaBus::getDati() { return read_u16(DATI); }

// ADCs
void FpgaBus::adcSnapshot() { write_u16(ADC_SNAPSHOT, 1); }

bool FpgaBus::getADC(uint8_t i, uint16_t* const value) {
    if (i < ADC_NUM) {
        *value = read_u16(ADC + i * sizeof(uint16_t));
        return *value < (1 << ADC_BITS);
    }
    *value = 0;
    return false;
}

void FpgaBus::setThld(uint16_t low, uint16_t high) {
    if (low > high)
        low = high;

    write_u16(LOW_THLD, low);
    write_u16(HIGH_THLD, high);
}
uint16_t FpgaBus::getLowThld() { return read_u16(LOW_THLD); }
uint16_t FpgaBus::getHighThld() { return read_u16(HIGH_THLD); }
void FpgaBus::setAdcEnable(bool ena) { write_u16(ADC_ENA, ena ? 1 : 0); }
bool FpgaBus::getAdcEnable() { return read_u16(ADC_ENA) == 1; }
void FpgaBus::setRunEnable(bool ena) { write_u16(RUN_ENA, ena ? 1 : 0); }
bool FpgaBus::getRunEnable() { return read_u16(RUN_ENA) == 1; }
bool FpgaBus::getPermit() { return read_u16(PERMIT) == 1; }

// Control
cnc_version_t FpgaBus::version() { return read_u32(VER32); }

std::string FpgaBus::readVersion() {
    cnc_version_t fpga_ver = version();
    std::string fpga_s = fpga_ver.toString();
    std::string res = QObject::tr("FPGA").toStdString() + " " + QObject::tr("ver.").toStdString() + " " + fpga_s;
    qDebug("%s", res.c_str());
    return res;
}

//void Fpga::setKeyLevel(uint32_t value) { return write_u32(KEYS_LEVEL32, value); }
//uint32_t Fpga::getKeyLevel() { return read_u32(KEYS_LEVEL32); }

// Aux
uint32_t FpgaBus::toTicks(double ms) {
    double res = ms * (CLOCK / 1000);

    if (res < 0)
        res = 0;
    else if (res > UINT_MAX)
        res = UINT_MAX;

    return static_cast<uint32_t>(round(res));
}
