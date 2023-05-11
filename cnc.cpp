#include "cnc.h"
#include <vector>
#include <list>
#include "aux_items.h"
#include <cstdio>
#include <ctime>
#include <cmath>
//#include <chrono>
//#include <thread>
#include <unistd.h>
#include <QThread>
#include <QTextEdit>
#include <QDebug>
#include <QObject>
#include <QString>

using namespace std;
using namespace auxItems;

CncCom Cnc::m_com;

Cnc::Cnc(QObject* parent) : QObject(parent), m_msg(nullptr), m_fpga(m_com) {
    connect(&m_com, &CncCom::writeBurstProcessing,  this, [=](int pct) { emit writeBurstProcessing(pct);});
    connect(&m_com, &CncCom::readBurstProcessing,  this, [=](int pct) { emit readBurstProcessing(pct);});

    connect(&m_com,     &CncCom::error,         this, [=](const string& s) { emit error(s); });
    connect(&m_fpga,    &Fpga::FpgaBus::error,  this, [=](const string& s) { emit error(s); });
}

Cnc::~Cnc() {
    close();
}

void Cnc::bindReporter(Reporter* const msg) {
    m_msg = msg;
}

void Cnc::unbindReporter() {
    m_msg = nullptr;
}

bool Cnc::open() {
    return m_com.open();
}

void Cnc::close() {
    reset();
    m_com.close();
}

void Cnc::reconnect() {
    m_com.close();
    m_com.open();
}

bool Cnc::stateClear() {
    return m_com.write32(ADDR::CLEAR, ADDR::STATE_RESET_MASK);
}

bool Cnc::reset() {
//    com.write32(ADDR::CLEAR, ADDR::RESET_MASK);

    bool OK = writeSettings(
        CncParam::inputLevel,
        CncParam::sdEnable, CncParam::sdEnable,
        CncParam::reverseX, CncParam::reverseY, CncParam::reverseU, CncParam::reverseV,
        CncParam::swapXY, CncParam::swapUV,
        CncParam::reverseEncX, CncParam::reverseEncY,
        CncParam::acc_ena, CncParam::acc, CncParam::dec
    ); // reset included

    if (OK)
        OK = writeStep(
            CncParam::step,
            CncParam::scaleX, CncParam::scaleY, CncParam::scaleU, CncParam::scaleV,
            CncParam::scaleEncX, CncParam::scaleEncY, CncParam::encXY
        );

    if (OK)
        OK = writeFeedback(
            CncParam::fb_ena,
            CncParam::low_thld[0], CncParam::high_thld[1],
            CncParam::rb_to, CncParam::rb_attempts, CncParam::rb_len, CncParam::rb_speed,
            CncParam::fb_acc, CncParam::fb_dec
        );

    return OK;
}

void Cnc::clear() {
    m_com.clear();
}

bool Cnc::writeArray(uint32_t addr, const vector<uint32_t>& data) {
    qDebug("cnc::writeArray>>Address 0x%08x, %d DWords: ", int(addr), int(data.size()));

    bool OK = m_com.write(addr, reinterpret_cast<const uint8_t*>(data.data()), data.size()<<2);
    if (OK)
        qDebug("OK");
    else
        qDebug("Error");

    return OK;
}

bool Cnc::writeProgArray(const vector<uint8_t>& bytes) {
    static vector<uint32_t> addr_ar(2);
    addr_ar[0] = 0;
    addr_ar[1] = (uint32_t)bytes.size();

    qDebug("cnc::writeProgArray>>%d bytes\n", (int)addr_ar[1]);
    bool OK = m_com.write(ADDR::PA, bytes);

    if (OK)
        OK = writeArray(ADDR::PA_RDADDR, addr_ar);

//    m_com.write32(ADDR::PA_RDADDR, 0);
//    m_com.write32(ADDR::PA_WRADDR, static_cast<uint32_t>(bytes.size()));

    return OK;
}

bool Cnc::writeProgArrayBurst(const vector<uint8_t>& bytes) {
    qDebug("cnc::writeProgArrayBurst>>Write to Program Array %d bytes (burst): ", int(bytes.size()));

    bool OK = m_com.writeBurst(ADDR::PA, bytes);

    if (OK)
        OK = writeArray(ADDR::PA_RDADDR, {0, uint32_t(bytes.size())});

    if (OK)
        qDebug("OK");
    else
        qDebug("Error");

    return OK;
}

uint32_t Cnc::readProgArraySize() {
    uint32_t data;
    m_com.read32(ADDR::PA_SIZE, data);
    return data;
}

vector<uint8_t> Cnc::readProgArray(size_t len) {
    vector<uint8_t> bytes;

    qDebug("Read Program Array\n");
    m_com.read(ADDR::PA, len, bytes);
    qDebug("read %d bytes\n", int(bytes.size()));

    return bytes;
}

vector<uint8_t> Cnc::readProgArray() {
    uint32_t len;
    vector<uint8_t> bytes;

    if (!m_com.read32(ADDR::PA_WRADDR, len))
        return bytes;

    bytes = readProgArray(len);

    return bytes;
}

