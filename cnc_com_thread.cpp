#include "cnc_com_thread.h"

#include <QSerialPortInfo>
#include <QDebug>
#include <QCoreApplication>

#include <iostream>
#include <memory>
#include <iostream>
#include <string>
#include <cstdio>
#include <stdexcept>
//#include <regex>

#include "main.h"
#include "aux_items.h"

using namespace std;
using namespace auxItems;

CncComThread::CncComThread(QObject *parent): QThread(parent), m_wrbytes255(UINT8_MAX), m_rdbytes255(UINT8_MAX) {}

CncComThread::~CncComThread() {
    close();
}

bool CncComThread::open() {
    QString portName;

    // Print all serial porta and open the last suitable
    QList<QSerialPortInfo> list = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo& info: list) {
        if (!info.isNull()) {
            qDebug("name: %s PRODUCT_ID: %x VENDOR_ID: %x", info.portName().toStdString().c_str(), info.productIdentifier(), info.vendorIdentifier());

            if (info.productIdentifier() == ST_PRODUCT_ID && info.vendorIdentifier() == ST_VENDOR_ID) {
                portName = info.portName();
            }
        }
    }

    if (m_port) {
        if (m_port->isOpen())
            m_port->close();

        delete m_port;
        m_port = nullptr;
    }

    if (portName.size() != 0) {
        m_port = new QSerialPort();
        m_port->setPortName(portName);

        // uart port parameters need for correct work, but don't use
        m_port->setBaudRate(QSerialPort::Baud115200);
        m_port->setDataBits(QSerialPort::Data8);
        m_port->setParity(QSerialPort::NoParity);
        m_port->setStopBits(QSerialPort::OneStop);
        m_port->setFlowControl(QSerialPort::NoFlowControl);
        m_port->setReadBufferSize(16 * 1024);

        if (!m_port->open(QIODevice::ReadWrite)) {
            delete m_port;
            m_port = nullptr;
            m_message = QString::asprintf("Port %s is not opened", portName.toStdString().c_str()).toStdString();
        }
        else {
            m_message = QString::asprintf("Port %s is opened", portName.toStdString().c_str()).toStdString();
            // Start thread
//            transaction(ComPacket()); // The thread isn't using
        }

        return m_port != nullptr;
    }
    else {
        m_message = "STM USB CDC device was not found";
        return false;
    }
}

bool CncComThread::isOpen() {
    if (isRunning()) {
        m_mutex.lock();
        bool open = m_port && m_port->isOpen();
        m_mutex.unlock();
        return open && isRunning();
    }

    return m_port && m_port->isOpen();
}

void CncComThread::close() {
    if (isRunning()) {
        m_mutex.lock();
        m_quit = true;
        m_cond.wakeOne();
        m_mutex.unlock();
        wait();
    }

    m_txpack.clear();
    m_rxpack.clear();
    m_wrbytes255.clear();
    m_rdbytes255.clear();
    m_rdbytes.clear();
    m_responseData.clear();

    m_lastError.clear();
    m_message.clear();

    m_quit = m_ready = false;
    m_request.clear();

    if (m_port) {
        if (m_port->isOpen()) {
            m_port->close();
        }

        delete m_port;
        m_port = nullptr;
    }
}

void CncComThread::reconnect() {
    close();
    open();
}

bool CncComThread::asyncReady() {
    m_mutex.lock();
    bool open = m_port && m_port->isOpen();
    bool ready = m_ready;
    m_mutex.unlock();

    return open && isRunning() && ready;
}

