#include "cnc_com.h"

using namespace std;
using namespace auxItems;

CncCom::CncCom(QObject *parent) : CncComThread(parent) {
    create();
}

CncCom::~CncCom() {
    close();
}

void CncCom::create() {
    connect(this, &CncComThread::response, this, &CncCom::onResponse);
    connect(this, &CncComThread::timeout, this, &CncCom::onTimeout);
}

void CncCom::clear() {
    CncComThread::clear();
    m_writeData.clear();
    m_readData.clear();
    m_wraddr = m_rdaddr = 0;
    m_rdLength = 0;
    m_reqPack.clear();    
    m_rdCommand = ComPacket::Command::CMD_IDLE;
    m_lastError.clear();
    m_state = IDLE;
}

// by 255 bytes
bool CncCom::writeReqAsync(uint32_t& addr, std::deque<uint8_t>& bytes, const size_t length) {
    if (!isOpen()) {
        errorResponse("COM port isn't opened");
    } else if ((addr & 0xF0000000) != 0) {
        errorResponse( string_format("Address error: 0x%08x", int(addr)) );
    } else if (length == 0) {
        errorResponse( "Sending pack size is 0 bytes" );
    } else if (length > ComPacket::MAX) {
        errorResponse("Packet size is bigger than 255 bytes");
    } else if (length > bytes.size()) {
        errorResponse( string_format("Packet size bigger than available data. Data size: %d, Required packet size: %d", int(addr), int(length)) );
    } else {
        std::vector<uint8_t> v(length); // TODO: use iterators or templete
        for (size_t i = 0; i < length && bytes.size() != 0; i++) {
            v[i] = bytes.front();
            bytes.pop_front();
        }

        m_reqPack.createWritePacket(addr, v, 0, v.size());
        transaction(m_reqPack);
        addr += v.size();

#ifdef PRINT_CNC_COM_DEBUG
        qDebug("Write to address 0x%08x (async):\n", m_reqPack.addr());
        auxItems::print_array(m_reqPack.rawData(), m_reqPack.rawSize());
#endif
        return true;
    }

    return false;
}

bool CncCom::writeAsync(const uint32_t addr, const std::vector<uint8_t> &data, bool block) {
    const size_t max = ComPacket::MAX;
    bool OK = false;
    size_t size = data.size();
    clear();

    if (!size) {
        m_lastError = "No write data";
    } else if (!block && !asyncReady()) {
        m_lastError = "CNC thread busy";
    } else {
        m_wraddr = addr;
        m_writeData = deque<uint8_t>(data.begin(), data.end());
        m_state = WRITE;

        if (size > max)
            size = max;

        OK = this->writeReqAsync(m_wraddr, m_writeData, size);
        m_lastError = CncComThread::message();
    }
    return OK;
}

bool CncCom::writeAsync() {
    const size_t max = ComPacket::MAX;
    bool OK = false;
    size_t size = m_writeData.size();

    if (size && m_state == WRITE) {
        if (size > max)
            size = max;

        OK = this->writeReqAsync(m_wraddr, m_writeData, size);
        m_lastError = CncComThread::message();
    } else {
        clear();
        m_lastError = "No write data";
    }
    return OK;
}

void CncCom::write16Async(const uint32_t addr, const uint16_t data, bool block) {
    const uint8_t* const ptr = reinterpret_cast<uint8_t*>(data);
    vector<uint8_t> v = {ptr[0], ptr[1]};
    writeAsync(addr, v, block);
}

void CncCom::write32Async(const uint32_t addr, const uint32_t data, bool block) {
    const uint8_t* const ptr = reinterpret_cast<uint8_t*>(data);
    vector<uint8_t> v = {ptr[0], ptr[1], ptr[2], ptr[3]};
    writeAsync(addr, v, block);
}

void CncCom::write48Async(const uint32_t addr, const uint64_t data, bool block) {
    const uint8_t* const ptr = reinterpret_cast<uint8_t*>(data);
    vector<uint8_t> v = {ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]};
    writeAsync(addr, v, block);
}

void CncCom::write64Async(const uint32_t addr, const uint64_t data, bool block) {
    const uint8_t* const ptr = reinterpret_cast<uint8_t*>(data);
    vector<uint8_t> v = {ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7]};
    writeAsync(addr, v, block);
}

// by 255 bytes
bool CncCom::readReqAsync(uint32_t& addr, const size_t length, const ComPacket::Command cmd) {
    if (!isOpen()) {
        errorResponse("COM port isn't opened");
    } else if ((addr & 0xF0000000) != 0) {
        errorResponse( string_format("Address error: 0x%08x", int(addr)) );
    } else if (length == 0) {
        errorResponse( "Sending pack size is 0 bytes" );
    } else if (length > ComPacket::MAX) {
        errorResponse("Packet size is bigger than 255 bytes");
    } else if (!(cmd == ComPacket::Command::CMD_READ || cmd == ComPacket::Command::CMD_READ_FIFO)) {
        errorResponse("Incorrect command");
    } else {
        m_reqPack.createReadPacket(cmd, addr, length);
        transaction(m_reqPack);
        if (cmd == ComPacket::Command::CMD_READ) {
            addr += length;
        }
#ifdef PRINT_CNC_COM_DEBUG
        qDebug("Read from address 0x%08x:\n", m_reqPack.addr());
        auxItems::print_array(m_reqPack.rawData(), m_reqPack.rawSize());
#endif
        return true;
    }
    return false;
}

