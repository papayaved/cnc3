#ifndef STRING_PARSER_H
#define STRING_PARSER_H

#include <string>

class StringParser {
private:
    std::string m_str;
    size_t m_pos;

public:
    StringParser(const std::string& line);
    StringParser(const StringParser& other);
    StringParser(StringParser&& other) noexcept;

    bool getChar(char& ch);
    bool getInt(int& d);
    bool getDouble(double& d);
    inline const std::string& getString() const {return m_str; }

    void removeForwardSpaces();
    void removeRearSpaces();
    void removeForwardRearSpaces();

    inline bool empty() const {return m_str.empty(); }
};

#endif // STRING_PARSER_H
