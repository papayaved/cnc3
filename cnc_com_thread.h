#ifndef CNCCOMASYNC_H
#define CNCCOMASYNC_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QSerialPort>
#include <vector>
#include <string>

#include "aux_items.h"
#include "com_packet.h"
#include "main.h"

#define PRINT_CNC_COM_DEBUG

// Class provides communication with CNC
class CncComThread : public QThread {
    Q_OBJECT

    static constexpr int TIMEOUT = 1000; // ms
    static constexpr int RW_ATT = 3;

    QSerialPort* m_port = nullptr;
    ComPacket m_txpack, m_rxpack;
    std::vector<uint8_t> m_wrbytes255, m_rdbytes255;
    std::vector<uint8_t> m_rdbytes;
    QByteArray m_responseData;

    std::string m_lastError;
    std::string m_message;    

    QMutex m_mutex, m_mutexPort;
    QWaitCondition m_cond;
    bool m_quit = false, m_ready = false;
    ComPacket m_request;

public:
    explicit CncComThread(QObject *parent = nullptr);
    ~CncComThread();

    //    bool open(const std::string& portName); // Linux: "/dev/ttyACM0" || Win: "COM6"
    bool open(); // open use STM vendor ID
    bool isOpen();
    void close();
    void reconnect();
    bool asyncReady();

    void clear();

    bool hasError() const { return !m_lastError.empty(); }
    const std::string& lastError() const { return m_lastError; }

private:
    int readPacket(ComPacket& rxd, int timeout_ms = 30000);

    bool _write255(uint32_t addr, const std::vector<uint8_t>& bytes);
    bool write255(uint32_t addr, const std::vector<uint8_t>& bytes, unsigned att = 0);
    bool write255(uint32_t addr, const uint8_t* bytes, size_t size, size_t begin, size_t length);

    bool _write255Burst(uint32_t addr, const std::vector<uint8_t>& bytes);
    bool write255Burst(uint32_t addr, const std::vector<uint8_t>& bytes, unsigned att = 0);
    bool write255Burst(uint32_t addr, const uint8_t* bytes, size_t size, size_t begin, size_t length);

    bool _read255(uint32_t addr, size_t length, ComPacket::Command command, std::vector<uint8_t>& data);
    bool read255(uint32_t addr, size_t length, ComPacket::Command command, std::vector<uint8_t>& data, unsigned att = 0);

    bool _readBurstReq(uint32_t addr, size_t length);
    bool readBurstReq(uint32_t addr, size_t length, unsigned att = 0);

    bool readBurstPacket(ComPacket& pack);

    void run() override;

public:
    bool write(uint32_t addr, const void* const data, size_t size);
    bool writeBurst(uint32_t addr, const void* const data, size_t size);

    bool write(uint32_t addr, const std::vector<uint8_t>& bytes);
    bool writeBurst(uint32_t addr, const std::vector<uint8_t>& bytes);

    bool write32(uint32_t addr, const std::vector<uint32_t>& data);

    bool write16(uint32_t addr, uint16_t data);
    bool write32(uint32_t addr, uint32_t data);
    bool writeFloat(uint32_t addr, float data);
    bool write48(uint32_t addr, uint64_t data);
    bool write64(uint32_t addr, uint64_t data);

    bool read(uint32_t addr, size_t len, std::vector<uint8_t>& data);
    bool readBurst(uint32_t addr, size_t length, std::vector<uint8_t>& data);

    bool readArray32(uint32_t addr, size_t len, std::vector<uint32_t>& data);
    bool readFifo255(uint32_t addr, size_t length, std::vector<uint8_t>& data);

public:
    bool read16(uint32_t addr, uint16_t& data);
    bool read32(uint32_t addr, uint32_t& data);
    bool readFloat(uint32_t addr, float& data);
    bool read48(uint32_t addr, uint64_t& data);
    bool read64(uint32_t addr, uint64_t& data);

protected:
    void transaction(const ComPacket& request);

    inline std::string message() {
        std::string s;
        s.swap(m_message);
        return s;
    }

signals:
    void error(const std::string& s);

    void response(const ComPacket& rxpack);    
    void timeout(const QString& s);
    void writeBurstProcessing(int pct);
    void readBurstProcessing(int pct);
};

#endif // CNCCOMASYNC_H