vector<uint8_t> Cnc::readProgArrayBurst(size_t len) {
    qDebug("cnc::readProgArrayBurst>>Read request %d bytes\n", (int)len);

    vector<uint8_t> bytes;
    bool OK = m_com.readBurst(ADDR::PA, len, bytes); // one attempt, try again manually

    if (!OK)
        qDebug("Read burst error: %s\n", m_com.lastError().c_str());

    qDebug("cnc::readProgArrayBurst>>Read %d bytes\n", int(bytes.size()));
    return bytes;
}

vector<uint8_t> Cnc::readProgArrayBurst() {
    uint32_t len;
    vector<uint8_t> bytes;

    if (!m_com.read32(ADDR::PA_WRADDR, len))
        return bytes;

    bytes = readProgArrayBurst(len);
    return bytes;
}

string fileReadLine(FILE* fp) {
    string s;
    s.resize(256);
    s.clear();

    while (!feof(fp)) {
        int ch = fgetc(fp);

        if (ferror(fp)) {
            qDebug("Read file error\n");
            s.clear();
            break;
        }
        else {
            if (ch == '\n' || ch == '\0')
                break;
            else
                s += static_cast<char>(ch);
        }
    }

    return s;
}

string textReadLine(const string& text,  string::const_iterator& it) {
    string s;
    s.resize(256);
    s.clear();

    while (it != text.end()) {
        if (*it == '\n' || *it == '\0')
            break;
        else
            s += *it;

        ++it;
    }

    return s;
}

bool Cnc::writeFromFile(const string& fileName) {
    FILE* fp = fopen(fileName.c_str(), "r");

    if (fp == nullptr) {
        m_msg->append( QString::asprintf("File \"%s\" is not found\n", fileName.c_str()) );
        return false;
    }
    m_msg->append( QString::asprintf("Read G-code from file \"%s\"\n", fileName.c_str()) );

    list<string> ss;

    string s = fileReadLine(fp);
    size_t n = s.size() + 1;

    while (!s.empty()) {
        ss.push_back(std::move(s));
        s = fileReadLine(fp);
        n += s.size() + 1;
    }

    print_strings(m_msg, ss);

    vector<uint8_t> data;
    for (list<string>::const_iterator it = ss.begin(); it != ss.end(); ++it)
        push_back_range(data, *it);

    uint32_t pa_size;
    if (!m_com.read32(ADDR::PA_SIZE, pa_size))
        return false;

    if (data.size() > pa_size) {
        qDebug("G-code size is too big %d bytes (available only %d bytes)", int(data.size()), int(pa_size));
        return false;
    }

    return writeProgArray(data);
}

//#if !defined(STONE)
    #define BURST_RW
//#endif

bool Cnc::write(const std::list<std::string>& frames, size_t* p_wrsize, size_t* p_pa_size) {
    QElapsedTimer t;

    t.start();
    m_wrbuf.clear();

    for (list<string>::const_iterator it = frames.begin(); it != frames.end(); ++it)
        push_back_range(m_wrbuf, *it);

    size_t wrsize = m_wrbuf.size();
    if (p_wrsize)
        *p_wrsize = wrsize;

    qDebug("cnc::write>>G-code size: %d bytes", (int)wrsize);

    uint32_t pa_size;

    if (!m_com.read32(ADDR::PA_SIZE, pa_size))
        return false;

    if (p_pa_size)
        *p_pa_size = pa_size;

    qDebug("cnc::write>>CNC program array size: %d bytes", (int)pa_size);

    if (wrsize > pa_size) {
        m_msg->append( QString::asprintf("G-code size is too big %d bytes (available only %d bytes)", int(wrsize), int(pa_size)) );
        qDebug("cnc::write>>G-code size is too big");
        return false;
    }

    qDebug("cncWrite elapsed 1: %d ms", (int)t.elapsed());

    t.start();
#if defined(BURST_RW)
    bool OK = writeProgArrayBurst(m_wrbuf);
#else
    bool OK = writeProgArray(m_wrbuf);
#endif
    qDebug("cncWrite elapsed 2: %d ms", (int)t.elapsed());

    return OK;
}

void Cnc::printGCode() {
    list<string> frames = read();
    print_strings(frames);
}

list<string> Cnc::read() {
    vector<uint8_t> bytes;
    list<string> frames;
    string s;

#if defined(BURST_RW)
    bytes = readProgArrayBurst();
#else
    bytes = readProgArray();
#endif

    qDebug("Read %d bytes:\n", (int)bytes.size());
    for (size_t i = 0; i < bytes.size(); i++) {
        if (bytes[i] == '\0') {
            qDebug("%s\n", s.c_str());
            frames.push_back(s);
            s.clear();
        }
        else
            s += static_cast<char>(bytes[i]);
    }

    return frames;
}

/* scale - steps/mm
 * speed - mm/min
 */