// read by packets
// return < 0 - error
int CncComThread::readPacket(ComPacket &rxd, int timeout_ms) {
    rxd.clear();

    if (!m_port) {
        rxd.finalize();
        return -1; // no port
    }

    m_port->clearError();

    if (timeout_ms < 10)
        timeout_ms = 10;

    do {
        if (m_responseData.isEmpty()) {
            if (m_port->waitForReadyRead(timeout_ms)) { // no timeout
                m_responseData += m_port->readAll();

                while (m_port->waitForReadyRead(100))
                    m_responseData += m_port->readAll();
            } else {
                rxd.finalize();

                if (m_port->error() != QSerialPort::NoError) {
                    qDebug("Serial port error %d: %s", m_port->error(), m_port->errorString().toStdString().c_str());
                    rxd.setSerialPortError( m_port->error(), m_port->errorString() );
                }

                return -2; // timeout
            }

            if (m_responseData.size() == 0 && m_port->error() != QSerialPort::NoError)
                qDebug("Serial port error %d: %s", m_port->error(), m_port->errorString().toStdString().c_str());


            qDebug("Rx %d bytes\n", m_responseData.size());
        } else {
            int size_reg;

            do {
                size_reg = m_responseData.size();
                m_responseData += m_port->readAll();

                if (size_reg != m_responseData.size())
                    qDebug("Rem+Rx %d bytes\n", m_responseData.size());
                else
                    break; // no more data
            } while (1);
        }

        for (QByteArray::const_iterator it = m_responseData.cbegin(); it != m_responseData.cend() && !rxd.complete(); ++it)
            rxd.append(*it);            

        if (rxd.complete()) {
            size_t rem = m_responseData.size() - rxd.rawSize();

            if (rem) {
                qDebug("Read remains %d bytes\n", (int)rem);
                m_responseData = m_responseData.right(rem);
            } else
                m_responseData.clear();

            break; // loop's quit is here
        } else
            m_responseData.clear(); // all copied, read more
    } while (1);

#ifdef RX_PRINT
    static int n;

    if (rxd.empty())
        n = 0;

    if (!rxd.empty()) {
        for (size_t i = 0; i < rxd.count(); i++) {
            if (n != 15)
                qDebug("%02X ", int(rxd.rawData(i)));
            else
                qDebug("%02X\n", int(rxd.rawData(i)));
        }
    }
    else {
        if (n != 15)
            qDebug("Empty packet ");
        else
            qDebug("Empty packet\n");
    }
    n++;
#endif

    return rxd.size();
}

void CncComThread::clear() {
    m_lastError.clear();
    m_responseData.clear();

    if (isOpen())
        m_port->clear();
}

// upto 255 bytes
bool CncComThread::_write255(uint32_t addr, const std::vector<uint8_t> &bytes) {
    clear();

    if (bytes.size()) {
        if ((addr & 0xF0000000) != 0) {
            m_lastError = string_format("Write address error: 0x%08x", int(addr));
            return false;
        }

        if (!m_port || !m_port->isOpen()) {
            m_lastError = "COM port isn't opened";
            return false;
        }

        m_txpack.createWritePacket(addr, bytes, 0, bytes.size());

#ifdef PRINT_CNC_COM_DEBUG
       qDebug("Writing to address 0x%08x bytes %d:\n%s\n", (int)addr, (int)bytes.size(), m_txpack.toString().c_str());
#endif

        size_t pos = 0;
        while (pos < m_txpack.rawSize()) {
            int res = m_port->write(reinterpret_cast<const char*>(m_txpack.rawData()) + pos, qint64(m_txpack.rawSize()) - pos);

            if (res < 0) {
                m_lastError = "Port write error";
                return false;
            }

            pos += res;

            if (!m_port->waitForBytesWritten(TIMEOUT)) {
#ifdef PRINT_CNC_COM_DEBUG
                qDebug("Write timeout");
#endif
                m_lastError = "Write timeout";
                return false;
            }

            qDebug("Sent %d bytes\n", (int)pos);
        }

        while (true) {
            int res = readPacket(m_rxpack, TIMEOUT);

            if (res <= 0) {
                m_lastError = "cnc_com_thread::writeBytes>>Rx timeout. Received " + to_string(m_rxpack.rawSize()) + " bytes";
                qDebug("%s", m_lastError.c_str());
                return false;
            }

#ifdef PRINT_CNC_COM_DEBUG
            qDebug("Received %d bytes:\n%s\n", (int)m_rxpack.rawSize(), m_rxpack.toString().c_str());
#endif

            if (m_rxpack.complete()) {
                if (m_rxpack.command() == ComPacket::Command::CMD_ERROR) {
                    m_lastError = "CNC sent an error code";
                    return false;
                }

                if (m_rxpack.error()) {
                    switch (m_rxpack.error_code()) {
                        case 1: m_lastError = "Write acknowledge. Unknown command"; break;
                        case 2: m_lastError = "Write acknowledge. CRC Error"; break;
                        case 3: m_lastError = "Write acknowledge. Timeout Error"; break;
                        default: m_lastError = "Write acknowledge. Unknown Error"; break;
                    }                    
                    return false;
                }

                if (m_rxpack.command() != ComPacket::Command::CMD_WRITE) {
                    m_lastError = string_format("Write acknowledge. Command error: 0x%x", int(m_rxpack.command()));
                    return false;
                }

                if (m_rxpack.addr() != addr) {
                    m_lastError = string_format("Write acknowledge. Address error: 0x%x", int(m_rxpack.addr()));
                    return false;
                }

                if (m_rxpack.size() != bytes.size()) {
                    m_lastError = string_format("Write acknowledge. Length error: %d", int(m_rxpack.size()));
                    return false;
                }

                break;
            }
        }
    }

    return true;
}

