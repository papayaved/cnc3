#include "gframe.h"

#include <regex>
#include <QString>

#include "string_parser.h"
#include <cmath>
#include <sstream>
#include <algorithm>

using namespace std;

GEntity::GEntity(GEntityType type, double value)   : m_type(type), m_code(value) {}
GEntity::GEntity(char type, double value) : m_code(value) {
    switch (type) {
    case 'G': m_type = GEntityType::G; break;
    case 'M': m_type = GEntityType::M; break;
    case 'X': m_type = GEntityType::X; break;
    case 'Y': m_type = GEntityType::Y; break;
    case 'U': m_type = GEntityType::U; break;
    case 'V': m_type = GEntityType::V; break;
    case 'I': m_type = GEntityType::I; break;
    case 'J': m_type = GEntityType::J; break;
    case 'R': m_type = GEntityType::R; break;
    case 'F': m_type = GEntityType::F; break;
    case 'P': m_type = GEntityType::P; break;
    case 'Q': m_type = GEntityType::Q; break;
    default: m_type = GEntityType::NONE; break;
    }
}

string GEntity::toString() const {
    switch (m_type) {
    case GEntityType::NONE: return "";
    case GEntityType::G: return writeString('G', static_cast<uint8_t>(m_code));
    case GEntityType::M: return writeString('M', static_cast<uint8_t>(m_code));
    case GEntityType::X: return writeString('X', m_code);
    case GEntityType::Y: return writeString('Y', m_code);
    case GEntityType::U: return writeString('U', m_code);
    case GEntityType::V: return writeString('V', m_code);
    case GEntityType::I: return writeString('I', m_code);
    case GEntityType::J: return writeString('J', m_code);
    case GEntityType::R: return writeString('R', m_code);
    case GEntityType::F: return writeString('F', m_code);
    case GEntityType::P: return writeString('P', m_code);
    case GEntityType::Q: return writeString('Q', m_code);
    }

    return "";
}

string GEntity::writeString(char ch, uint8_t value) {
    return ch + to_string(value);
}

string GEntity::writeString(char ch, double value) {
    double prec = 0.001;
    stringstream ss;
    value = round(value / prec) * prec;
    if (value <= 0 && value > -prec) value = 0;
    ss<<ch<<value;
    return ss.str();
}

bool GFrame::empty() const {
    return m_entities.empty() && m_comment.empty() && m_pragma == PragmaType::NONE;
}

//string GFrame::toString() const {
//    string s;

//    if (valid.bit.delimiter)
//        s += "%";
//    else {
//        if (valid.bit.M) s += !s.empty() ? " M" + std::to_string(M) : "M" + std::to_string(M);
//        if (valid.bit.P) s += !s.empty() ? " P" + std::to_string(P) : "P" + std::to_string(P);
//        if (valid.bit.Q) s += !s.empty() ? " Q" + std::to_string(Q) : "Q" + std::to_string(Q);

//        if (valid.bit.G) s += !s.empty() ? " G" + std::to_string(G) : "G" + std::to_string(G);
//        if (valid.bit.X) s += !s.empty() ? " X" + std::to_string(X) : "X" + std::to_string(X);
//        if (valid.bit.Y) s += !s.empty() ? " Y" + std::to_string(Y) : "Y" + std::to_string(Y);
//        if (valid.bit.U) s += !s.empty() ? " U" + std::to_string(U) : "U" + std::to_string(U);
//        if (valid.bit.V) s += !s.empty() ? " V" + std::to_string(V) : "V" + std::to_string(V);
//        if (valid.bit.F) s += !s.empty() ? " F" + std::to_string(X) : "F" + std::to_string(F);
//    }
//    if (!comment.empty()) s += !s.empty() ? " " + comment : comment;
//    return s;
//}

void write_string(QString& s, QChar ch, uint8_t value) {
    if (s.length() != 0) s += " ";
    s += QString::asprintf("%c%d", ch.toLatin1(), value);
}

void write_string(QString& s, QChar ch, double value) {
    if (s.length() != 0) s += " ";
    s += QString::asprintf("%c%g", ch.toLatin1(), value);
}

