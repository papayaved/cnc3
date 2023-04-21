#ifndef CNCCOM_H
#define CNCCOM_H

#include <deque>
#include <vector>

#include "cnc_com_thread.h"
#include "aux_items.h"

class CncCom : public CncComThread {
    Q_OBJECT

    std::deque<uint8_t> m_writeData;
    std::vector<uint8_t> m_readData;
    uint32_t m_wraddr = 0, m_rdaddr = 0;
    size_t m_rdLength = 0;
    ComPacket::Command m_rdCommand = ComPacket::Command::CMD_IDLE;

    ComPacket m_reqPack;
    std::string m_lastError;

    enum {IDLE, WRITE, READ_BYTES, READ_FIFO, READ_ARRAY32, READ16, READ32, READ48, READ64} m_state;

    void create();
public:
    explicit CncCom(QObject *parent = nullptr);
//    CncComAsync(const aux_func::Reporter& txt, QObject *parent = nullptr);
    ~CncCom();

    void clear();

    inline bool isError() const { return !m_lastError.empty(); }
    inline std::string lastError() const {return m_lastError; }

private:    
    bool writeReqAsync(uint32_t& addr, std::deque<uint8_t>& bytes, size_t length);
public:
    bool writeAsync(uint32_t addr, const std::vector<uint8_t>& data, bool block = true);
private:
    bool writeAsync();

//    bool writeBytes(uint32_t addr, const uint8_t *data, size_t size, size_t begin, size_t length);
//    void write(uint32_t addr, const void* data, size_t size);
//    void write(uint32_t addr, const void* data, size_t size, void (*callback)());

private:
    bool readReqAsync(uint32_t& addr, size_t length, ComPacket::Command cmd = ComPacket::Command::CMD_READ);
public:
    bool readAsync(uint32_t addr, size_t length, ComPacket::Command cmd = ComPacket::Command::CMD_READ, bool block = true);
private:
    bool readAsync();

    void errorResponse(const std::string& s);
    bool checkResponse(const ComPacket& rxPack);
    bool readResponse(const ComPacket& rxPack);

private slots:
    void onResponse(const ComPacket& rxPack);
    void onError(const std::string& s);
    void onTimeout(const QString& s);

public:
    void write16Async(uint32_t addr, uint16_t data, bool block = true);
    void write32Async(uint32_t addr, uint32_t data, bool block = true);
    void write48Async(uint32_t addr, uint64_t data, bool block = true);
    void write64Async(uint32_t addr, uint64_t data, bool block = true);

    bool readFifoAsync(uint32_t addr, size_t length, bool block = true);
    bool readArray32Async(uint32_t addr, size_t length32, bool block = true);
    bool read16Async(uint32_t addr, bool block = true);
    bool read32Async(uint32_t addr, bool block = true);
    bool read48Async(uint32_t addr, bool block = true);
    bool read64Async(uint32_t addr, bool block = true);

signals:
    void writeResult(bool valid);
    void readResult(const std::vector<uint8_t>&);
    void readFifoResult(const std::vector<uint8_t>&);
    void readArray32Result(const std::vector<uint32_t>&);
    void read16Result(uint16_t, bool valid);
    void read32Result(uint32_t, bool valid);
    void read48Result(uint64_t, bool valid);
    void read64Result(uint64_t, bool valid);
};

#endif // CNCCOM_H