// upto 255 bytes
bool CncComThread::write255(uint32_t addr, const std::vector<uint8_t> &bytes, unsigned att) {
    if (isOpen()) {
        qDebug("Write attempt %d", att + 1);

        if ( _write255(addr, bytes) )
            return true;
        else {
            if (++att < RW_ATT) {
                if ( write255(addr, bytes, att) )
                    return true;
            }
        }

        emit error(m_lastError);
    }

    return false;
}

// upto 255 bytes
bool CncComThread::write255(uint32_t addr, const uint8_t* data, size_t size, size_t begin, size_t length) {
    if (begin < size) {
        data += begin;

        if (begin + length > size)
            length = static_cast<uint8_t>(size - begin);

        if (length != 0) {
            if (length > UINT8_MAX)
                length = UINT8_MAX;

            m_wrbytes255.resize(length);
            memcpy(m_wrbytes255.data(), data, m_wrbytes255.size());
            return write255(addr, m_wrbytes255);
        }
    }

    return false;
}

// write without acknowledges
// upto 255 bytes
bool CncComThread::_write255Burst(uint32_t addr, const std::vector<uint8_t> &bytes) {
    clear();

    if (bytes.size()) {
        if ((addr & 0xF0000000) != 0) {
            m_lastError = string_format("Address error: 0x%08x", int(addr));
            return false;
        }

        if (!m_port || !m_port->isOpen()) {
            m_lastError = "COM port isn't opened";
            return false;
        }

        m_txpack.createWritePacket(addr, bytes, 0, bytes.size(), true);

#ifdef PRINT_CNC_COM_DEBUG
       qDebug("Writing burst to address 0x%08x bytes %d:\n%s\n", (int)addr, (int)bytes.size(), m_txpack.toString().c_str());
#endif

        size_t pos = 0;
        while (pos < m_txpack.rawSize()) {
            int res = m_port->write(reinterpret_cast<const char*>(m_txpack.rawData()) + pos, qint64(m_txpack.rawSize()) - pos);

            if (res < 0) {
                m_lastError = "Port write error";
                return false;
            }
            pos += res;

            if (!m_port->waitForBytesWritten(TIMEOUT)) {
#ifdef PRINT_CNC_COM_DEBUG
                qDebug("Write timeout\n");
#endif
                m_lastError = "Write timeout\n";
                return false;
            }

            qDebug("Sent %d bytes\n", (int)pos);
        }
    } else {
#ifdef PRINT_CNC_COM_DEBUG
        qDebug("Data is empty\n");
#endif
    }

    return true;
}