string GFrame::toString() const {
    switch (m_pragma) {
    case PragmaType::DELIMITER:
        return "%";
    case PragmaType::CUTLINE_BEGIN:
        return "(<cutline>)";
    case PragmaType::CUTLINE_END:
        return "(</cutline>)";
    default:
        break;
    }

    string s;
    for (const GEntity& cmd: m_entities) {
        if (!s.empty()) s += " ";
        s += cmd.toString();
    }

    if (!m_comment.empty()) {
        if (!s.empty()) s += " ";
        s += "(" + m_comment + ")";
    }

    return s;
}

const GFrame& GFrame::removeComment() {
    if (m_pragma != PragmaType::DELIMITER) m_pragma = PragmaType::NONE;
    m_comment.clear();
    return *this;
}

//GFrame GFrame::noComment() const {
//    GFrame new_gframe(*this);
//    new_gframe.m_pragma = PragmaType::NONE; // fixme
//    new_gframe.m_comment.clear();
//    return new_gframe;
//}

void GFrame::offset(double X, double Y, double U, double V) {
//    bool XY = false, UV = false;

    for (GEntity& ent: m_entities)
        switch (ent.type()) {
        case GEntityType::X:
            ent.inc(X);
//            XY = true;
//            UV = false;
            break;

        case GEntityType::Y:
            ent.inc(Y);
//            XY = true;
//            UV = false;
            break;

        case GEntityType::U:
            ent.inc(U);
//            XY = false;
//            UV = true;
            break;

        case GEntityType::V:
            ent.inc(V);
//            XY = false;
//            UV = true;
            break;

//        case GCommandType::I:
//            if (XY) ent.inc(X);
//            else if (UV) ent.inc(U);
//            break;

//        case GCommandType::J:
//            if (XY) ent.inc(Y);
//            else if (UV) ent.inc(V);
//            break;

        default:
            break;
        }
}

void GFrame::removeG(uint8_t value) {
    for (auto it = m_entities.begin(); it < m_entities.end();) {
        if (it->type() == GEntityType::G && uint8_t(it->code()) == value)
            it = m_entities.erase(it);
        else
            ++it;
    }
}

void GFrame::replaceRtoIJ(bool BOT, const fpoint_t& C) {
    bool XY = false, UV = false;

    for (auto it = m_entities.begin(); it < m_entities.end();) {
        switch (it->type()) {
        case GEntityType::X: case GEntityType::Y:
            XY = true;
            UV = false;
            ++it;
            break;

        case GEntityType::U: case GEntityType::V:
            XY = false;
            UV = true;
            ++it;
            break;

        case GEntityType::R:
            if (BOT) {
                if (XY) {
                    it = m_entities.erase(it);
                    it = m_entities.insert(it, GEntity('I', C.x));
                    it = m_entities.insert(it, GEntity('J', C.y));
                }
                else
                    ++it;
            }
            else {
                if (UV) {
                    it = m_entities.erase(it);
                    it = m_entities.insert(it, GEntity('I', C.x));
                    it = m_entities.insert(it, GEntity('J', C.y));
                }
                else
                    ++it;
            }
            break;

        default:
            ++it;
            break;
        }
    }
}

GFrame::GFrame() : m_pragma(PragmaType::NONE) {}
GFrame::~GFrame() {}
GFrame::GFrame(const GFrame &other) { *this = other; }
GFrame::GFrame(GFrame&& other) noexcept : m_pragma(other.m_pragma) {
    m_entities = std::move(other.m_entities);
    m_comment = std::move(other.m_comment);
}

GFrame &GFrame::operator=(const GFrame& other) {
    if (this != &other) {
        m_pragma = other.m_pragma;
        m_entities = other.m_entities;
        m_comment = other.m_comment;
    }
    return *this;
}

GFrame &GFrame::operator=(GFrame&& other) noexcept {
    if (this != &other) {
        m_pragma = other.m_pragma;
        m_entities = std::move(other.m_entities);
        m_comment = std::move(other.m_comment);
    }
    return *this;
}

void GFrame::push_back(const GEntity &entity) { m_entities.push_back(entity); }

void GFrame::append(const GFrame &frame) {
    m_entities.insert(m_entities.end(), frame.m_entities.cbegin(), frame.m_entities.cend());
}

void GFrame::clear() {
    m_pragma = PragmaType::NONE;
    m_entities.clear();
    m_comment.clear();
}