bool Cnc::directMoveOn(int32_t nx, double scale_x, int32_t ny, double scale_y, int32_t nu, double scale_u, int32_t nv, double scale_v, double speed) {
    if (nx || ny || nu || nv) {
        if (scale_x <= 0) scale_x = 0;
        if (scale_y <= 0) scale_y = 0;
        if (scale_u <= 0) scale_u = 0;
        if (scale_v <= 0) scale_v = 0;
        speed = fabs(speed);

        double dx = nx / scale_x; // mm
        double dy = ny / scale_y;
        double du = nu / scale_u;
        double dv = nv / scale_v;

        double len_xy = sqrt(dx * dx + dy * dy);
        double len_uv = sqrt(du * du + dv * dv);
        double len = len_xy > len_uv ? len_xy : len_uv;

        double T = 60.0 * FPGA_CLOCK / speed; // clocks/mm

        double ts = len * T;

        int32_t Tx = nx ? auxItems::double_to_int32(ts / fabs(nx)) : 0;
        int32_t Ty = ny ? auxItems::double_to_int32(ts / fabs(ny)) : 0;
        int32_t Tu = nu ? auxItems::double_to_int32(ts / fabs(nu)) : 0;
        int32_t Tv = nv ? auxItems::double_to_int32(ts / fabs(nv)) : 0;

        int32_t data[] = {nx, Tx, ny, Ty, nu, Tu, nv, Tv};
        return m_com.write(ADDR::MOVE_X, data, sizeof(data));
    }

    return false;
}

bool Cnc::directSetPos(int32_t nx, int32_t ny, int32_t nu, int32_t nv, int32_t enc_x, int32_t enc_y) {
    int32_t data[] = {nx, ny, nu, nv, enc_x, enc_y};
    return m_com.write(ADDR::SET_X, data, sizeof(data));
}

float angle360(float a) {
    if (a < 0)
        a = 0;
    else if (a > 2*M_PI)
        a = 2*M_PI;

    return a;
}

bool Cnc::centering(
        CENTER_MODE_T mode,
        int touches, int attempts,
        int rollVel, int thld,
        int fineSharePct, float R,
        float rollback,
        float speedCoarse, float speedFine,
        float angle0, float angle1, float angle2)
{
    if (touches < 1)
        touches = 1;
    else if (touches > CENTER_ATTEMPTS_MAX)
        touches = CENTER_ATTEMPTS_MAX;

    if (attempts < 1)
        attempts = 1;
    else if (attempts > CENTER_ATTEMPTS_MAX)
        attempts = CENTER_ATTEMPTS_MAX;

   if (rollVel < 0)
        rollVel = 0;
    else if (rollVel > 7)
        rollVel = 7;

   if (fineSharePct < 0)
       fineSharePct = 0;
   else if (fineSharePct > 100)
       fineSharePct = 100;

    if (R < -CENTER_RADIUS_MAX)
        R = -CENTER_RADIUS_MAX;
    else if (R > CENTER_RADIUS_MAX)
        R = CENTER_RADIUS_MAX;

    rollback = fabs(rollback);
    if (rollback > CENTER_RADIUS_MAX)
        rollback = CENTER_RADIUS_MAX;

    speedCoarse = fabs(speedCoarse);
    if (speedCoarse > cnc_param::SPEED_MAX) speedCoarse = cnc_param::SPEED_MAX;

    speedFine = fabs(speedFine);
    if (speedFine > cnc_param::SPEED_MAX) speedFine = cnc_param::SPEED_MAX;

    angle0 = angle360(angle0);
    angle1 = angle360(angle1);
    angle2 = angle360(angle2);

    if (mode == CENTER_MODE_T::CENTER_X || mode == CENTER_MODE_T::CENTER_Y || mode == CENTER_MODE_T::CENTER_CIRCLE_4R || mode == CENTER_MODE_T::CENTER_CIRCLE_3ADJ) {
        if (R >= 0)
            rollback = -rollback;

        int32_t data[] = {
            int32_t(rollVel)<<24 | int32_t(attempts)<<16 | int32_t(touches)<<8 | int32_t(mode),
            int32_t(fineSharePct)<<16 | int32_t(cnc_adc_volt_t::toCode(0, thld)),
            *reinterpret_cast<int32_t*>(&R),
            *reinterpret_cast<int32_t*>(&rollback),
            *reinterpret_cast<int32_t*>(&speedCoarse),
            *reinterpret_cast<int32_t*>(&speedFine),
            *reinterpret_cast<int32_t*>(&angle0),
            *reinterpret_cast<int32_t*>(&angle1),
            *reinterpret_cast<int32_t*>(&angle2)
        };

        return m_com.write(ADDR::CENTER_MODE, data, sizeof(data));
    } else if (mode == CENTER_MODE_T::CENTER_NO)
        return true;

    return false;
}

bool Cnc::runReq() {
    return m_com.write32(ADDR::STATUS, ADDR::RUN_MASK);
}

bool Cnc::revReq() {
    return m_com.write32(ADDR::STATUS, ADDR::REV_MASK | ADDR::RUN_MASK);
}

bool Cnc::cancelReq() {
    return m_com.write32(ADDR::CLEAR, ADDR::CANCEL_MASK);
}

