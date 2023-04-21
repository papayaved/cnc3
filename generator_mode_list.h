#ifndef GENERATORMODELIST_H
#define GENERATORMODELIST_H

#include <QtXml>
#include <QFile>
#include <deque>

struct GeneratorMode {
    uint8_t roll_velocity, current_level, pulse_width, pulse_ratio;
    bool low_high_voltage;

    GeneratorMode(uint8_t rollVelocity = 7, uint8_t currentIndex = 7, uint8_t pulseWidth = 36, uint8_t pulseRatio = 8, bool lowHV = false):
        roll_velocity(rollVelocity), current_level(currentIndex), pulse_width(pulseWidth), pulse_ratio(pulseRatio), low_high_voltage(lowHV) {}

    void write(QXmlStreamWriter& xml) const;
    bool read(QXmlStreamReader& xml);
};

class GeneratorModeList {
    std::deque<GeneratorMode> m_list;
    QString m_error;
public:
    GeneratorModeList();
    GeneratorModeList(const std::deque<GeneratorMode>& modes);

    const std::deque<GeneratorMode>& get() const { return m_list; }

    void write(QXmlStreamWriter& xmlWriter) const;
    bool read(QXmlStreamReader& xmlReader);

    bool writeXML(QFile& file) const;
    bool readXML(QFile& file);

    bool hasError() const { return m_error.length() != 0; }
    const QString& errorString() const { return m_error; }
};

#endif // GENERATORMODELIST_H