GFrame GFrame::parse(string str, bool& err, const GFrame& prev) {
    GFrame frame;
    frame.m_comment = cutComment(str);
    frame.m_pragma = GFrame::toPragma(frame.m_comment);

    if (frame.m_pragma == PragmaType::NONE) {
        switch (prev.m_pragma) {
        case PragmaType::CUTLINE_BEGIN:
        case PragmaType::CUTLINE_BODY:
            frame.m_pragma = PragmaType::CUTLINE_BODY;
            break;
        default:
            break;
        }
    }

    StringParser parser(str);

    char ch;
    err = false;
    int newInt;
    double newDouble;

    while (!err && parser.getChar(ch)) {
        switch (ch) {
        case 'G': case 'M':
            err = !parser.getInt(newInt);
            err |= newInt < 0 || newInt > UINT8_MAX;

            if (!err) {
                frame.m_entities.push_back(GEntity(ch, static_cast<uint8_t>(newInt)));
            }

            break;

        case 'X': case 'Y': case 'U': case 'V': case 'I': case 'J': case 'R': case 'F': case 'P': case 'Q':
            err = !parser.getDouble(newDouble);

            if (!err) {
                frame.m_entities.push_back(GEntity(ch, newDouble));
            }

            break;

        case '%':
            frame.m_pragma = PragmaType::DELIMITER;
            break;

        default:
            err = true;
            break;
        }
    }

    return frame;
}

string GFrame::cutComment(string& line) {
    string comment;
    string new_line;
    bool isComment = false;

    for (char ch: line) {
        if (!isComment) {
            if (ch == '(')
                isComment = true;
            else
                new_line += ch;
        }
        else {
            if (ch == ')')
                isComment = false;
            else
                comment += ch;
        }
    }

    line = std::move(new_line);

    return comment;
}

void GFrame::removeAllSpaces(string& str) {
    string new_str;

    for (char ch: str) {
        if (ch != ' ' && ch != '\t')
            new_str += ch;
    }

    str = std::move(new_str);
}

GFrame::PragmaType GFrame::toPragma(string& comment) {
    PragmaType pragma_type;

    regex re_cutline_begin(R"(^\s*<cutline>\s*$)");
    regex re_cutline_end(R"(^\s*</cutline>\s*$)");

    smatch m_cutline_begin, m_cutline_end;

    if (regex_match(comment, m_cutline_begin, re_cutline_begin))
        pragma_type = PragmaType::CUTLINE_BEGIN;
    else if (regex_match(comment, m_cutline_end, re_cutline_end))
        pragma_type = PragmaType::CUTLINE_END;
    else
        pragma_type = PragmaType::NONE;

    if (pragma_type != PragmaType::NONE)
        comment.clear();

    return pragma_type;
}

//const double* GFrame::find(GEntity::CommandType type) const {
//    auto it = std::find(m_entities.begin(), m_entities.end(), [type](GEntity ent) { return ent.m_type == type; });
//    return it != m_entities.end() ? &(it->m_code) : nullptr;
//}

//int GFrame::G() const {
//    const double* value = find(GEntity::CommandType::G);
//    return value ? static_cast<int>(*value) : -1;
//}
//const double* GFrame::X() const { return find(GEntity::CommandType::X); }
//const double* GFrame::Y() const { return find(GEntity::CommandType::Y); }
//const double* GFrame::U() const { return find(GEntity::CommandType::U); }
//const double* GFrame::V() const { return find(GEntity::CommandType::V); }