bool Cnc::stopReq() {
    return m_com.write32(ADDR::CLEAR, ADDR::STOP_MASK);
}

bool Cnc::isRun() {
    uint32_t data;
    m_com.read32(ADDR::STATUS, data);
    return (data & ADDR::RUN_MASK) != 0;
}

cnc_context_t Cnc::readCncContext() {
    vector<uint8_t> v;

    if (m_com.read(ADDR::CONTEXT, CncContext::SIZE, v)) {
        cnc_context_t ctx = CncContext::parse(v);
        ctx.field.backup_valid = true; // add valid for save param
        return ctx;
    }

    return CncContext::defaultContext();
}

double Cnc::readDiameter() {
    float res;
    m_com.readFloat(ADDR::DIA, res);
    return res;
}

cnc_context_t Cnc::readBackup() {
    vector<uint8_t> v;

    if (m_com.read(ADDR::BACKUP, CncContext::BACKUP_SIZE, v))
        return CncContext::parse(v);

    return CncContext::defaultContext();
}

bool Cnc::clearBackup() {
    return m_com.write32(ADDR::BACKUP + ((CncContext::BACKUP_SIZE32 - 1) << 2), 0);
}

// program must be loaded + reset
void Cnc::initialContext(const cnc_context_t& ctx) {
    writeXYUVEnc(ctx.field.x, ctx.field.y, ctx.field.u, ctx.field.v, ctx.field.enc_x, ctx.field.enc_y);
    writeGoto(ctx.field.id);

//    writePumpEnable(ctx.field.pump_ena);
//    writeRollEnable(ctx.field.roll_state == unsigned(roll_state_t::ROLL_ANY) ||
//                    ctx.field.roll_state == unsigned(roll_state_t::ROLL_REV) ||
//                    ctx.field.roll_state == unsigned(roll_state_t::ROLL_REV)
//                    );
    writeWireEnable(ctx.field.wire_ena);
//    writeVoltageEnable(ctx.field.voltage_ena);
    writeHoldEnable(ctx.field.hold_ena);
    writeRollVel(ctx.field.roll_vel);
    writeEnableLowHighVolt(ctx.field.low_high_voltage_ena);
    writeCurrentIndex(ctx.field.current_index);
    writePulseWidth(ctx.field.pulse_width);
    writePulseRatio(ctx.field.pulse_ratio);
    writeSpeed( WireSpeed::TtoSpeed(ctx.field.T) );
    writeEnableUV(ctx.field.uv_ena);
}

cnc_adc_t Cnc::readADC() {
    vector<uint8_t> v;

    if (m_com.read(ADDR::ADC, cnc_adc_t::SIZE, v))
        return cnc_adc_t::parse(v);

    return cnc_adc_t();
}

cnc_adc_volt_t Cnc::readADCVolt() {
    return cnc_adc_volt_t( readADC() );
}

void Cnc::imitEna(bool value) {
    if (value)
        m_com.write32(ADDR::STATUS, ADDR::IMIT_MASK);
    else
        m_com.write32(ADDR::CLEAR, ADDR::IMIT_MASK);
}

bool Cnc::isImitEna() {
    uint32_t data;
    m_com.read32(ADDR::STATUS, data);
    return (data & ADDR::IMIT_MASK) != 0;
}

void Cnc::readImitFifo() {
    m_imit_list.clear();

    int timeout = 10;
    int cnt = timeout;
    int pack_num = 0, total = 0;
    MotorRecord rec;
    const uint8_t max = 21 * MotorRecord::size; // len = 252

    do {
        timespec tic, toc;
        clock_gettime(CLOCK_MONOTONIC, &tic);

        vector<uint8_t> v;
        if (!m_com.readFifo255(ADDR::IMIT_FIFO_Q, max, v))
            return;

        clock_gettime(CLOCK_MONOTONIC, &toc);
        double sec = 1.0 * (toc.tv_sec - tic.tv_sec) + 1e-9 * (toc.tv_nsec - tic.tv_nsec);

        cnt--;
        size_t it = 0;
        pack_num++;

        if (!v.empty()) {
            double baud = 8.0 * v.size() / sec;
            total += v.size();
            m_msg->append(QString::asprintf(\
                           "%d Read %g records (%d bytes). Total: records %g (%d bytes). Baud %g bit/s\n",\
                           pack_num, double(v.size()) / MotorRecord::size, int(v.size()), double(total) / MotorRecord::size, total, std::round(baud))\
                       );

            do {
                rec = MotorRecord(v, &it);
                m_imit_list.push_back(rec);
                if (rec.stop) break;
            } while (it < v.size());

            if (rec.stop) break;
            cnt = timeout;
        }
        else
            qDebug("%d Read %g records (%d bytes). Total: records %g (%d bytes). Baud %g bit/s\n", pack_num, 0.0, 0, double(total) / MotorRecord::size, total, 0.0);

//            if (b.size() < max / 2) usleep(100);
//            usleep(100);
        QThread::msleep(100);
    } while (cnt != 0);

//        if (!rec.stop)
//            qDebug("Timeout\n");
}

