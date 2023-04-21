#include <clocale>
#include "string_parser.h"

using namespace std;

StringParser::StringParser(const string& line) : m_str(line), m_pos(0) {
    std::setlocale(LC_NUMERIC, "C");
//    std::setlocale(LC_NUMERIC, "en_US.UTF-8");
}
StringParser::StringParser(const StringParser& other) : m_str(other.m_str), m_pos(0) {}
StringParser::StringParser(StringParser&& other) noexcept : m_str(std::move(other.m_str)), m_pos(0) {}

bool StringParser::getChar(char& ch) {
    while (m_pos < m_str.length()) {
        const char& c = m_str.at(m_pos);

        if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f')
            m_pos++;
        else if (c == '\0')
            return false;
        else {
            ch = c;
            m_pos++;
            return true;
        }
    }
    return false;
}

//static int read_uint(int* value) {
//    char* new_pos;
//    *value = (int)strtoul(cur_str, &new_pos, 10);

//    if (cur_str != new_pos) {
//        cur_str = new_pos;
//        return 1;
//    }

//    return 0;
//}

bool StringParser::getInt(int &d) {
    size_t len;

    try {
        d = stoi(m_str.substr(m_pos), &len);
        if (len != 0) {
            m_pos += len;
            return true;
        }
    } catch(...) {
        return false;
    }

    return false;
}

//static int read_double(double* value) {
//	char* new_pos;
//	*value = strtod(cur_str, &new_pos);

//	if (cur_str != new_pos) {
//		cur_str = new_pos;
//		return 1;
//	}

//	return 0;
//}

bool StringParser::getDouble(double &d) {
    size_t len;
    try {
        d = stod(m_str.substr(m_pos), &len);
        if (len != 0) {
            m_pos += len;
            return true;
        }
    } catch (...) {
        return false;
    }

    return false;
}

void StringParser::removeForwardSpaces() {
    size_t i;

    for (i = 0; i < m_str.length(); i++) {
        const char& c = m_str.at(i);

        if (!(c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f'))
            break;
    }

    m_str = m_str.substr(i);
    m_pos = 0;
}

void StringParser::removeRearSpaces() {
    int i;

    for (i = static_cast<int>(m_str.length()) - 1; i >= 0 ; i--) {
        const char& c = m_str.at(static_cast<size_t>(i));

        if (!(c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f'))
            break;
    }

    if (i >= 0) {
        m_str = m_str.substr(0, static_cast<size_t>(i + 1));

        if (m_pos > static_cast<size_t>(i))
            m_pos = static_cast<size_t>(i) + 1;
    }
}

void StringParser::removeForwardRearSpaces() {
    removeForwardSpaces();
    removeRearSpaces();
}