// upto 255 bytes
bool CncComThread::write255Burst(uint32_t addr, const std::vector<uint8_t>& bytes, unsigned att) {
    if (isOpen()) {
        qDebug("Write burst attempt %d", att + 1);

        if ( _write255Burst(addr, bytes) )
            return true;
        else {
            if (++att < RW_ATT) {
                if ( write255Burst(addr, bytes, att) )
                    return true;
            }
        }

        emit error(m_lastError);
    }

    return false;
}

// upto 255 bytes
bool CncComThread::write255Burst(uint32_t addr, const uint8_t* data, size_t size, size_t begin, size_t length) {
    if (begin < size) {
        data += begin;

        if (begin + length > size)
            length = static_cast<uint8_t>(size - begin);

        if (length != 0) {
            if (length > UINT8_MAX)
                length = UINT8_MAX;

            m_wrbytes255.resize(length);
            memcpy(m_wrbytes255.data(), data, m_wrbytes255.size());
            return write255Burst(addr, m_wrbytes255);
        }
    }

    return false;
}

// upto 255 bytes
bool CncComThread::_read255(uint32_t addr, size_t length, ComPacket::Command cmd, vector<uint8_t>& data) {
//    const QMutexLocker portLocker(&m_mutexPort);
    clear();
    data.clear();

    if (length != 0 && (cmd == ComPacket::Command::CMD_READ || cmd == ComPacket::Command::CMD_READ_FIFO)) {
        if ((addr & 0xF0000000) != 0) {
            m_lastError = string_format("Address error: 0x%08x", int(addr));
            return false;
        }

        if (!m_port || !m_port->isOpen()) {
            m_lastError = "COM port isn't opened";
            return false;
        }

        m_txpack.createReadPacket(cmd, addr, length);

#ifdef PRINT_CNC_COM_DEBUG
        qDebug("Reading address 0x%08x, %d bytes:\n%s\n", addr, (int)m_txpack.rawSize(), m_txpack.toString().c_str());
#endif

        size_t pos = 0;
        while (pos < m_txpack.rawSize()) {
            int res = m_port->write(reinterpret_cast<const char*>(m_txpack.rawData()) + pos, qint64(m_txpack.rawSize()) - pos);

            if (res < 0) {
                m_lastError = "Port write error";
                return false;
            }

            pos += res;

            if (!m_port->waitForBytesWritten(TIMEOUT)) {
#ifdef PRINT_CNC_COM_DEBUG
            qDebug("Write timeout\n");
#endif
                m_lastError = "Write timeout";
                return false;
            }

            qDebug("Sent %d bytes\n", (int)pos);
        }

        while (true) {
            int res = readPacket(m_rxpack, TIMEOUT);

            if (res <= 0) {
                switch (res) {
                case 0:
                    m_lastError = "cnc_com_thread::readBytes>>No data";
                    break;
                case -1:
                    m_lastError = "cnc_com_thread::readBytes>>No serial port";
                    break;
                default:
                    m_lastError = "cnc_com_thread::readBytes>>Rx timeout. Received " + to_string(m_rxpack.rawSize()) + " bytes. " + m_rxpack.serialPortErrorString().toStdString();
                    break;
                }

                return false;
            }

#ifdef PRINT_CNC_COM_DEBUG
            qDebug("Received %d bytes:\n%s\n", (int)m_rxpack.rawSize(), m_rxpack.toString().c_str());
#endif
            if (m_rxpack.complete()) {
                if (m_rxpack.command() == ComPacket::Command::CMD_ERROR) {
                    m_lastError = "CNC sent an error code";
                    return false;
                }

                if (m_rxpack.error()) {
                    switch (m_rxpack.error_code()) {
                        case 1: m_lastError = "Read acknowledge. Unknown command"; break;
                        case 2: m_lastError = "Read acknowledge. CRC Error"; break;
                        case 3: m_lastError = "Read acknowledge. Timeout Error"; break;
                        default: m_lastError = "Read acknowledge. Unknown Error"; break;
                    }
                    return false;
                }

                if (m_rxpack.command() != cmd) {
                    m_lastError = string_format("Read acknowledge. Command Error: 0x%x", int(m_rxpack.command()));
                    return false;
                }

                if (m_rxpack.addr() != addr) {
                    m_lastError = string_format("Read acknowledge. Address Error: 0x%x", int(m_rxpack.addr()));
                    return false;
                }

                if (m_rxpack.size() != length) {
                    m_lastError = string_format("Read acknowledge. Length Error: %d", int(m_rxpack.size()));
                    return false;
                }

                break;
            }
        }

        data.resize(length);
        memcpy(data.data(), m_rxpack.data(), data.size());
    } else
        return false;

    return true;
}

