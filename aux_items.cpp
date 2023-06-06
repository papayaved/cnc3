#include "aux_items.h"
#include <QDebug>

#include <cstdarg>  // For va_start, etc.
#include <string>
//#include <memory>    // For std::unique_ptr

using namespace std;

namespace auxItems {
    // Reverse the byte order of a number
    void byte_reverse(uint32_t& data) {
        uint8_t* const p = reinterpret_cast<uint8_t*>(&data);
        swap(p[0], p[3]);
        swap(p[1], p[2]);
    }

    // Swap two bytes. Fast, memory saving version for integer numbers
    void swap(uint8_t& a, uint8_t& b) {
        a = a ^ b;
        b = a ^ b;
        a = b ^ a;
    }

    // Print bytes in HEX format by 16 numbers in row
    string toString(const uint8_t * const bytes, size_t size) {
        size_t i;
        QString s;

        for (i = 0; i < size; i++) {
            s += QString::asprintf("%02X", static_cast<int>(bytes[i]));

            if ((i & 0xF) == 0xF || i == size - 1)
                s += "\n";
            else
                s += " ";
        }

        return s.toStdString();
    }

    // Print bytes in a debug console in HEX format by 16 numbers in row
    void print_array(const uint8_t * const bytes, size_t size) {
        qDebug("%s", toString(bytes, size).c_str());
    }

    // Print a vector to the debug console
    void print_vector(const vector<uint8_t>& data) {
        print_array(data.data(), data.size());
    }

    // Print list of std::strings to debug console
    void print_strings(const list<string>& ss) {
        for (list<string>::const_iterator it = ss.begin(); it != ss.end(); ++it)
            qDebug("%s\n", it->c_str());
    }

    // Print list of std::strings to windows form
    void print_strings(Reporter* const txt, const list<string>& ss) {
        if (txt)
            for (const string& s: ss)
                txt->write( QString::asprintf("%s\n", s.c_str()) );
    }

    // Add data to the vector "dst" to tail from "src"
    void push_back_range(vector<uint8_t>& dst, const vector<uint8_t>& src, const size_t& src_begin, const size_t& src_length) {
//        size_t size = dst.size();
//        dst.resize(dst.size() + src_length);
//        memcpy(&dst[size], &src[src_begin], src_length);

        for (size_t i = src_begin; i < src_begin + src_length; i++)
            dst.push_back(src[i]);
    }

    // Add data to the vector "dst" to tail from "src"
    void push_back_range(std::vector<uint8_t> &dst, const ComPacket &src) {
        size_t size = dst.size();
        dst.resize(dst.size() + src.size());
        memcpy(&dst[size], src.data(), src.size());
    }

    // Add std::string to the end of the vector of bytes
    void push_back_range(vector<uint8_t>& v, const string& s) {
        for (size_t i = 0; i < s.size(); i++)
            v.push_back(static_cast<uint8_t>(s[i]));

        v.push_back(0); // add the sign "end of stiring"
    }

    // Add to the end of the bytes vector the number int32
    void push_back_range(std::vector<uint8_t>& v, const int32_t& value) {
        const uint8_t* p = reinterpret_cast<const uint8_t*>(&value);
        v.push_back(static_cast<uint8_t>(*p++));
        v.push_back(static_cast<uint8_t>(*p++));
        v.push_back(static_cast<uint8_t>(*p++));
        v.push_back(static_cast<uint8_t>(*p));
    }

    /* Compare two vectors of bytes.
     * Function returns true if the vectors are same.
     * Function prints error messages to the debug console
     */
    bool vector_compare(const vector<uint8_t>& a, const vector<uint8_t>& b) {
        if (a.size() != b.size()) {
            qDebug("Sizes aren't equal. size A: %d, size B: %d\n", int(a.size()), int(b.size()));
            return false;
        }

        for (size_t i = 0; i < a.size(); i++)
            if (a[i] != b[i]) {
                qDebug("Arrays aren't equal at %d. A: %d, B: %d\n", int(i), int(a[i]), int(b[i]));
                return false;
            }

        return true;
    }

    // Generate a vector of the given size with random bytes
    vector<uint8_t> gen_rnd(size_t size) {
        vector<uint8_t> v(size);
        srand(0);
        for (size_t i = 0; i < size; i++)
            v[i] = static_cast<uint8_t>(rand());

        return v;
    }

    // Read uint32_t in reverse byte order from the vector, starting at the given position
    uint32_t BitConverter::toUInt32Rev(const vector<uint8_t>& v, size_t pos) {
        uint32_t res;
        uint8_t* const ptr = reinterpret_cast<uint8_t*>(&res);
        ptr[3] = v[pos++];
        ptr[2] = v[pos++];
        ptr[1] = v[pos++];
        ptr[0] = v[pos];
        return res;
    }

