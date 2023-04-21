#ifndef CONTOUR_SET_H
#define CONTOUR_SET_H

#include <deque>
#include <vector>
#include "contour_list.h"
#include "cnc_types.h"

class ContourPass {
    const ContourList& m_base;
    std::deque<ContourPair> m_contours;

    static void contour_replace(std::deque<ContourPair>& rcv, const std::deque<ContourPair>& src);
    static void offset_replace(std::vector<offset_t>& rcv, const std::vector<offset_t>& src);
    static void connect(const ContourPair& prev, ContourPair& next);
    static std::deque<ContourPair> join(std::deque<ContourPair>& pass, std::deque<ContourPair>& tab, std::deque<ContourPair>& overcut);
    static void join(std::deque<ContourPair>& contours, ContourPair& out);
    static std::deque<ContourPair> join(std::deque<ContourPair>& pass, std::deque<ContourPair>& tab, std::deque<ContourPair>& overcut, ContourPair& out);
    static std::deque<ContourPair> joinLoop(std::deque<ContourPair>& pass, std::deque<ContourPair>& tab, std::deque<ContourPair>& overcut);

    void copy_back(const ContourPair& pair);
    void move_back(std::deque<ContourPair>& contours);

    static void printDebug(const std::deque<ContourPair>& contours);
    static void printDebug(const ContourPair& pair);
    static void printDebug(const std::deque<ContourPair>& pass, const std::deque<ContourPair>& tab, const std::deque<ContourPair>& overcut, const std::deque<ContourPair>& out);

    static void clear(std::deque<ContourPair>& contours);

public:
    constexpr static const double M_PRECISION = CncParam::PRECISION; // mm
    constexpr static const double M_TAB_MIN = 1; // mm
    constexpr static const double M_OVERCUT_MIN = 1; // mm

    ContourPass(const ContourList& base);
    ~ContourPass();

    void clear();
    bool empty() const;

    std::deque<ContourPair>& generate();
    const std::deque<ContourPair>& contours() const { return m_contours; }
};

#endif // CONTOUR_SET_H
