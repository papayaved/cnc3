#include "generator_mode_list.h"
#include "xml_ext.h"

using namespace xml_ext;

void GeneratorMode::write(QXmlStreamWriter &xml) const {
    xml.writeStartElement("mode");
        writeUInt(xml, "roll_velocity", roll_velocity);
        writeUInt(xml, "current_level", current_level);
        writeUInt(xml, "pulse_width", pulse_width);
        writeUInt(xml, "pulse_ratio", pulse_ratio);
        writeBool(xml, "low_high_voltage", low_high_voltage);
    xml.writeEndElement();
}

bool GeneratorMode::read(QXmlStreamReader& xml) {
    uint flags = 0;
    *this = GeneratorMode();

    while (!(xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "mode")) {
        if (xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "modes")
            return false;
        else if (xml.atEnd() || xml.hasError())
            return false;

        xml.readNext();
    }

    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "mode")) {
        if (xml.atEnd() || xml.hasError())
            return false;

        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "roll_velocity")
                flags |= (uint)readUInt8(xml, roll_velocity) << 0;
            else if (xml.name() == "current_level")
                flags |= (uint)readUInt8(xml, current_level) << 1;
            else if (xml.name() == "pulse_width")
                flags |= (uint)readUInt8(xml, pulse_width) << 2;
            else if (xml.name() == "pulse_ratio")
                flags |= (uint)readUInt8(xml, pulse_ratio) << 3;
            else if (xml.name() == "low_high_voltage")
                flags |= (uint)readBool(xml, low_high_voltage) << 4;
        }

        xml.readNext();
    }

    return flags == 0x1F;
}

//////////////////////////////////////////////////////////////////////////////

GeneratorModeList::GeneratorModeList() {}

GeneratorModeList::GeneratorModeList(const std::deque<GeneratorMode>& modes) {
    m_list = modes;
    m_error.clear();
}

void GeneratorModeList::write(QXmlStreamWriter& xml) const {
    xml.writeStartElement("modes");
        for (const GeneratorMode& m : m_list)
            m.write(xml);
    xml.writeEndElement();
}

bool GeneratorModeList::read(QXmlStreamReader& xml) {
    GeneratorMode m;
    m_list.clear();

    while (!(xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "modes")) {
        if (xml.atEnd() || xml.hasError())
            return false;

        xml.readNext();
    }

    while (1) {
        bool OK = m.read(xml);

        if (OK)
            m_list.push_back(m);
        else if (xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "modes")
            return true;
        else if (xml.atEnd() || xml.hasError())
            return false;
    }

    return true;
}

bool GeneratorModeList::writeXML(QFile& file) const {
    if (!file.isOpen())
        return false;

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
        write(xml);
    xml.writeEndDocument();

    return true;
}

bool GeneratorModeList::readXML(QFile& file) {
    QXmlStreamReader xml;
    xml.setDevice(&file);

    while (xml.tokenType() != QXmlStreamReader::StartDocument) {
        if (xml.atEnd() || xml.hasError())
            return false;

        xml.readNext();
    }

    read(xml);

    if (xml.hasError()) {
        m_error = xml.errorString();
        return false;
    }

    return !m_list.empty();
}