bool CncComThread::read255(uint32_t addr, size_t length, ComPacket::Command command, std::vector<uint8_t>& data, unsigned att) {
    if (isOpen()) {
        qDebug("Read attempt %d", att + 1);

        if ( _read255(addr, length, command, data) )
            return true;
        else {
            if (++att < RW_ATT) {
                if ( read255(addr, length, command, data, att) )
                    return true;
            }
        }

        emit error(m_lastError);
    }

    return false;
}

bool CncComThread::_readBurstReq(uint32_t addr, size_t length) {
//    const QMutexLocker portLocker(&m_mutexPort);
    clear();

    if (!length)
        return false;

    if ((addr & 0xF0000000) != 0) {
        m_lastError = string_format("ReadBurstReq address error: 0x%08x", int(addr));
        return false;
    }

    if (!m_port || !m_port->isOpen()) {
        m_lastError = "ReadBurstReq COM port isn't opened";
        return false;
    }

    m_txpack.createBurstReadPacket(addr, length);

#ifdef PRINT_CNC_COM_DEBUG
    qDebug("Read burst address request 0x%08x, %d bytes:\n%s\n", addr, (int)length, m_txpack.toString().c_str());
#endif

    size_t pos = 0;
    while (pos < m_txpack.rawSize()) {
        int res = m_port->write(reinterpret_cast<const char*>(m_txpack.rawData()) + pos, qint64(m_txpack.rawSize()) - pos);

        if (res < 0) {
            m_lastError = "Port write error";
            return false;
        }

        pos += res;

        if (!m_port->waitForBytesWritten(TIMEOUT)) {
#ifdef PRINT_CNC_COM_DEBUG
        qDebug("Write timeout\n");
#endif
            m_lastError = "Write timeout";
            return false;
        }

        qDebug("Sent %d bytes\n", (int)pos);
    }

    return true;
}

bool CncComThread::readBurstReq(uint32_t addr, size_t length, unsigned att) {
    if (isOpen()) {
        qDebug("Read burst attempt %d", att + 1);

        if ( _readBurstReq(addr, length) )
            return true;
        else {
            if (++att < RW_ATT) {
                if ( readBurstReq(addr, length, att) )
                    return true;
            }
        }

        emit error(m_lastError);
    }

    return false;
}

bool CncComThread::readBurstPacket(ComPacket& rxd) {
//    const QMutexLocker portLocker(&m_mutexPort);
    rxd.clear();

    while (true) {
        int res = readPacket(rxd, TIMEOUT);

        if (res <= 0) {
            m_lastError = string("Rx timeout (") + to_string(res) + string("). Received ") + to_string(rxd.rawSize()) + " bytes";
            return false;
        }

#ifdef PRINT_CNC_COM_DEBUG
        qDebug("Received %d bytes:\n%s\n", (int)rxd.rawSize(), rxd.toString().c_str());
#endif
        if (rxd.complete()) {
            if (rxd.command() == ComPacket::Command::CMD_ERROR) {
                m_lastError = "CNC sent an error code";
                return false;
            }

            if (rxd.error()) {
                switch (rxd.error_code()) {
                    case 1: m_lastError = "Read acknowledge. Unknown command"; break;
                    case 2: m_lastError = "Read acknowledge. CRC Error"; break;
                    case 3: m_lastError = "Read acknowledge. Timeout Error"; break;
                    default: m_lastError = "Read acknowledge. Unknown Error"; break;
                }

                return false;
            }

            if (rxd.command() != ComPacket::Command::CMD_READ_BURST) {
                m_lastError = string_format("Read acknowledge. Command Error: 0x%x", int(rxd.command()));
                return false;
            }

            break;
        }
    }

//    m_rdbytes.resize(m_rxpack.size());
//    memcpy(m_rdbytes.data(), m_rxpack.data(), m_rdbytes.size());
    return true;
}