// todo: read all command param by patern, if error finish
list<GCommand> GFrame::get(int& G, AXIS_ENA& axis_ena) const {
    list<GCommand> list;
    GCommand cmd = GCommand();
    bool uv_reg = false;
    AXIS_ENA _axis_ena = AXIS_ENA::NO_AXIS;

    if (!(G >= 1 && G <= 3))
        G = -1;

    for (vector<GEntity>::const_iterator it = m_entities.begin(); it != m_entities.end(); ++it) {
        switch (it->type()) {
        case GEntityType::G:
        {
            int g = static_cast<int>(it->code());

            switch (g) {
            case 0:
                if (!cmd.empty()) {
                    list.push_back(cmd);
                    cmd.clear();
                }

                cmd.setG(g);
                break;

            case 1: case 2: case 3:
                if (cmd.isXY() && !cmd.isG2()) {
                    cmd.setG2(g);
                    G = g;
                }
                else {
                    if (!cmd.empty()) {
                        list.push_back(cmd);
                        cmd.clear();
                    }

                    cmd.setG(g);
                    G = g;
                }

                break;

            case 90: case 91:
                if (!cmd.empty()) {
                    list.push_back(cmd);
                    cmd.clear();
                }

                cmd.setG(g);
                list.push_back(cmd);
                cmd.clear();

                break;

            case 92:
                if (!cmd.empty()) {
                    list.push_back(cmd);
                    cmd.clear();
                }
                cmd.setG(g);

                break;

            default:
                break;
            }
        }
            break;

        case GEntityType::M:
            if (!cmd.empty()) {
                list.push_back(cmd);
                cmd.clear();
            }

            cmd.setM(it->code());
            break;

        case GEntityType::P:
            if (cmd.isM() || (cmd.isG() && cmd.G() == 4))
                cmd.setP(it->code());
            break;

        case GEntityType::Q:
            if (cmd.isM())
                cmd.setQ(it->code());
            break;

        case GEntityType::F:
            cmd.setF(it->code());
            break;

        case GEntityType::X:
            if (_axis_ena == AXIS_ENA::NO_AXIS)
                _axis_ena = AXIS_ENA::XY_AXIS;

            uv_reg = false;
            cmd.setX(it->code());

            if (G >= 1 && G <= 3)
                cmd.setG(G);
            break;

        case GEntityType::Y:
            if (_axis_ena == AXIS_ENA::NO_AXIS)
                _axis_ena = AXIS_ENA::XY_AXIS;

            uv_reg = false;
            cmd.setY(it->code());

            if (G >= 1 && G <= 3)
                cmd.setG(G);
            break;

        case GEntityType::U:
            _axis_ena = AXIS_ENA::UV_AXIS;
            uv_reg = true;
            cmd.setU(it->code());

            if (G >= 1 && G <= 3)
                cmd.setG2(G);
            break;

        case GEntityType::V:
            _axis_ena = AXIS_ENA::UV_AXIS;
            uv_reg = true;
            cmd.setV(it->code());

            if (G >= 1 && G <= 3)
                cmd.setG2(G);

            break;

        case GEntityType::I:
            if (!uv_reg) {
                bool OK = cmd.setI(it->code());
                if (OK && (G == 2 || G == 3))
                    cmd.setG(G);
            }
            else {
                bool OK = cmd.setI2(it->code());
                if (OK && (G == 2 || G == 3))
                    cmd.setG2(G);
            }
            break;

        case GEntityType::J:
            if (!uv_reg) {
                bool OK = cmd.setJ(it->code());
                if (OK && (G == 2 || G == 3))
                    cmd.setG(G);
            }
            else {
                bool OK = cmd.setJ2(it->code());
                if (OK && (G == 2 || G == 3))
                    cmd.setG2(G);
            }
            break;

        case GEntityType::R:
            if (!uv_reg) {
                bool OK = cmd.setR(it->code());
                if (OK && (G == 2 || G == 3))
                    cmd.setG(G);
            }
            else {
                bool OK = cmd.setR2(it->code());
                if (OK && (G == 2 || G == 3))
                    cmd.setG2(G);
            }
            break;

        default:
            break;
        }
    }

    if (!cmd.empty()) {
        list.push_back(cmd);
        cmd.clear();
    }

    if (!list.empty())
        list.back().setEOF();

    if (axis_ena == AXIS_ENA::NO_AXIS)
        axis_ena = _axis_ena;

    return list;
}

bool GFrame::any(const GEntityType type, const std::list<uint8_t>& codes) const {
    for (const GEntity& ent: m_entities) {
        if (ent.type() == type) {
            uint8_t ent_code = ent.codeUInt8();

            for (uint8_t code: codes) {
                if (ent_code == code)
                    return true;
            }
        }
    }

    return false;
}

GFrame::PLANE_T GFrame::getPlane() const {
    PLANE_T plane = PLANE_T::PLANE_UNKNOWN;

    for (const GEntity& ent: m_entities) {
        if ((plane == PLANE_T::PLANE_UNKNOWN || plane == PLANE_T::PLANE_XY) && (ent.type() == GEntityType::U || ent.type() == GEntityType::V)) {
            plane = PLANE_T::PLANE_XYUV;
            break;
        }
        else if (plane == PLANE_T::PLANE_UNKNOWN && (ent.type() == GEntityType::X || ent.type() == GEntityType::Y))
            plane = PLANE_T::PLANE_XY;
    }

    return plane;
}
