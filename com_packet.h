#ifndef COMPACKET_H
#define COMPACKET_H

#include <cstdint>
#include <vector>
#include <string>
#include <QSerialPort>

class ComPacket {
public:
    enum Command: int {CMD_IDLE, CMD_READ, CMD_WRITE, CMD_READ_FIFO, CMD_READ_BURST, CMD_WRITE_BURST, CMD_ERROR = 0xF};
//    static constexpr unsigned MAX = ( (256 - 9) & 0xFC ); // should be <= 255, align by 4 bytes
    static constexpr unsigned MAX = 252; // align by 4 bytes
    static constexpr unsigned SIZE = MAX + 4 + 1 + 4;    
private:
    std::vector<uint8_t> m_data = std::vector<uint8_t>(SIZE);
    uint32_t m_addr = 0, m_crc32 = 0;
    size_t m_size = 0, m_i = 0;
    Command m_command = CMD_IDLE;
    bool m_valid = false;
    int m_error = 0;
    QSerialPort::SerialPortError m_port_error;
    QString m_port_error_string;

public:
    ComPacket();
    void clear();
    void append(uint8_t data);
    void finalize();

    inline bool complete() const { return m_valid; }
    inline bool empty() const { return m_size == 0; };
    inline size_t size() const { return m_size; }
    inline Command command() const { return m_command; }

    inline bool error() const { return m_error != 0 && m_port_error == QSerialPort::NoError; }
    inline int error_code() const { return m_error; }

    inline QSerialPort::SerialPortError serialPortError() const { return m_port_error; }
    inline const QString& serialPortErrorString() const { return m_port_error_string; }
    inline void setSerialPortError(const QSerialPort::SerialPortError& code, const QString& desc) {
        m_port_error = code;
        m_port_error_string = desc + " (" + QString::number(code) + ")";
    }

    inline uint32_t addr() const { return m_addr; }
    uint8_t data(size_t i) const;
    inline const uint8_t* data() const { return &m_data[5]; }

    uint8_t rawData(size_t i) const;
    const uint8_t* rawData() const { return m_data.data(); }
    size_t rawSize() const { return m_i; }

    static uint32_t crc32(const uint8_t buf[], size_t len);
//    static uint32_t toUInt32Rev(const uint8_t* const data, size_t pos);
    static uint32_t toUInt32Rev(const std::vector<uint8_t>& data, size_t pos);
//    static void toBytesRev(uint8_t* const dst, size_t pos, uint32_t src);
    static void toBytesRev(std::vector<uint8_t>& dst, size_t pos, uint32_t data);

    void createWritePacket(uint32_t addr, const std::vector<uint8_t>& bytes, size_t begin, size_t length, bool async = false);
    void createReadPacket(Command cmd, uint32_t addr, size_t length);
    void createBurstReadPacket(uint32_t addr, size_t length);

    std::string toString() const;
};

#endif // COMPACKET_H
