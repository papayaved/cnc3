#include "xml_ext.h"
using namespace std;

void xml_ext::writeElement(QXmlStreamWriter &xml, const QString &name, const QString &type, const QString &value) {
    xml.writeStartElement(name);
        xml.writeAttribute("type", type);
        xml.writeCharacters(value);
    xml.writeEndElement();
}

void xml_ext::writeDouble(QXmlStreamWriter &xml, const QString &name, double value) {
    writeElement(xml, name, "double", QString::number(static_cast<double>(value), 'g', 9));
}

void xml_ext::writeUInt(QXmlStreamWriter &xml, const QString &name, unsigned long long value) {
    writeElement(xml, name, "unsigned", QString::number(value));
}

void xml_ext::writeBool(QXmlStreamWriter &xml, const QString &name, bool value) {
    writeElement(xml, name, "boolean", value ? "true" : "false");
}

void xml_ext::writeOffset(QXmlStreamWriter &xml, const QString &name, const offset_t &offset) {
    xml.writeStartElement(name);
        writeDouble(xml, "value", offset.offset);
        writeUInt(xml, "mode_id", offset.mode_id);
    xml.writeEndElement();
}

void xml_ext::writeOffsets(QXmlStreamWriter& xml, const std::vector<offset_t>& offsets) {
    xml.writeStartElement("offsets");
    for (const offset_t& o: offsets)
        writeOffset(xml, "offset", o);
    xml.writeEndElement();
}

void xml_ext::writeFPoint(QXmlStreamWriter& xml, const QString& name, const fpoint_t& pt) {
    xml.writeStartElement(name);
        writeDouble(xml, "X", pt.x);
        writeDouble(xml, "Y", pt.y);
    xml.writeEndElement();
}

void xml_ext::writeContourLine(QXmlStreamWriter &xml, const DxfLine* line) {
    if (!line) return;

    xml.writeStartElement("line");
        writeFPoint(xml, "A", line->point_0());
        writeFPoint(xml, "B", line->point_1());
    xml.writeEndElement();
}

void xml_ext::writeContourArc(QXmlStreamWriter &xml, const DxfArc* arc) {
    if (!arc) return;

    xml.writeStartElement("arc");
        writeFPoint(xml, "C", arc->center());
        writeDouble(xml, "R", arc->radius());
        writeDouble(xml, "a", arc->startAngle());
        writeDouble(xml, "b", arc->endAngle());
        writeBool(xml, "ccw", arc->CCW());
    xml.writeEndElement();
}

void xml_ext::writeContourPoint(QXmlStreamWriter &xml, const DxfPoint* pt) {
    if (!pt) return;

    xml.writeStartElement("point");
        writeFPoint(xml, "P", pt->point_0());
    xml.writeEndElement();
}

void xml_ext::writeContourEntity(QXmlStreamWriter &xml, const DxfEntity *ent) {
    if (!ent) return;

    switch (ent->type()) {
    case ENTITY_TYPE::LINE:
        writeContourLine(xml, dynamic_cast<const DxfLine*>(ent));
        break;
    case ENTITY_TYPE::ARC:
        writeContourArc(xml, dynamic_cast<const DxfArc*>(ent));
        break;
    case ENTITY_TYPE::POINT:
        writeContourPoint(xml, dynamic_cast<const DxfPoint*>(ent));
        break;
    default:
        break;
    }
}

void xml_ext::writeContour(QXmlStreamWriter &xml, const QString &name, const list<DxfEntity*> &list) {
    xml.writeStartElement(name);

    for (auto ent : list)
        writeContourEntity(xml, ent);

    xml.writeEndElement();
}

void xml_ext::writeContour(QXmlStreamWriter &xml, const QString& type, const std::list<DxfEntity *> &bot, const std::list<DxfEntity *> &top) {
    xml.writeStartElement(type);
        writeContour(xml, "bot", bot);
        writeContour(xml, "top", top);
    xml.writeEndElement();
}

void xml_ext::writeContours(QXmlStreamWriter &xml, const std::deque<ContourPair> &contours) {
    xml.writeStartElement("contours");

    for (auto& pair : contours)
        writeContour(xml, QString( pair.typeToXmlName().data() ), pair.bot()->entities(), pair.top()->entities());

    xml.writeEndElement();
}

QString xml_ext::readType(QXmlStreamReader& xml) {
    return xml.attributes().constFirst().value().toString();
}

void xml_ext::readElement(QXmlStreamReader& xml, QString& type, QString& value) {
    type = xml_ext::readType(xml);
    value = xml.readElementText();
}

