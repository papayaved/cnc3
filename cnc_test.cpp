#include "cnc.h"
#include "aux_items.h"

using namespace std;
using namespace auxItems;

bool Cnc::testRegs() {
    qDebug("Test: RW test_reg\n");

    uint32_t test_reg;
    m_com.read32(ADDR::TEST_REG, test_reg);
    qDebug("Read test_reg: 0x%08X\n", test_reg);

    byte_reverse(test_reg);
    uint32_t wrdata = test_reg;

    m_com.write32(ADDR::TEST_REG, wrdata);
    m_com.read32(ADDR::TEST_REG, test_reg);

    qDebug("Read test_reg: 0x%08X", test_reg);

    if (test_reg == wrdata) {
        qDebug(" - OK\n");
        qDebug("###################\n");
        return true;
    }
    else {
        qDebug(" - Error (expected 0x%08X)\n", wrdata);
        qDebug("###################\n");
        return false;
    }
}

bool Cnc::testProgArray() {
    qDebug("Test: RW Program Array...\n");
    uint32_t run = isRun();
    uint32_t imit = isImitEna();
    uint32_t rdaddr, wraddr, size;
    m_com.read32(ADDR::PA_RDADDR, rdaddr);
    m_com.read32(ADDR::PA_WRADDR, wraddr);
    m_com.read32(ADDR::PA_SIZE, size);

    qDebug("PA size: %d, wraddr: %d, rdaddr = %d, run = %d, imit = %d\n", size, wraddr, rdaddr, run, imit);

    vector<uint8_t> wrdata = gen_rnd(size);

    m_com.write(ADDR::PA, wrdata);

    vector<uint8_t> rddata;
    m_com.read(ADDR::PA, size, rddata);

    bool OK = vector_compare(wrdata, rddata);

    if (OK) {
        qDebug("RW Program Array - OK\n");
        qDebug("###################\n");
        return true;
    }
    else
    {
        qDebug("RW Program Array - ERROR!\n");
        qDebug("###################\n");
        return false;
    }
}

bool Cnc::testFpga() {
    qDebug("FPGA test\n");
    cnc_version_t ver = m_fpga.version();
    std::string s = ver.toString();
    m_msg->writeLine("FPGA version: " + s);
    qDebug("FPGA version: %s\n", ver.toString().c_str());

    return true;
}