void Cnc::saveImitData(string fileName) {
    deque<int32_t> X, Y;

    for (deque<MotorRecord>::const_iterator it = m_imit_list.begin(); it != m_imit_list.end(); ++it)
        switch (it->axis) {
            case AXIS::AXIS_X: X.push_back(it->N); Y.push_back(0); break;
            case AXIS::AXIS_Y: X.push_back(0); Y.push_back(it->N); break;
            default: throw string("Axis error");
        }

    vector<int32_t> v;

    for (size_t i = 0; i < min(X.size(), Y.size()); i++) {
        v.push_back(X[i]);
        v.push_back(Y[i]);
    }

    FILE* fp = fopen(fileName.c_str(), "wb");

    if (fp == nullptr) {
        qDebug("File \"%s\" not found\n", fileName.c_str());
        return;
    }

    fwrite(v.data(), sizeof(int32_t), v.size(), fp);
    fflush(fp);
    m_msg->append("Imitation data saved to file \"" + fileName + "\"");
}

string Cnc::versionDate() {
    char date[4 * sizeof(uint32_t)] = {0}, time[4 * sizeof(uint32_t)] = {0};
    uint32_t rddata;

    for (uint32_t i = 0; i < sizeof(date) / sizeof(uint32_t); i++) {
        rddata = 0;

        if (m_com.read32(ADDR::VER_DATE + (i << 2), rddata))
            memcpy(&date[sizeof(uint32_t) * i], &rddata, sizeof(uint32_t));
    }

    for (uint32_t i = 0; i < sizeof(time) / sizeof(uint32_t); i++) {
        rddata = 0;

        if (m_com.read32(ADDR::VER_TIME + (i << 2), rddata))
            memcpy(&time[sizeof(uint32_t) * i], &rddata, sizeof(uint32_t));
    }

    return string(date, 0, sizeof(date)) + " " + string(time, 0, sizeof(time));
}

uint32_t Cnc::sysClock() {
    uint32_t data;
    m_com.read32(ADDR::SYS_CLOCK, data);
    return data;
}

cnc_version_t Cnc::version() {
    uint32_t data;
    m_com.read32(ADDR::VER, data);
    return data;
}

cnc_wdt_t Cnc::wdt() {
    uint32_t data;
    cnc_wdt_t res = {0,0};

    if (m_com.read32(ADDR::WDT, data)) {
        if (data)
            qDebug("WDT_REG: %x", data);

        res.wdt = (data & 1) != 0;
        res.swdt = (data & 2) != 0;
    }

    return res;
}

string Cnc::readVersion() {
    cnc_version_t mcu_ver = version();    
    string mcu_s = mcu_ver.toString();
    string mcu_date = versionDate();
    cnc_wdt_t mcu_wdt = wdt();
    uint32_t f = sysClock();
    string res = QObject::tr("CNC").toStdString() + " " + QObject::tr("ver.").toStdString() + " " + mcu_s;
    res += string_format(" (%g", static_cast<double>(f/1e6));
    res += " " + QObject::tr("MHz").toStdString() + ")";
    res += " " + QObject::tr("built on").toStdString();
    res += string_format(" %s", mcu_date.c_str());

    if (mcu_wdt.wdt || mcu_wdt.swdt) {
        res += " (" + QObject::tr("Error").toStdString() + ": ";

        if (mcu_wdt.wdt)
            res += string("WDT");

        if (mcu_wdt.swdt) {
            if (mcu_wdt.wdt) res += ", ";
            res += "Soft WDT";
        }

        res += ")";
    }

    qDebug("%s", res.c_str());
    return res;
}

bool Cnc::writeUInt32(uint32_t addr, uint32_t data) {
    return m_com.write32(addr, data);
}

bool Cnc::writeFloat(uint32_t addr, float data) {
    return m_com.writeFloat(addr, data);
}

bool Cnc::writeSetBits(uint32_t addr, unsigned bit, unsigned width, uint16_t data) {
    uint32_t mask;

    if (width >= 16)
        mask = 0xffff;
    else
        mask = (1<<width) - 1;

    mask <<= bit;

    uint32_t data32 = (mask<<16) | (uint16_t(data<<bit) & mask);

    return writeUInt32(addr, data32);
}

uint32_t Cnc::readUInt32(uint32_t addr, uint32_t defaultValue, bool* pOK) {
    uint32_t res = defaultValue;

    if (isOpen()) {
        bool OK = m_com.read32(addr, res);

        if (pOK)
            *pOK = OK;
    }

    return res;
}

uint32_t Cnc::readBit(uint32_t addr, unsigned bit, unsigned width, uint32_t defaultValue, bool* pOK) {
    bool OK = false;
    uint32_t res = readUInt32(addr, 0, &OK);

    if (pOK)
        *pOK = OK;

    if (OK) {
        uint32_t mask;
        if (width > 31)
            mask = 0xffffffff;
        else
            mask = (1U<<width) - 1;

        return bit < 32 ? res>>bit & mask : 0;
    }

    return defaultValue;
}