bool CncCom::readAsync(const uint32_t addr, const size_t length, const ComPacket::Command cmd, bool block) {
    const size_t max = ComPacket::MAX;
    bool OK = false;
    clear();
    size_t size = length;

    if (!size) {
        m_lastError = "No read data";
    } else if (!block && !asyncReady()) {
        m_lastError = "CNC thread busy";
    } else if (!(cmd == ComPacket::Command::CMD_READ || cmd == ComPacket::Command::CMD_READ_FIFO)) {
        m_lastError = "Invalid write command";
    } else {
        m_rdaddr = addr;
        m_rdLength = size;
        m_state = cmd == ComPacket::Command::CMD_READ_FIFO ? READ_FIFO : READ_BYTES;

        if (size > max)
            size = max;

        OK = this->readReqAsync(m_rdaddr, size, cmd);
        m_rdLength -= size;
        m_lastError = CncComThread::message();
    }
    return OK;
}

bool CncCom::readAsync() {
    const size_t max = ComPacket::MAX;
    bool OK = false;
    size_t size = m_rdLength;

    if (size && m_state >= READ_BYTES && m_state <= READ64) {
        if (size > max) {
            size = max;
        }

        OK = this->readReqAsync(m_rdaddr, size, m_reqPack.command());
        m_rdLength -= size;        
        m_lastError = CncComThread::message();
    } else {
        clear();
        m_lastError = "No read data";
    }
    return OK;
}

bool CncCom::readFifoAsync(uint32_t addr, size_t length, bool block) {
    return readAsync(addr, length, ComPacket::Command::CMD_READ_FIFO, block);
}

bool CncCom::readArray32Async(uint32_t addr, size_t length32, bool block) {
    const size_t max = ComPacket::MAX;
    bool OK = false;
    clear();
    size_t size = length32<<2;

    if (!size) {
        m_lastError = "No read data";
    } else if (!block && !asyncReady()) {
        m_lastError = "CNC thread busy";
    } else {
        m_state = READ_ARRAY32;
        m_rdaddr = addr & ~3U;
        m_rdLength = size;

        if (size > max)
            size = max;

        OK = this->readReqAsync(m_rdaddr, size);
        m_rdLength -= size;
    }
    return OK;
}

bool CncCom::read16Async(uint32_t addr, bool block) {
    bool OK = false;
    clear();

    if (!block && !asyncReady()) {
        m_lastError = "CNC thread busy";
    } else {
        m_state = READ16;
        m_rdaddr = addr & ~1U;
        m_rdLength = 2;

        OK = this->readReqAsync(m_rdaddr, m_rdLength);
        m_lastError = CncComThread::message();
    }
    return OK;
}

bool CncCom::read32Async(const uint32_t addr, bool block) {
    bool OK = false;
    clear();

    if (!block && !asyncReady()) {
        m_lastError = "CNC thread busy";
    } else {
        m_state = READ32;
        m_rdaddr = addr & ~3U;
        m_rdLength = 4;

        OK = this->readReqAsync(m_rdaddr, m_rdLength, ComPacket::Command::CMD_READ);
        m_lastError = CncComThread::message();
    }
    return OK;
}

bool CncCom::read48Async(const uint32_t addr, bool block) {
    bool OK = false;
    clear();

    if (!block && !asyncReady()) {
        m_lastError = "CNC thread busy";
    } else {
        m_state = READ48;
        m_rdaddr = addr & ~3U;
        m_rdLength = 6;

        OK = this->readReqAsync(m_rdaddr, m_rdLength, ComPacket::Command::CMD_READ);
        m_lastError = CncComThread::message();
    }
    return OK;
}

bool CncCom::read64Async(const uint32_t addr, bool block) {
    bool OK = false;
    clear();

    if (!block && !asyncReady()) {
        m_lastError = "CNC thread busy";
    } else {
        m_state = READ64;
        m_rdaddr = addr & ~3U;
        m_rdLength = 8;

        OK = this->readReqAsync(m_rdaddr, m_rdLength, ComPacket::Command::CMD_READ);
        m_lastError = CncComThread::message();
    }
    return OK;
}