    // Read boolean value from the vector at the given byte number and bit number
    bool BitConverter::toBoolean(const vector<uint8_t> &v, size_t pos, uint8_t bit) {
        if (pos < v.size() && bit < 8)
        return (v[pos] & 1<<bit) != 0;

        return false;
    }

    // Read a number from the given byte possition
     uint16_t BitConverter::toUInt16(const vector<uint8_t>& v, size_t pos) {
         uint16_t res;
         if (pos + sizeof(res) <= v.size()) {
            memcpy(&res, &v[pos], sizeof(res));
            return res;
         }
         return 0;
     }

     // Read a number from the given byte possition
     uint32_t BitConverter::toUInt32(const vector<uint8_t>& v, size_t pos) {
         uint32_t res;
         if (pos + sizeof(res) <= v.size()) {
            memcpy(&res, &v[pos], sizeof(res));
            return res;
         }
         return 0;
     }

     // Read a number from the given byte possition
     uint64_t BitConverter::toUInt48(const vector<uint8_t>& v, size_t pos) {
         uint64_t res;
         const size_t size = sizeof(uint32_t) + sizeof(uint16_t);

         if (pos + size <= v.size()) {
            memcpy(&res, &v[pos], size);
            return res;
         }
         return 0;
     }

     // Read a number from the given byte possition
     uint64_t BitConverter::toUInt64(const vector<uint8_t>& v, size_t pos) {
         uint64_t res;
         if (pos + sizeof(res) <= v.size()) {
            memcpy(&res, &v[pos], sizeof(res));
            return res;
         }
         return 0;
     }

     // Read a number from the given byte possition
     int32_t BitConverter::toInt32(const vector<uint8_t>& v, size_t pos) {
         int32_t res;
         if (pos + sizeof(res) <= v.size()) {
            memcpy(&res, &v[pos], sizeof(res));
            return res;
         }
         return 0;
     }

     // Read a number from the given byte possition
     float BitConverter::toFloat(const vector<uint8_t>& v, size_t pos) {
         float res;
         if (pos + sizeof(res) <= v.size()) {
            memcpy(&res, &v[pos], sizeof(res));
            return res;
         }
         return 0;
     }

     // Boolean value to string
     std::string toString(const bool& value) noexcept {
         return value ? "true" : "false";
     }

//     std::string string_format(const char *format, ...) {
//         va_list args1, args2;

//         va_start(args1, format);
//         va_copy(args2, args1);

//         int size = std::vsnprintf(nullptr, 0, format, args1) + 1;
//         va_end(args1);

//         if (size > 0) {
//             std::vector<char> buf(static_cast<unsigned>(size));
//             std::vsnprintf(buf.data(), buf.size(), format, args2);
//             va_end(args2);
//             return string(buf.data());
//         }
//         else {
//             va_end(args2);
//             return string();
//         }
//     }

     // Print message to std::string use printf format. Maximum message size 256 bytes
     std::string string_format(const char *format, ...) {
        const size_t BUF_SIZE = 256;

        va_list args;
        string res;

        va_start(args, format);

        char* buf = new char[BUF_SIZE];

        if (buf != nullptr) {
            int n = std::vsnprintf(buf, BUF_SIZE, format, args);
            if (n > 0 && n < static_cast<int>(BUF_SIZE)) {
                buf[BUF_SIZE-1] = 0;
                res = string(buf);
//                res.shrink_to_fit();
            }
            delete[] buf;
        }
        va_end(args);

        return res;
     }

     Reporter::Reporter(QTextEdit* const txt) : m_txt(txt) {}

     Reporter::Reporter(const Reporter& other) : m_txt(other.m_txt) {}

     Reporter &Reporter::operator=(const Reporter& other) {
         m_txt = other.m_txt;
         return *this;
     }

     void Reporter::clear() {
         if (m_txt)
             m_txt->clear();
     }

     // Write to the end and repaint
     void Reporter::write(const QString& s) {
         if (m_txt) {
             m_txt->append(s);
             m_txt->repaint();
         }
         qDebug() << s;
     }

     void Reporter::write(const char* const s) {
         write( QString(s) );
     }

     void Reporter::write(const string& s) {
         write( QString::fromStdString(s) );
     }

     // Write the string and start a new line
     void Reporter::writeLine(const QString& s) {
         write(s + "\n");
     }

     void Reporter::writeLine(const char* const s) {
         writeLine( QString(s) );
     }

     void Reporter::writeLine(const string& s) {
         writeLine( QString::fromStdString(s) );
     }

     // Convert double into int32_t with rounding and limits
     int32_t double_to_int32(double value) {
         double res = round(value);
         if (res > INT32_MAX)
             res = INT32_MAX;
         else if (res < INT32_MIN)
             res = INT32_MIN;

         return int32_t(res);
     }

     //  Size of a list of strings in CNC program memory. Size including newlines
     size_t sizeOf(const std::list<std::string>& gframes) {
         size_t size = 0;

         for (const string& s : gframes) {
             size += s.size() + 1; // + "/n"
         }

         return size;
     }
}