// return OK
bool xml_ext::readDouble(QXmlStreamReader &xml, double &value) {
    bool OK;
    double res = xml.readElementText().toDouble(&OK);
    if (OK) value = res;
    return OK;
}

bool xml_ext::readFloat(QXmlStreamReader &xml, float &value) {
    bool OK;
    float res = xml.readElementText().toFloat(&OK);
    if (OK) value = res;
    return OK;
}

// return OK
bool xml_ext::readUInt(QXmlStreamReader& xml, size_t& value) {
    bool OK;
    QString s = xml.readElementText();
    unsigned long long res = s.toULongLong(&OK);
    if (OK) value = res;
    return OK;
}

// return OK
bool xml_ext::readUInt16(QXmlStreamReader &xml, uint16_t& value) {
    bool OK;
    QString s = xml.readElementText();
    unsigned long long res = s.toUInt(&OK);
    if (OK) value = res;
    return OK;
}

// return OK
bool xml_ext::readUInt8(QXmlStreamReader &xml, uint8_t& value) {
    bool OK;
    QString s = xml.readElementText();
    unsigned long long res = s.toUInt(&OK);
    if (OK) value = res;
    return OK;
}

// return OK
bool xml_ext::readBool(QXmlStreamReader &xml, bool &value) {
    QString s = xml.readElementText();
    s = s.trimmed();

    if (s.compare("true", Qt::CaseSensitivity::CaseInsensitive) == 0) {
        value = true;
        return true;
    } else if (s.compare("false", Qt::CaseSensitivity::CaseInsensitive) == 0) {
        value = false;
        return true;
    }

    return false;
}

bool xml_ext::readOffsetSide(QXmlStreamReader &xml, OFFSET_SIDE &value) {
    bool OK = false;
    QString type, str;

    xml_ext::readElement(xml, type, str);

    if (type == "OFFSET_SIDE") {
        if (str == "RIGHT") {
            value = OFFSET_SIDE::RIGHT;
            OK = true;
        } else if (str == "LEFT") {
            value = OFFSET_SIDE::LEFT;
            OK = true;
        }
    }

    return OK;
}

bool xml_ext::nextStartElement(QXmlStreamReader& xml) {
    while (xml.tokenType() != QXmlStreamReader::StartElement) {
        if (xml.atEnd() || xml.hasError())
            return false;

        xml.readNext();
    }

    return true;
}

bool xml_ext::nextEndElement(QXmlStreamReader &xml) {
    while (xml.tokenType() != QXmlStreamReader::EndElement) {
        if (xml.atEnd() || xml.hasError())
            return false;

        xml.readNext();
    }

    return true;
}

bool xml_ext::readOffset(QXmlStreamReader& xml, offset_t& value) {
    while (!(xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "value")) {
        if (xml.atEnd() || xml.hasError())
            return false;

        xml.readNext();
    }

    bool OK = xml_ext::readDouble(xml, value.offset);

    while (!(xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "mode_id")) {
        if (xml.atEnd() || xml.hasError())
            return false;

        xml.readNext();
    }

    OK &= xml_ext::readUInt8(xml, value.mode_id);

    return OK;
}

bool xml_ext::readOffsets(QXmlStreamReader &xml, std::vector<offset_t> &v) {
    v.clear();

    while (1) {
        if (xml.atEnd() || xml.hasError())
            return false;
        else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "offset") {
            offset_t value;
            bool OK = xml_ext::readOffset(xml, value);
            if (OK)
                v.push_back(value);
        } else if (xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "offsets")
            return true;

        xml.readNext();
    }
}

bool xml_ext::readFPoint(QXmlStreamReader& xml, const QString& name, fpoint_t& pt) {
    int valid = 0;

    if (xml.name() != name)
        return false;

    while ( !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == name) ) {
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "X")
                valid |= readDouble(xml, pt.x) << 0;
            else if (xml.name() == "Y")
                valid |= readDouble(xml, pt.y) << 1;
        } else if (xml.atEnd() || xml.hasError())
            return false;

        xml.readNext();
    }

    return valid == 3;
}

bool xml_ext::readContourLine(QXmlStreamReader &xml, DxfLine &line) {
    fpoint_t A, B;
    int valid = 0;

    if (xml.name() != "line")
        return false;

    while ( !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "line") ) {
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "A")
                valid |= readFPoint(xml, "A", A) << 0;
            else if (xml.name() == "B")
                valid |= readFPoint(xml, "B", B) << 1;
        } else if (xml.atEnd() || xml.hasError())
            return false;

        xml.readNext();
    }

    if (valid == 3) {
        line = DxfLine(A, B);
        return true;
    }

    return false;
}

