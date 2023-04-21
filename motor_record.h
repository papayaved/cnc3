#ifndef MOTOR_RECORD_H
#define MOTOR_RECORD_H

#include <cstdint>
#include <vector>
#include <string>

#include "my_types.h"

class MotorRecord {
public:
    static const size_t size = 3 * sizeof(uint32_t);

    int cnt; // 24
    AXIS axis; // 3
    bool sync; // 1
    int mode; // 2
    bool stop; // 1
    bool valid; // 1
    int32_t N; // 32
    uint32_t T; // 32

    MotorRecord();
    ~MotorRecord();
    MotorRecord(std::vector<uint8_t>& bytes, size_t* const pos);
    std::string toString();
};

#endif // MOTOR_RECORD_H
