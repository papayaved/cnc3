#ifndef XML_EXT_H
#define XML_EXT_H

#include <QObject>
#include <QtXml>
#include "my_types.h"
//#include "dxf_entity.h"
#include "dxf_line.h"
#include "dxf_arc.h"
#include "dxf_point.h"
#include "dxf.h"
#include "contour_pair.h"

namespace xml_ext {
    void writeElement(QXmlStreamWriter& xml, const QString& name, const QString& type, const QString& value);

    void writeBool(QXmlStreamWriter& xml, const QString& name, bool value);
    void writeUInt(QXmlStreamWriter& xml, const QString& name, unsigned long long value);
    void writeDouble(QXmlStreamWriter& xml, const QString& name, double value);

    void writeOffset(QXmlStreamWriter& xml, const QString& name, const offset_t& offset);
    void writeOffsets(QXmlStreamWriter& xml, const std::vector<offset_t>& offsets);

    void writeFPoint(QXmlStreamWriter& xml, const QString& name, const fpoint_t& pt);

    void writeContourLine(QXmlStreamWriter &xml, const DxfLine* line);
    void writeContourArc(QXmlStreamWriter &xml, const DxfArc* arc);
    void writeContourPoint(QXmlStreamWriter &xml, const DxfPoint* pt);
    void writeContourEntity(QXmlStreamWriter& xml, const DxfEntity* ent);

    void writeContour(QXmlStreamWriter& xml, const QString& name, const std::list<DxfEntity*>& list);
    void writeContour(QXmlStreamWriter& xml, const QString& type, const std::list<DxfEntity*>& bot, const std::list<DxfEntity*>& top);
    void writeContours(QXmlStreamWriter& xml, const std::deque<ContourPair>& contours);

    QString readType(QXmlStreamReader& xml);
    void readElement(QXmlStreamReader& xml, QString& type, QString& value);
    bool readDouble(QXmlStreamReader& xml, double& value);
    bool readFloat(QXmlStreamReader& xml, float& value);
    bool readUInt(QXmlStreamReader& xml, size_t& value);
    bool readUInt16(QXmlStreamReader& xml, uint16_t& value);
    bool readUInt8(QXmlStreamReader& xml, uint8_t& value);
    bool readBool(QXmlStreamReader& xml, bool& value);

    bool nextStartElement(QXmlStreamReader& xml);
    bool nextEndElement(QXmlStreamReader& xml);

    bool readOffsetSide(QXmlStreamReader& xml, OFFSET_SIDE& value);
    bool readOffset(QXmlStreamReader& xml, offset_t& value);
    bool readOffsets(QXmlStreamReader& xml, std::vector<offset_t>& v);

    bool readFPoint(QXmlStreamReader& xml, const QString& name, fpoint_t& pt);
    bool readContourLine(QXmlStreamReader& xml, DxfLine& line);
    bool readContourArc(QXmlStreamReader& xml, DxfArc& arc);
    bool readContourPoint(QXmlStreamReader& xml, DxfPoint& pt);

    DxfEntity* readContourEntity(QXmlStreamReader& xml);
    Dxf readContourLayer(QXmlStreamReader& xml, const QString& name);
    ContourPair readContourPair(QXmlStreamReader& xml, CONTOUR_TYPE type);
    std::deque<ContourPair> readContours(QXmlStreamReader& xml);
};

#endif // XML_WRITER_EXT_H