bool xml_ext::readContourArc(QXmlStreamReader &xml, DxfArc &arc) {
    fpoint_t C;
    double R(0), a(0), b(0);
    bool ccw = true;
    int valid = 0;


    if (xml.name() != "arc")
        return false;

    while ( !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "arc") ) {
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "C")
                valid |= readFPoint(xml, "C", C) << 0;
            else if (xml.name() == "R")
                valid |= readDouble(xml, R) << 1;
            else if (xml.name() == "a")
                valid |= readDouble(xml, a) << 2;
            else if (xml.name() == "b")
                valid |= readDouble(xml, b) << 3;
            else if (xml.name() == "ccw")
                valid |= readBool(xml, ccw) << 4;
        } else if (xml.atEnd() || xml.hasError())
            return false;

        xml.readNext();
    }

    if (valid == 0x1F) {
        arc = DxfArc(C, R, a, b, ccw);
        return true;
    }

    return false;
}

bool xml_ext::readContourPoint(QXmlStreamReader &xml, DxfPoint &pt) {
    fpoint_t P;
    bool OK = false;

    if (xml.name() != "point")
        return false;

    while ( !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "point") ) {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "P")
            OK = readFPoint(xml, "P", P);
        else if (xml.atEnd() || xml.hasError())
            return false;

        xml.readNext();
    }

    if (OK)
        pt = DxfPoint(P);

    return OK;
}

DxfEntity* xml_ext::readContourEntity(QXmlStreamReader &xml) {
    if (xml.tokenType() == QXmlStreamReader::StartElement) {
        if (xml.name() == "line") {
            DxfLine* line = new DxfLine();

            if (!readContourLine(xml, *line)) {
                delete line;
                line = nullptr;
            }

            return line;
        } else if (xml.name() == "arc") {
            DxfArc* arc = new DxfArc();

            if (!readContourArc(xml, *arc)) {
                delete arc;
                arc = nullptr;
            }

            return arc;
        } else if (xml.name() == "point") {
            DxfPoint* pt = new DxfPoint();

            if (!readContourPoint(xml, *pt)) {
                delete pt;
                pt = nullptr;
            }

            return pt;
        }
    }

    return nullptr;
}

// read until end of name
Dxf xml_ext::readContourLayer(QXmlStreamReader &xml, const QString& name) {
    Dxf dxf;

    while ( !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == name) ) {
        if (xml.atEnd() || xml.hasError()) {
            dxf.clear();
            return dxf;
        }

        DxfEntity* ent = readContourEntity(xml);
        if (ent)
            dxf.push_back(ent);

        xml.readNext();
    }

    return dxf;
}

// read given contour type until the end
ContourPair xml_ext::readContourPair(QXmlStreamReader &xml, CONTOUR_TYPE type) {
    ContourPair p(type);

    while (! (xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == ContourPair::xmlName(type).data()) ) {
        if (xml.atEnd() || xml.hasError()) {
            p.clear();
            return p;
        }

        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "bot") {
            Dxf bot = xml_ext::readContourLayer(xml, "bot");
            p.setBot(bot);
        } else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "top") {
            Dxf top = xml_ext::readContourLayer(xml, "top");
            p.setTop(top);
        }

        xml.readNext();
    }

    return p;
}

deque<ContourPair> xml_ext::readContours(QXmlStreamReader &xml) {
    deque<ContourPair> pairs;

    // go to contours
    while ( !(xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "contours") ) {
        if (xml.atEnd() || xml.hasError())
            return deque<ContourPair>();

        xml.readNext();
    }

    // go to contour
    while ( !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "contours") ) {
        if (xml.atEnd() || xml.hasError())
            return deque<ContourPair>();

        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == ContourPair::xmlName(CONTOUR_TYPE::CUTLINE_CONTOUR).data() ) {
            ContourPair p = readContourPair(xml, CONTOUR_TYPE::CUTLINE_CONTOUR);
            pairs.push_back(p);
        } else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == ContourPair::xmlName(CONTOUR_TYPE::MAIN_CONTOUR).data() ) {
            ContourPair p = readContourPair(xml, CONTOUR_TYPE::MAIN_CONTOUR);
            pairs.push_back(p);
        }

        xml.readNext();
    }

    return pairs;
}