bool Cnc::writePumpEnable(bool ena) {
    return writeSetBits(ADDR::CONTROLS_ENABLE, 0, 1, ena);
}
bool Cnc::writeRollEnable(bool ena) {
    return writeSetBits(ADDR::CONTROLS_ENABLE, 1, 2, ena ? uint16_t(roll_state_t::ROLL_ANY) : uint16_t(roll_state_t::ROLL_DIS));
}
bool Cnc::writeWireEnable(bool ena) {
    return writeSetBits(ADDR::CONTROLS_ENABLE, 3, 1, ena);
}
// Enable high voltage if roll and pump enabled
// Disable high voltage, roll and pump
bool Cnc::writeEnableHighVoltage(bool ena) {
    return writeSetBits(ADDR::CONTROLS_ENABLE, 4, 1, ena);
}
bool Cnc::writeHoldEnable(bool ena) {
    return writeSetBits(ADDR::CONTROLS_ENABLE, 5, 1, ena);
}

bool Cnc::writeSemaphoreEnable(bool ena) {
    return writeSetBits(ADDR::CONTROLS_ENABLE, 6, 1, ena);
}

bool Cnc::writeSemaphoreCncEnable(bool ena) {
    return writeSetBits(ADDR::CONTROLS_ENABLE, 6, 2, ena ? 3 : 0);
}

bool Cnc::writeCncEnable(bool ena) {
    return writeSetBits(ADDR::CONTROLS_ENABLE, 7, 1, ena);
}

bool Cnc::writeWeakHVEnable(bool ena) {
    return writeSetBits(ADDR::CONTROLS_ENABLE, 8, 1, ena);
}

bool Cnc::writeEncXYEna(bool ena) {
    return writeSetBits(ADDR::BOOL_SET, 0, 1, ena);
}

bool Cnc::writeUVEna(bool ena) {
    return writeSetBits(ADDR::BOOL_SET, 8, 1, ena);
}

bool Cnc::writeRollVel(unsigned value) { return writeUInt32(ADDR::ROLL_VEL, value); }
bool Cnc::writeEnableLowHighVolt(bool value) { return writeUInt32(ADDR::LOW_HV, value); }
bool Cnc::writeCurrentIndex(size_t index) {
    if (index > UINT8_MAX)
        index = UINT8_MAX;

    return writeUInt32(ADDR::CURRENT_INDEX, uint32_t(index));
}
bool Cnc::writePulseWidth(unsigned value) { return writeUInt32(ADDR::PULSE_WIDTH, value); }
bool Cnc::writePulseRatio(unsigned value) { return writeUInt32(ADDR::PULSE_RATIO, value); }

// value - mm/min
bool Cnc::writeSpeed(float value) { return writeFloat(ADDR::SPEED, value); }
bool Cnc::writeSpeed(const WireSpeed& value) { return writeSpeed( float(value.getMMM()) ); }

// mm
bool Cnc::writeStep(float value) { return writeFloat(ADDR::STEP, value); }

bool Cnc::writeEnableUV(bool ena) { return writeUInt32(ADDR::BOOL_SET, 1<<(16+8) | (uint32_t)ena<<8); }

bool Cnc::writeGoto(int32_t frame_num) {
    if (frame_num < 0)
        frame_num = 0;

    return writeUInt32(ADDR::GOTO, uint32_t(frame_num));
}
bool Cnc::writeXYUVEnc(int nx, int ny, int nu, int nv, int enc_x, int enc_y) {
    int32_t data[] = {nx, ny, nu, nv, enc_x, enc_y};
    return m_com.write(ADDR::REC_X, data, sizeof(data));
}

bool Cnc::writeSettings(
        uint16_t input_lvl,
        bool sd_oe, bool sd_ena,
        bool rev_x, bool rev_y, bool rev_u, bool rev_v, bool swap_xy, bool swap_uv,
        bool rev_enc_x, bool rev_enc_y,
        bool acc_ena, double acc, double dec
){
    vector<uint8_t> v = vector<uint8_t>(6 * sizeof(uint32_t));

    uint32_t data = input_lvl;
    memcpy(&v[0], reinterpret_cast<uint8_t*>(&data), sizeof(uint32_t));

    data = uint32_t(rev_enc_y)<<17 | uint32_t(rev_enc_x)<<16 |
           uint32_t(swap_uv)<<9 | uint32_t(swap_xy)<<8 | uint32_t(rev_v)<<3 | uint32_t(rev_u)<<2 | uint32_t(rev_y)<<1 | uint32_t(rev_x);
    memcpy(&v[4], reinterpret_cast<uint8_t*>(&data), sizeof(uint32_t));

    data = uint32_t(sd_ena)<<1 | uint32_t(sd_oe);
    memcpy(&v[8], reinterpret_cast<uint8_t*>(&data), sizeof(uint32_t));

    // Acceleration
    data = uint32_t(acc_ena);
    memcpy(&v[12], reinterpret_cast<uint8_t*>(&data), sizeof(uint32_t));

    float fdata = acc;
    memcpy(&v[16], reinterpret_cast<uint8_t*>(&fdata), sizeof(float));

    fdata = dec;
    memcpy(&v[20], reinterpret_cast<uint8_t*>(&fdata), sizeof(float));

    bool OK = m_com.write(ADDR::INPUT_LEVEL, v);

    if (!OK)
        qDebug("WRITE SETTINGS ERROR");

    return OK;
}

