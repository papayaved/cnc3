#ifndef MODE_LIST_H
#define MODE_LIST_H

#include <list>
#include <cstdint>

struct GeneratorMode {
    uint32_t pulseWidth, pulseInterval, pulseVoltage, drumVelocity;
};

class ModeList {
    std::list<GeneratorMode> m_list;
public:
    ModeList();
};

#endif // MODE_LIST_H