/*
std::vector<uint8_t> CncComThread::readBurst_bak(uint32_t addr, size_t length) {
    const QMutexLocker portLocker(&m_mutexPort);

    if (length) {
        if ((addr & 0xF0000000) != 0)
            throw runtime_error( string_format("Address error: 0x%08x", int(addr)) );

        if (!m_port || !m_port->isOpen())
            throw runtime_error("COM port isn't opened");

        m_txpack.createBurstReadPacket(addr, length);

        m_port->write(reinterpret_cast<const char*>(m_txpack.rawData()), qint64(m_txpack.rawSize()));
        m_port->flush();

#ifdef PRINT_CNC_COM_DEBUG
        qDebug("Read address 0x%08x, %d bytes:\n%s\n", addr, (int)length, m_txpack.toString().c_str());
#endif

        m_rdbytes.resize(length);
        size_t pos = 0;

        while (pos < length) {
            while (1) {
                int res = readPacket(m_rxpack, TIMEOUT);

                if (res <= 0)
                    throw runtime_error( string("Rx (fast) timeout. Received ") + to_string(m_rxpack.rawSize()) + " bytes." + "Error: " + to_string(res) );

#ifdef PRINT_CNC_COM_DEBUG
                qDebug("Received %d bytes:\n%s\n", (int)m_rxpack.rawSize(), m_rxpack.toString().c_str());
#endif
                if (m_rxpack.complete()) { // always complete
                    if (m_rxpack.command() == ComPacket::Command::CMD_ERROR)
                        throw runtime_error("CNC sent an error code");

                    if (m_rxpack.error()) {
                        switch (m_rxpack.error_code()) {
                        case 1: throw runtime_error("Read acknowledge. Unknown command");
                        case 2: throw runtime_error("Read acknowledge. CRC Error");
                        case 3: throw runtime_error("Read acknowledge. Timeout Error");
                        default: throw runtime_error("Read acknowledge. Unknown Error");
                        }
                    }

                    if (m_rxpack.command() != ComPacket::Command::CMD_READ_BURST)
                        throw runtime_error(string_format("Read acknowledge. Command Error: 0x%x", int(m_rxpack.command())));

                    break;
                }
            }

            size_t next_pos = pos + m_rxpack.size();

            if (next_pos <= m_rdbytes.size()) {
                memcpy(&m_rdbytes[pos], m_rxpack.data(), m_rxpack.size());
                pos = next_pos;
                qDebug("Received (fast) %d bytes\n", (int)pos);

                if (pos >= m_rdbytes.size())
                    break;
            } else {
                break;
            }
        }
    } else {
        m_rdbytes.clear();
    }

    return m_rdbytes;
} */

// Start or wake up a transducer thread
void CncComThread::transaction(const ComPacket& request) {
    const QMutexLocker locker(&m_mutex);
    m_request = request;

    if (!isRunning())
        start();
    else
        m_cond.wakeOne();
}