bool Cnc::writeStep(float step, float scale_x, float scale_y, float scale_u, float scale_v, float scale_enc_x, float scale_enc_y, bool encXY) {
    vector<uint8_t> v = vector<uint8_t>(8 * sizeof(float));

    memcpy(&v[0], reinterpret_cast<uint8_t*>(&step), sizeof(float));
    memcpy(&v[4], reinterpret_cast<uint8_t*>(&scale_x), sizeof(float));
    memcpy(&v[8], reinterpret_cast<uint8_t*>(&scale_y), sizeof(float));
    memcpy(&v[12], reinterpret_cast<uint8_t*>(&scale_u), sizeof(float));
    memcpy(&v[16], reinterpret_cast<uint8_t*>(&scale_v), sizeof(float));
    memcpy(&v[20], reinterpret_cast<uint8_t*>(&scale_enc_x), sizeof(float));
    memcpy(&v[24], reinterpret_cast<uint8_t*>(&scale_enc_y), sizeof(float));

    uint32_t data = 1U<<16 | (uint32_t)encXY<<0; // enable mask and flag
    memcpy(&v[28], reinterpret_cast<uint8_t*>(&data), sizeof(uint32_t));

    bool OK = m_com.write(ADDR::STEP, v);

    if (!OK)
        qDebug("WRITE STEP ERROR");

    return OK;
}

bool Cnc::readSettings(
    uint16_t& input_lvl,
    bool& sd_oe, bool& sd_ena,
    bool& rev_x, bool& rev_y, bool& rev_u, bool& rev_v,
    bool& swap_xy, bool& swap_uv,
    bool& rev_enc_x, bool& rev_enc_y,
    bool& acc_ena, double& acc, double& dec
){
    vector<uint8_t> v;

    if (m_com.read(ADDR::INPUT_LEVEL, 6 * sizeof(uint32_t), v)) {
        if (v.size() == 6 * sizeof(uint32_t)) {
            input_lvl = BitConverter::toUInt16(v, 0);

            uint32_t motor_dir = BitConverter::toUInt32(v, 4);
            rev_x = motor_dir & 1;
            rev_y = (motor_dir & 1<<1) != 0;
            rev_u = (motor_dir & 1<<2) != 0;
            rev_v = (motor_dir & 1<<3) != 0;
            swap_xy = (motor_dir & 1<<8) != 0;
            swap_uv = (motor_dir & 1<<9) != 0;
            rev_enc_x = (motor_dir & 1<<16) != 0;
            rev_enc_y = (motor_dir & 1<<17) != 0;

            uint32_t sd = BitConverter::toUInt32(v, 8);
            sd_oe = (sd & 1<<0) != 0;
            sd_ena = (sd & 1<<1) != 0;

            uint32_t data = BitConverter::toUInt32(v, 12);
            acc_ena = (data & 1<<0) != 0;

            acc = BitConverter::toFloat(v, 16);
            dec = BitConverter::toFloat(v, 20);

            return true;
        }
    }

    qDebug("READ SETTINGS ERROR");
    return false;
}

bool Cnc::readStep(float &step, float &scaleX, float &scaleY, float &scaleU, float &scaleV, float &scaleEncX, float &scaleEncY, bool &encXY) {
    const size_t size = 8 * sizeof(uint32_t);
    vector<uint8_t> v;

    if (m_com.read(ADDR::STEP, size, v)) {
        if (v.size() == size) {
            step = BitConverter::toFloat(v, 0);
            scaleX = BitConverter::toFloat(v, 4);
            scaleY = BitConverter::toFloat(v, 8);
            scaleU = BitConverter::toFloat(v, 12);
            scaleV = BitConverter::toFloat(v, 16);
            scaleEncX = BitConverter::toFloat(v, 20);
            scaleEncY = BitConverter::toFloat(v, 24);
            encXY = v[28] & 1;

            return true;
        }
    }

    qDebug("READ SETTINGS ERROR");
    return false;
}

//bool Cnc::writeAdcThreshold(bool enable, uint16_t low, uint16_t high) {
//    if (high < low)
//        high = low;

//    //    CncParam::inputLevel = value;

//        try {
//            if (isOpen()) {
//                if (!enable)
//                    com.write32(ADDR::ADC_ENA, 0);
//                else
//                    com.write64(ADDR::ADC_THDL, static_cast<uint64_t>(enable)<<32 | static_cast<uint64_t>(high)<<16 | low);

//                return true;
//            }
//        }
//        catch (...) {}

//        qDebug("WRITE ADC THRESHOLD LEVEL ERROR");
//        return false;
//}

//bool Cnc::writeAdcThresholdVolt(bool enable, double low, double high) {
//    return writeAdcThreshold( enable, cnc_adc_volt_t::toCode(0, low), cnc_adc_volt_t::toCode(0, high) );
//}

