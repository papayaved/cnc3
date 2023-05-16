#ifndef AUX_ITEMS_H
#define AUX_ITEMS_H

#include <cstdint>
#include <vector>
#include <list>
#include <string>

#include <memory>
#include <cstdio>
#include <cstdlib>

#include <cstdarg>
//#include <cstring>

#include <iostream>
#include <sstream>

#include <cmath>

#include <QTextEdit>
#include <stdexcept>

#include "com_packet.h"
#include "dxf_entity.h"
#include "dxf_arc.h"
#include "dxf_line.h"

// Collection of auxiliary functions and classes for the project

namespace auxItems {
    void byte_reverse(uint32_t& data);
    void swap(uint8_t& a, uint8_t& b);

    std::string toString(const uint8_t * const bytes, size_t size);
    void print_array(const uint8_t* const bytes, size_t size);
    void print_vector(const std::vector<uint8_t>& bytes);

    void print_strings(const std::list<std::string>& ss);

    void push_back_range(std::vector<uint8_t>& dst, const std::vector<uint8_t>& src, const size_t& src_begin, const size_t& src_length);
    void push_back_range(std::vector<uint8_t>& dst, const ComPacket& src);
    void push_back_range(std::vector<uint8_t>& dst, const std::string& str);
    void push_back_range(std::vector<uint8_t>& dst, const int32_t& value);

    std::string string_format(const char *format, ...);
//    template<typename ... Args> std::string string_format(const std::string& format, Args ... args) {
//        size_t size = snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
//        std::unique_ptr<char[]> buf( new char[ size ] );
//        snprintf( buf.get(), size, format.c_str(), args ... );
//        return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
//    }

    bool vector_compare(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b);
    std::vector<uint8_t> gen_rnd(size_t size);

    // Class represents conversion from bytes to number types
    class BitConverter {
    public:
        static uint32_t toUInt32Rev(const std::vector<uint8_t>& v, size_t pos);
        static bool toBoolean(const std::vector<uint8_t>& v, size_t pos, uint8_t bit = 0);
        static uint16_t toUInt16(const std::vector<uint8_t>& v, size_t pos);
        static uint32_t toUInt32(const std::vector<uint8_t>& v, size_t pos);
        static int32_t toInt32(const std::vector<uint8_t>& v, size_t pos);
        static float toFloat(const std::vector<uint8_t>& v, size_t pos);
        static uint64_t toUInt48(const std::vector<uint8_t>& v, size_t pos);
        static uint64_t toUInt64(const std::vector<uint8_t>& v, size_t pos);
    };

    std::string toString(const bool& value) noexcept;

    // Function print number to string in hex format
    template<typename T> std::string toHex(const T& value) {
        const size_t BUF_SIZE = 64;
        std::string res;

        char* buf = new char[BUF_SIZE];

        if (buf) {
            int n = snprintf(buf, BUF_SIZE, "%X", value);
            if (n > 0) res = std::string(buf, BUF_SIZE);
            delete[] buf;
        }
        return res;
    }

    // My own an exception object. Inherits here is not necessary
    class MyException: public std::runtime_error {
        std::ostringstream m_stream;
    public:
        explicit MyException(std::string s = std::string()) noexcept : std::runtime_error("") {
            m_stream << s;
        }
        MyException(MyException& other) noexcept : std::runtime_error(other) {
            m_stream<<other.toString();
        }
        MyException& operator= (const MyException& other) noexcept {
            std::runtime_error::operator=(other);
            m_stream<<other.toString();
            return *this;
        }
        std::string toString() const { return std::string(what()) + ": " + m_stream.str(); }
    };

    // Class for writing reports to QTextEdit form
    class Reporter {
        QTextEdit* m_txt;
    public:
        explicit Reporter(QTextEdit* const txt = nullptr);
        Reporter(const Reporter& other);
        Reporter& operator=(const Reporter& other);

        // Add link to QTextEdit form
        inline void bind(QTextEdit* const txt) { m_txt = txt; }
        // Remove link to QTextEdit form
        inline void unbind() { m_txt = nullptr; }

        void clear();

        void write(const QString& s);
        void write(const char* const s);
        void write(const std::string& s);

        void writeLine(const QString& s);
        void writeLine(const char* const s);
        void writeLine(const std::string& s);
    };

    void print_strings(Reporter* const txt, const std::list<std::string>& ss);

    // Class help to work with hours, minuts and second
    class HMS {
    public:
        uint8_t m, s;
        int32_t h;

        HMS() : m(0), s(0), h(0) {}

        // Contractor accepts time in milliseconds
        HMS(qint64 ms) { fromMS(ms); }

        // Initialize hours, minuts, seconds using milliseconds
        void fromMS(qint64 ms) {
            if (ms < 0)
                ms = 0;

            qint64 h = ms / (1000 * 60 * 60);
            ms -= h * (60 * 60 * 1000);

            qint64 m = ms / (1000 * 60);
            ms -= m * (60 * 1000);

            qint64 s = ms / 1000;
            ms -= s * 1000;
            if (ms >= 500)
                s++;

            if (h > INT32_MAX)
                h = INT32_MAX;

            this->h = int32_t(h);
            this->m = uint8_t(m);
            this->s = uint8_t(s);
        }
    };

    int32_t double_to_int32(double value);

    size_t sizeOf(const std::list<std::string>& gframes);

    template<typename To, typename From> To convert(From value, To min, To max) {
        if (value < min)
            return min;
        else if (value > max)
            return max;
        else
            return static_cast<To>(value);
    }
}

#endif // AUX_ITEMS_H