void CncComThread::run() {
    ComPacket response;

    qDebug("CNC thread started");

    m_mutex.lock();
    ComPacket currentRequest = m_request;
    m_ready = false;
    m_mutex.unlock();    

    while (!m_quit) {
        if (!isOpen()) {
            qDebug("CNC thread is closing. Port is not opened");
            return;
        }

        if (!currentRequest.empty()) {
            const QMutexLocker portLocker(&m_mutexPort);

            // write request
            m_port->write(reinterpret_cast<const char*>(m_txpack.rawData()), qint64(currentRequest.rawSize()));
//            m_port->flush();

            if (m_port->waitForBytesWritten(TIMEOUT)) {
                // read response
                while (true) {
                    if (m_port->waitForReadyRead(TIMEOUT)) {
                        QByteArray responseData = m_port->readAll();

                        while (m_port->waitForReadyRead(10))
                            responseData += m_port->readAll();

                        for (QByteArray::iterator it = responseData.begin(); it != responseData.end(); ++it) {
                            response.append(*it);

                            if (response.complete()) {
                                emit this->response(response);
                                response.clear();
                                break;
                            }
                        }
                    } else {
                        emit timeout(tr("Read response timeout"));
                        break;
                    }
                }
            } else {
                emit timeout(tr("Write request timeout"));
            }
        }

        m_mutex.lock();
        m_ready = true;
        qDebug("CNC thread is waiting");
        m_cond.wait(&m_mutex);
        m_ready = false;
        currentRequest = m_request;
        m_mutex.unlock();
    }

    qDebug("CNC thread is quit");
}

bool CncComThread::write(uint32_t addr, const void* const data, const size_t size) {
    const size_t max = ComPacket::MAX;
//    const QMutexLocker portLocker(&m_mutexPort);
    size_t pos = 0, len = 0;
    size_t rem = size;

    while (rem > 0) {
        len = rem > max ? max : rem;

        // write by 255 bytes
        if ( write255(addr, reinterpret_cast<const uint8_t*>(data), size, pos, len) ) {
            addr += len;
            pos += len;
            rem -= len;
        } else
            return false;
    }

    return true;
}

bool CncComThread::writeBurst(uint32_t addr, const void * const data, size_t size) {
    const size_t max = ComPacket::MAX;
//    const QMutexLocker portLocker(&m_mutexPort);
    size_t pos = 0, len = 0;
    size_t rem = size;
    int pct_reg = 0;

    emit writeBurstProcessing(0);

    while (rem > 0) {
        len = rem > max ? max : rem;

        // write by 255 bytes
        if ( write255Burst(addr, reinterpret_cast<const uint8_t*>(data), size, pos, len) ) {
            addr += len;
            pos += len;
            rem -= len;

            int pct = static_cast<int>(round((size - rem) * 100 / size));

            if (pct_reg != pct) {
                pct_reg = pct;
                emit writeBurstProcessing(pct);
            }
        } else
            return false;
    }

    return true;
}

bool CncComThread::write(uint32_t addr, const vector<uint8_t>& bytes) {
    return write(addr, bytes.data(), bytes.size());
}

bool CncComThread::writeBurst(uint32_t addr, const vector<uint8_t>& bytes) {
    return writeBurst(addr, bytes.data(), bytes.size());
}

bool CncComThread::write32(uint32_t addr, const std::vector<uint32_t> &data) {
    return write(addr, data.data(), data.size() << 2);
}

// Data don't reverse
bool CncComThread::write16(uint32_t addr, uint16_t data) {
    return write255(addr, reinterpret_cast<uint8_t*>(&data), sizeof(uint16_t), 0, sizeof(uint16_t));
}

bool CncComThread::write32(uint32_t addr, uint32_t data) {
    return write255(addr, reinterpret_cast<uint8_t*>(&data), sizeof(uint32_t), 0, sizeof(uint32_t));
}

bool CncComThread::writeFloat(uint32_t addr, float data) {
    return write255(addr, reinterpret_cast<uint8_t*>(&data), sizeof(float), 0, sizeof(float));
}

bool CncComThread::write64(uint32_t addr, uint64_t data) {
    return write255(addr, reinterpret_cast<uint8_t*>(&data), sizeof(uint64_t), 0, sizeof(uint64_t));
}