//bool Cnc::readAdcThreshold(bool& enable, uint16_t& low, uint16_t& high) {
//    try {
//        if (isOpen()) {
//            uint64_t data = com.read64(ADDR::ADC_THDL);
//            low = static_cast<uint16_t>(data);
//            high = static_cast<uint16_t>(data>>16);
//            enable = (data & (1ULL << 32)) != 0;
////            CncParam::inputLevel = value;
//            return true;
//        }
//    } catch (...) {}

//    qDebug("WRITE ADC THRESHOLD LEVEL ERROR");
//    return false;
//}

//bool Cnc::readAdcThresholdVolt(bool& enable, double& low, double& high) {
//    uint16_t low_code, high_code;

//    bool OK = readAdcThreshold(enable, low_code, high_code);
//    low = cnc_adc_volt_t::toVolt(0, low_code);
//    high = cnc_adc_volt_t::toVolt(0, high_code);
//    return OK;
//}

bool Cnc::writeFeedback(bool enable, double Vlow, double Vhigh, double to_sec, uint32_t attempts, double length_mm, double speed_mmm, double fb_acc, double fb_dec) {
//    int Vthld_max = static_cast<int>( round(cnc_adc_volt_t::maxVolt(0)) );
//    Vlow  = Vlow  > Vthld_max ? Vthld_max : Vlow;
//    Vhigh = Vhigh > Vthld_max ? Vthld_max : Vhigh;

    if (Vlow > Vhigh)
        Vlow = Vhigh;

    if (!enable)
        return m_com.write32(ADDR::FB_ENA, 0);

    vector<uint8_t> v = vector<uint8_t>(8 * sizeof(uint32_t));

    uint32_t data = enable;
    memcpy(&v[0], reinterpret_cast<uint8_t*>(&data), sizeof(uint32_t));

    data = static_cast<uint32_t>(cnc_adc_volt_t::toCode(0, Vhigh))<<16 | cnc_adc_volt_t::toCode(0, Vlow);
    memcpy(&v[4], reinterpret_cast<uint8_t*>(&data), sizeof(uint32_t));

    uint32_t ms = static_cast<uint32_t>(round(to_sec * 1e3));
    memcpy(&v[8], reinterpret_cast<uint8_t*>(&ms), sizeof(uint32_t));

    memcpy(&v[12], reinterpret_cast<uint8_t*>(&attempts), sizeof(uint32_t));

    float mm = length_mm;
    memcpy(&v[16], reinterpret_cast<uint8_t*>(&mm), sizeof(float));

    float mmm = speed_mmm;
    memcpy(&v[20], reinterpret_cast<uint8_t*>(&mmm), sizeof(float));

    float fdata = fb_acc; // per 100V
    memcpy(&v[24], reinterpret_cast<uint8_t*>(&fdata), sizeof(float));

    fdata = fb_dec; // per 100V
    memcpy(&v[28], reinterpret_cast<uint8_t*>(&fdata), sizeof(float));

    return m_com.write(ADDR::FB_ENA, v);
}

bool Cnc::writeFeedback(bool enable, double Vlow, double Vhigh) {
    if (Vhigh < Vlow)
        Vhigh = Vlow;

    if (!enable)
        return m_com.write32(ADDR::FB_ENA, 0);

    vector<uint8_t> v = vector<uint8_t>(6 * sizeof(uint32_t));

    uint32_t data = enable;
    memcpy(&v[0], reinterpret_cast<uint8_t*>(&data), sizeof(uint32_t));

    data = static_cast<uint32_t>(cnc_adc_volt_t::toCode(0, Vhigh))<<16 | cnc_adc_volt_t::toCode(0, Vlow);
    memcpy(&v[4], reinterpret_cast<uint8_t*>(&data), sizeof(uint32_t));

    return m_com.write(ADDR::FB_ENA, v);
}

bool Cnc::readFeedback(bool &enable, double &Vlow, double &Vhigh, double &to_sec, uint32_t &attempts, double &length_mm, double &speed_mmm, double& fb_acc, double& fb_dec) {
    vector<uint8_t> v;

    if (m_com.read(ADDR::FB_ENA, 8 * sizeof(uint32_t), v)) {
        if (v.size() == 8 * sizeof(uint32_t)) {
            enable = (v[0] & 1) != 0;
            Vlow = cnc_adc_volt_t::toVolt(0, BitConverter::toUInt16(v, 4));
            Vhigh = cnc_adc_volt_t::toVolt(0, BitConverter::toUInt16(v, 6));
            to_sec = BitConverter::toUInt32(v, 8) / 1e3;
            attempts = BitConverter::toUInt32(v, 12);
            length_mm = BitConverter::toFloat(v, 16);
            speed_mmm = BitConverter::toFloat(v, 20);
            fb_acc = BitConverter::toFloat(v, 24); // per 100V
            fb_dec = BitConverter::toFloat(v, 28); // per 100V
            return true;
        }
    }

    return false;
}