void CncCom::errorResponse(const std::string& s) {
    m_lastError = s;

    switch (m_state) {
    case IDLE:
        break;
    case WRITE:
        emit writeResult(false);
        break;
    case READ_BYTES:
        emit readResult(vector<uint8_t>());
        break;
    case READ_FIFO:
        emit readFifoResult(vector<uint8_t>());
        break;
    case READ_ARRAY32:
        emit readArray32Result(vector<uint32_t>());
        break;
    case READ16:
        emit read16Result(0, false);
        break;
    case READ32:
        emit read32Result(0, false);
        break;
    case READ48:
        emit read48Result(0, false);
        break;
    case READ64:
        emit read64Result(0, false);
        break;
    }

    clear();
}

// TRUE - Okey
bool CncCom::checkResponse(const ComPacket &rxPack) {
    if (rxPack.command() != m_reqPack.command()) {
        errorResponse( string_format("Response. Command error: 0x%x", int(rxPack.command())) );
    } else if (rxPack.addr() != m_reqPack.addr()) {
        errorResponse( string_format("Response. Address error: 0x%x", int(rxPack.addr())) );
    } else if (rxPack.size() != m_reqPack.size() || rxPack.size() == 0) {
        errorResponse( string_format("Response. Size error: %d", int(rxPack.size())) );
    } else {
        return true;
    }
    return false;
}

bool CncCom::readResponse(const ComPacket &rxPack) {
    if (checkResponse(rxPack)) {
        for (size_t i = 0; i < rxPack.size(); i++)
            m_readData.push_back(rxPack.data(i));

        return true;
    }

    return false;
}

void CncCom::onResponse(const ComPacket &rxPack) {
    if (rxPack.command() == ComPacket::Command::CMD_ERROR) {
        errorResponse("CNC sent an error code");
    } else if (rxPack.error()) {
        switch (rxPack.error_code()) {
        case 1: errorResponse("Response. Unknown command"); break;
        case 2: errorResponse("Response. CRC Error"); break;
        case 3: errorResponse("Response. Timeout Error"); break;
        default: errorResponse("Response. Unknown Error"); break;
        }
    } else {
        switch (m_state) {
        case IDLE:
            errorResponse("onResponse. Invalid state");
            break;
        case WRITE:
            if (checkResponse(rxPack)) {
                if (m_writeData.empty()) {
                    emit writeResult(true);
                    clear();
                } else {
                    writeAsync();
                }
            }
            else {
                errorResponse("Unexpected response");
            }
            break;
        case READ_BYTES:
            if (readResponse(rxPack)) {
                if (m_readData.size() == m_rdLength) {
                    emit readResult(m_readData); // Another slot is used for timeout
                    clear();
                } else {
                    readAsync();
                }
            } else {
                errorResponse("Unexpected response");
            }
            break;
        case READ_FIFO:
            if (readResponse(rxPack)) {
                if (m_readData.size() == m_rdLength ) {
                    emit readFifoResult(m_readData);
                    clear();
                } else {
                    readAsync();
                }
            } else {
                errorResponse("Unexpected response");
            }
            break;
        case READ_ARRAY32:
            if (readResponse(rxPack)) {
                if (m_readData.size() == m_rdLength ) {
                    static vector<uint32_t> v;

                    v.resize(m_readData.size() >> 2);

                    memcpy(v.data(), m_readData.data(), v.size());

                    emit readArray32Result(v);
                    clear();
                } else {
                    readAsync();
                }
            } else {
                errorResponse("Unexpected response");
            }
            break;
        case READ16:
            if (readResponse(rxPack)) {
                if (m_readData.size() >= 2) {
                    emit read16Result(BitConverter::toUInt16(m_readData, 0), true);
                    clear();
                } else {
                    errorResponse("Unexpected response");
                }
            } else {
                errorResponse("Unexpected response");
            }
            break;
        case READ32:
            if (readResponse(rxPack)) {
                if (m_readData.size() >= 4) {
                    emit read32Result(BitConverter::toUInt32(m_readData, 0), true);
                    clear();
                } else {
                    errorResponse("Unexpected response");
                }
            } else {
                errorResponse("Unexpected response");
            }
            break;
        case READ48:
            if (readResponse(rxPack)) {
                if (m_readData.size() >= 6) {
                    emit read48Result(BitConverter::toUInt48(m_readData, 0), true);
                    clear();
                } else {
                    errorResponse("Unexpected response");
                }
            } else {
                errorResponse("Unexpected response");
            }
            break;
        case READ64:
            if (readResponse(rxPack)) {
                if (m_readData.size() >= 8) {
                    emit read64Result(BitConverter::toUInt64(m_readData, 0), true);
                    clear();
                } else {
                    errorResponse("Unexpected response");
                }
            } else {
                errorResponse("Unexpected response");
            }
            break;
        }
    }
}

void CncCom::onError(const string& s) {
    errorResponse("CNC Error: " + s);
}

void CncCom::onTimeout(const QString &s) {
    errorResponse("CNC Timeout: " + s.toStdString());
}