bool CncComThread::write48(uint32_t addr, uint64_t data) {
    return write255(addr, reinterpret_cast<uint8_t*>(&data), sizeof(uint64_t), 0, sizeof(uint16_t) * 3);
}

bool CncComThread::read(uint32_t addr, size_t len, vector<uint8_t>& data) {
    const size_t max = ComPacket::MAX;
    data.resize(len);
    data.clear();

    size_t rem = len;

    while (rem > 0) {
        size_t size = rem > max ? max : static_cast<size_t>(rem);

        if ( read255(addr, size, ComPacket::Command::CMD_READ, m_rdbytes255) ) {
            push_back_range(data, m_rdbytes255, 0, m_rdbytes255.size());
            addr += size;
            rem -= size;
        } else
            return false;
    }

    return true;
}

// 3 attempts to write request but 1 attempt to read
bool CncComThread::readBurst(uint32_t addr, size_t len, vector<uint8_t>& data) {
    data.resize(len);
    data.clear(); // Leaves the capacity
    clear();

    size_t rem = len;
    int pct_reg = 0;

    if (!readBurstReq(addr, len)) // 3 attempts to write, emit error signal if error
        return false;

    while (rem > 0) {
        if ( readBurstPacket(m_rxpack) ) {
            push_back_range(data, m_rxpack);

    //        addr += packet.size();
            rem -= m_rxpack.size();

            int pct = static_cast<int>(round((len - rem) * 100 / len));

            if (pct_reg != pct) {
                pct_reg = pct;
                emit readBurstProcessing(pct);
            }
        } else
            return false;
    }

    return true;
}

bool CncComThread::readFifo255(uint32_t addr, size_t length, vector<uint8_t>& data) {
    return read255(addr, length, ComPacket::Command::CMD_READ_FIFO, data);
}

bool CncComThread::readArray32(uint32_t addr, size_t len, vector<uint32_t>& data) {
    m_rdbytes.resize(len * sizeof(uint32_t)); // todo: use pointers of vector data

    if (read(addr, m_rdbytes.size(), m_rdbytes)) {
        data.resize(len);
        memcpy(data.data(), m_rdbytes.data(), m_rdbytes.size());
        return true;
    }

    data.clear();
    return false;
}

bool CncComThread::read16(uint32_t addr, uint16_t& data) {
    if (read255(addr, sizeof(uint16_t), ComPacket::Command::CMD_READ, m_rdbytes255)) {
        data = BitConverter::toUInt16(m_rdbytes255, 0);
        return true;
    }

    data = 0;
    return false;
}

bool CncComThread::read32(uint32_t addr, uint32_t& data) {
    if (read255(addr, sizeof(uint32_t), ComPacket::Command::CMD_READ, m_rdbytes255)) {
        data = BitConverter::toUInt32(m_rdbytes255, 0);
        return true;
    }

    data = 0;
    return false;
}

bool CncComThread::readFloat(uint32_t addr, float& data) {
    if (read255(addr, sizeof(float), ComPacket::Command::CMD_READ, m_rdbytes255)) {
        data = BitConverter::toFloat(m_rdbytes255, 0);
        return true;
    }

    data = 0;
    return false;
}

bool CncComThread::read64(uint32_t addr, uint64_t& data) {
    if (read255(addr, sizeof(uint64_t), ComPacket::Command::CMD_READ, m_rdbytes255)) {
        data = BitConverter::toUInt64(m_rdbytes255, 0);
        return true;
    }

    data = 0;
    return false;
}

bool CncComThread::read48(uint32_t addr, uint64_t& data) {
    if (read255(addr, sizeof(uint16_t) * 3, ComPacket::Command::CMD_READ, m_rdbytes255)) {
        data = BitConverter::toUInt48(m_rdbytes255, 0);
        return true;
    }

    data = 0;
    return false;
}
