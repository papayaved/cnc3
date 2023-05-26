#include "wire_cut.h"
#include "xml_ext.h"

using namespace xml_ext;

cut_t::cut_t() :
    times(3),
    cutline_mode_id(0),
    offset_side(OFFSET_SIDE::RIGHT),
    aux_offset_side(OFFSET_SIDE::LEFT),
    offset_ena(true),
    aux_offset_ena(false),
    tab_multi_pass(false),
    tab_pause(true),
    pump_pause(false),
    speed_ena(true),
    pump_delay(0),
    seg_pause(0),
    tab_seg(0),
    aux_offset(0),
    tab(DEFAULT_TAB),
    overcut(0),
    speed(cnc_param::SPEED),
    L(cnc_param::L_DEFAULT),
    H(cnc_param::H_DEFAULT),
    T(cnc_param::T_DEFAULT),
    D(cnc_param::D_DEFAULT),
    wire_D(cnc_param::WIRE_DIAMETER),
    uv_ena(false),
    D_ena(false),
    axis_D(cnc_param::AXIS_DEFAULT),
    tab_offset(offset_t())
  //        overcut_offset(offset_t()),
  //        out_offset(offset_t())
{
    offsets.resize(MAX_TIMES);
    for (offset_t& o: offsets)
        o.offset = cnc_param::WIRE_DIAMETER / 2.0;

#ifndef STONE
    offsets[0] = offset_t(0, cnc_param::WIRE_DIAMETER / 2.0 + 0.06);
    offsets[1] = offset_t(1, cnc_param::WIRE_DIAMETER / 2.0 + 0.02);
    offsets[2] = offset_t(2, cnc_param::WIRE_DIAMETER / 2.0);
#endif

    tab_offset = offsets[0];
}

void cut_t::write(QXmlStreamWriter& xml) const {
    xml.writeStartElement("cut");

        writeUInt(xml, "times", times);
        writeUInt(xml, "cutline_mode_id", cutline_mode_id);

        writeElement(xml, "offset_side", "OFFSET_SIDE", offset_side == OFFSET_SIDE::RIGHT ? "RIGHT" : "LEFT");
        writeElement(xml, "aux_offset_side", "OFFSET_SIDE", aux_offset_side == OFFSET_SIDE::RIGHT ? "RIGHT" : "LEFT");

        writeBool(xml, "offset_ena", offset_ena);
        writeBool(xml, "aux_offset_ena", aux_offset_ena);
        writeBool(xml, "speed_ena", speed_ena);

        writeUInt(xml, "seg_pause", seg_pause);
        writeUInt(xml, "tab_seg", tab_seg);

        writeDouble(xml, "aux_offset", aux_offset);
        writeDouble(xml, "speed", speed);

        writeBool(xml, "tab_multi_pass", tab_multi_pass);
        writeDouble(xml, "tab", tab);

        writeOffset(xml, "tab_offset", tab_offset);

        writeBool(xml, "tab_pause", tab_pause);

        writeDouble(xml, "overcut", overcut);

        writeUInt(xml, "pump_delay", pump_delay);
        writeBool(xml, "pump_pause", pump_pause);

        writeBool(xml, "uv_ena", uv_ena);
        writeDouble(xml, "L", L);
        writeDouble(xml, "H", H);
        writeDouble(xml, "T", T);

        writeBool(xml, "D_ena", D_ena);
        writeDouble(xml, "D", D);
        writeDouble(xml, "wire_D", wire_D);
        writeElement(xml, "axis_D", "AXIS", axis_D == AXIS::AXIS_X ? "X" : "Y");

        xml.writeStartElement("offsets");
        for (const offset_t& o: offsets)
            writeOffset(xml, "offset", o);
        xml.writeEndElement();

    xml.writeEndElement();
}

bool cut_t::read(QXmlStreamReader &xml) {
    while (!(xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "cut")) {
        if (xml.atEnd() || xml.hasError())
            return false;

        xml.readNext();
    }

    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "cut")) {
        if (xml.atEnd() || xml.hasError())
            return false;

       if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "times")
                readUInt8(xml, times);
            else if (xml.name() == "cutline_mode_id")
                readUInt8(xml, cutline_mode_id);
            else if (xml.name() == "offset_side")
                readOffsetSide(xml, offset_side);
            else if (xml.name() == "aux_offset_side")
                readOffsetSide(xml, aux_offset_side);
            else if (xml.name() == "offset_ena")
                readBool(xml, offset_ena);
            else if (xml.name() == "aux_offset_ena")
                readBool(xml, aux_offset_ena);
            else if (xml.name() == "speed_ena")
                readBool(xml, speed_ena);
            else if (xml.name() == "tab_seg")
                readUInt(xml, tab_seg);
            else if (xml.name() == "aux_offset")
                readDouble(xml, aux_offset);
            else if (xml.name() == "speed")
                readDouble(xml, speed);
            else if (xml.name() == "tab_multi_pass")
                readBool(xml, tab_multi_pass);
            else if (xml.name() == "tab")
                readDouble(xml, tab);
            else if (xml.name() == "tab_offset")
                readOffset(xml, tab_offset);
            else if (xml.name() == "tab_pause")
                readBool(xml, tab_pause);
            else if (xml.name() == "overcut")
                readDouble(xml, overcut);
            else if (xml.name() == "pump_delay")
                readUInt16(xml, pump_delay);
            else if (xml.name() == "pump_pause")
                readBool(xml, pump_pause);
            else if (xml.name() == "uv_ena")
                readBool(xml, uv_ena);
            else if (xml.name() == "L")
                readDouble(xml, L);
            else if (xml.name() == "H")
                readDouble(xml, H);
            else if (xml.name() == "T")
                readDouble(xml, T);
            else if (xml.name() == "D_ena")
                readBool(xml, D_ena);
            else if (xml.name() == "D")
                readDouble(xml, D);
            else if (xml.name() == "wire_D")
                readDouble(xml, wire_D);
            else if (xml.name() == "axis_D")
                readAxisD(xml, axis_D);
            else if (xml.name() == "offsets") {
                readOffsets(xml, offsets);

//                offset_t offset;

//                if (readOffset(xml, offset))
//                    offsets.push_back(offset);
            }
        }

        xml.readNext();
    }

    return true;
}

//bool WireCut::writeXML(QFile &file) const {
//    if (!file.isOpen())
//        return false;

//    QXmlStreamWriter xml(&file);
//    xml.setAutoFormatting(true);
//    xml.writeStartDocument();
//        write(xml);
//    xml.writeEndDocument();

//    return true;
//}

//bool WireCut::readXML(QFile& file) {
//    m_error.clear();
//    QXmlStreamReader xml;
//    xml.setDevice(&file);

//    while (xml.tokenType() != QXmlStreamReader::StartDocument) {
//        if (xml.atEnd() || xml.hasError()) {
//            if (xml.hasError())
//                m_error = xml.errorString();

//            return false;
//        }

//        xml.readNext();
//    }

//    bool OK = read(xml);

//    if (xml.hasError())
//        m_error = xml.errorString();

//    while (xml.tokenType() != QXmlStreamReader::EndDocument) {
//        if (xml.atEnd() || xml.hasError()) {
//            if (xml.hasError())
//                m_error = xml.errorString();

//            return false;
//        }

//        xml.readNext();
//    }

//    return OK;
//}
