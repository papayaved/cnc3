#include "com_packet.h"
#include "aux_items.h"
#include  <cstring>

using namespace std;

ComPacket::ComPacket() {
    clear();
}

void ComPacket::clear() {
    memset(m_data.data(), 0, m_data.size());
    m_addr = 0;
    m_crc32 = 0;
    m_size = 0;
    m_i = 0;
    m_command = CMD_IDLE;
    m_valid = false;
    m_error = 0;
}

void ComPacket::append(uint8_t data) {
    if (!m_valid) {
        m_data[m_i++] = data;

        if (m_i == 1) {
           uint8_t cmd = m_data[0] >> 4;
           if (cmd == Command::CMD_READ || cmd == Command::CMD_WRITE || cmd == Command::CMD_READ_BURST || cmd == Command::CMD_ERROR)
               m_command = static_cast<Command>(cmd);
           else {
               m_error = 1;
               m_valid = true;
           }
        } else if (m_i == 4) {
            m_addr = toUInt32Rev(m_data, 0) & 0x0fffFFFF;
        } else if (m_i == 5) {
            m_size = m_data[4];
            if (m_size == 0)
                m_size = ComPacket::MAX;
        } else if ((m_command == Command::CMD_WRITE || m_command == Command::CMD_ERROR) && m_i == 9) {
            m_crc32 = toUInt32Rev(m_data, 5);
            uint32_t crc32_calc = crc32(m_data.data(), 5);

            if (m_crc32 != crc32_calc)
                m_error = 2;

            m_valid = true;
        } else if ((m_command == Command::CMD_READ || m_command == Command::CMD_READ_BURST) && m_i == (m_size + 9)) {
            m_crc32 = toUInt32Rev(m_data, m_size + 5);
            uint32_t crc32_calc = crc32(m_data.data(), m_size + 5);

            if (m_crc32 != crc32_calc)
                m_error = 2;

            m_valid = true;
        }
    }
}

void ComPacket::finalize() {
    m_error = 3;
    m_valid = true;
}

uint8_t ComPacket::data(size_t i) const { return rawData(i + 6); }

uint8_t ComPacket::rawData(size_t i) const {
    return i < SIZE ? m_data[i] : 0;
}

uint32_t ComPacket::crc32(const uint8_t buf[], size_t len) {
    uint32_t crc = ~0U;

    for (size_t pos = 0; pos < len; pos++) {
        crc ^= uint32_t(buf[pos]);

        for (int i = 8; i != 0; i--) {
            if ((crc & 1) != 0) {
                crc >>= 1;
                crc ^= 0xEDB88320;
            }
            else
                crc >>= 1;
        }
    }

    return crc;
}

// Read uint32_t in reverse byte order from the vector, starting at the given position
//uint32_t ComPacket::toUInt32Rev(const uint8_t* const data, size_t pos) {
//    uint32_t res;
//    uint8_t* const ptr = reinterpret_cast<uint8_t*>(&res);
//    ptr[3] = data[pos++];
//    ptr[2] = data[pos++];
//    ptr[1] = data[pos++];
//    ptr[0] = data[pos];
//    return res;
//}

uint32_t ComPacket::toUInt32Rev(const std::vector<uint8_t>& data, size_t pos) {
    uint32_t res;
    uint8_t* const ptr = reinterpret_cast<uint8_t*>(&res);
    ptr[3] = data[pos++];
    ptr[2] = data[pos++];
    ptr[1] = data[pos++];
    ptr[0] = data[pos];
    return res;
}

// Append to the end of a vector in reverse byte order
//void ComPacket::toBytesRev(uint8_t* const dst, size_t pos, uint32_t src) {
//    const uint8_t* const ptr = reinterpret_cast<uint8_t*>(&src);
//    dst[pos++] = ptr[3];
//    dst[pos++] = ptr[2];
//    dst[pos++] = ptr[1];
//    dst[pos] = ptr[0];
//}

void ComPacket::toBytesRev(std::vector<uint8_t>& dst, size_t pos, uint32_t data) {
    const uint8_t* const ptr = reinterpret_cast<uint8_t*>(&data);
    dst[pos++] = ptr[3];
    dst[pos++] = ptr[2];
    dst[pos++] = ptr[1];
    dst[pos] = ptr[0];
}

void ComPacket::createWritePacket(uint32_t addr, const std::vector<uint8_t>& bytes, const size_t begin, const size_t length, bool async) {
    clear();

    if (length) {
        addr &= 0x0fffFFFF;

        m_size = length > ComPacket::MAX ? ComPacket::MAX : length;

        if (begin + m_size > bytes.size())
            m_size = bytes.size() - begin;

        if (m_size) {
            uint32_t data = (async ? CMD_WRITE_BURST : CMD_WRITE) << 28 | addr;
            toBytesRev(m_data, 0, data);

            m_data[4] = m_size & 0xFF;

            memcpy(&m_data[5], &bytes[begin], m_size);

            m_i = m_size + 5;
            m_crc32 = crc32(m_data.data(), m_i);

            toBytesRev(m_data, m_i, m_crc32);
            m_i += sizeof(uint32_t);
        }
    }
}

void ComPacket::createReadPacket(Command cmd, uint32_t addr, size_t length) {
    clear();

    if (length != 0 && (cmd == Command::CMD_READ || cmd == Command::CMD_READ_FIFO)) {
        addr &= 0x0fffFFFF;

        if (length > ComPacket::MAX)
            length = ComPacket::MAX;

        uint32_t data = cmd << 28 | addr;
        toBytesRev(m_data, 0, data);

        m_data[4] = length & 0xFF;

        m_crc32 = crc32(m_data.data(), 5);
        toBytesRev(m_data, 5, m_crc32);

        m_i = 9;
    }
}

void ComPacket::createBurstReadPacket(uint32_t addr, size_t length) {
    clear();

    if (length && length <= UINT32_MAX) {
        addr &= 0x0fffFFFF;

        uint32_t data = Command::CMD_READ_BURST << 28 | addr;
        toBytesRev(m_data, 0, data);

        m_data[4] = 4;

        memcpy(&m_data[5], &length, 4);

        m_crc32 = crc32(m_data.data(), 9);
        toBytesRev(m_data, 9, m_crc32);

        m_i = 13;
    }
}

string ComPacket::toString() const {
    return auxItems::toString(m_data.data(), m_i);
}
