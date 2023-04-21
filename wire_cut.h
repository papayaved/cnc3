#ifndef WIRECUT_H
#define WIRECUT_H

#include <cstdint>
#include <vector>

#include <QString>
#include <QtXml>
#include <QFile>
#include "main.h"
#include "my_types.h"
#include "generator_mode_list.h"

struct cut_t {
    constexpr static const uint32_t MAX_TIMES = 7;
    constexpr static const uint32_t DEFAULT_TAB = 2;

    uint8_t times;
    uint8_t cutline_mode_id;
    OFFSET_SIDE offset_side, aux_offset_side;
    bool offset_ena, aux_offset_ena, tab_multi_pass, tab_pause, pump_pause, speed_ena;
    uint16_t pump_delay, seg_pause; // sec
    size_t tab_seg;
    double aux_offset, tab, overcut, speed, L, H, T;
    bool LHT_valid {false};

    std::vector<offset_t> offsets;
    offset_t tab_offset;
//    offset_t overcut_offset;
//    offset_t out_offset;

    cut_t();

    uint8_t getTimes() const;
    std::vector<offset_t> getOffsets() const;
    std::vector<offset_t> getTabOffsets() const;
    offset_t getOvercutOffset() const;
    offset_t getOutOffset() const;

    void write(QXmlStreamWriter& xml) const;
    bool read(QXmlStreamReader& xml);

    bool parseOffset(QXmlStreamReader& xml, offset_t& offset, bool& valid);
    bool parseOffsets(QXmlStreamReader& xml, std::vector<offset_t>& offsets);
    bool parseCut(QXmlStreamReader& xml, cut_t& cut);
};

//class WireCut : public cut_t {
//    using cut_t::cut_t;

//    QString m_error;

//public:
//    WireCut() {}
//    WireCut(const cut_t& other) : cut_t(other) {}

//    const cut_t& get() const { return *this; }

//    bool writeXML(QFile& file) const;
//    bool readXML(QFile& file);

//    bool hasError() const { return m_error.length() != 0; }
//    const QString& errorString() const { return m_error; }
//};

#endif // WIRECUT_H
