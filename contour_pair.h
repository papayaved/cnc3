#ifndef CONTOUR_PAIR_H
#define CONTOUR_PAIR_H

#include <set>
#include "dxf.h"

//class CONTOUR_TYPE {
//public:
//    enum Value: uint8_t { CONTOUR_MAIN, CONTOUR_CUTLINE, CONTOUR_PASS, CONTOUR_TAB, CONTOUR_OVERCUT, CONTOUR_OUT, CONTOUR_UNKNOWN };
//    const QString s[7] = {"CONTOUR_MAIN", "CONTOUR_CUTLINE", "CONTOUR_PASS", "CONTOUR_TAB", "CONTOUR_OVERCUT", "CONTOUR_OUT", "CONTOUR_UNKNOWN"};

//    CONTOUR_TYPE(Value value) : m_value(value) {}

////    explicit CONTOUR_TYPE(const QString& type) {
////        if (type == "CONTOUR_MAIN")
////            m_value = CONTOUR_MAIN;
////        else
////            m_value = CONTOUR_UNKNOWN;
////    }

//    CONTOUR_TYPE& operator=(const CONTOUR_TYPE& value) {
//        if (this != &value)
//            m_value = value.m_value;

//        return *this;
//    }

//    CONTOUR_TYPE& operator= (Value value) {
//        m_value = value;
//        return *this;
//    }

//    operator Value() const { return m_value; }
//    explicit operator bool() const = delete;
//    bool operator==(Value o) const { return m_value == o; }
//    bool operator!=(Value o) const { return m_value != o; }

////    const QString& toString() {
////        return s[m_value];
////    }

//private:
//    Value m_value;
//};


// CONTOUR_EXIT - it's the type of cutline (don't use)
enum class CONTOUR_TYPE : uint8_t { MAIN_CONTOUR, CUTLINE_CONTOUR, PASS_CONTOUR, TAB_CONTOUR, OVERCUT_CONTOUR, OUT_CONTOUR, CONTOUR_UNKNOWN };

class ContourPair {
    constexpr static const double M_PRECISION = CncParam::PRECISION; // mm

    static GeneratorMode m_genModeDefault;
    static cut_t m_cutDefault;

    CONTOUR_TYPE m_type {CONTOUR_TYPE::MAIN_CONTOUR};
    Dxf *m_bot {nullptr}, *m_top {nullptr};    

    cut_t* m_cut {nullptr};
    std::deque<GeneratorMode>* m_genModes {nullptr};

    void lengthRatio(double len, double& bot_len, double& top_len) const;
    inline void swap(void*& x, void*& y);

public:
//    double speed() const { return m_cut ? m_cut->speed : m_cutDefault.speed; }

    class Reader {
        struct {
            uint8_t bot:1;
            uint8_t top:1;
        } valid;
        const ContourPair& m_pair;
        std::list<DxfEntity*>::const_iterator it_bot, it_top;

    public:
        Reader(const ContourPair& pair);
        const DxfEntity* bot();
        const DxfEntity* top();
        void next(const DxfEntity*& bot, const DxfEntity*& top);
    };

    Reader reader(const DxfEntity*& bot, const DxfEntity*& top) const;

    ContourPair(CONTOUR_TYPE type = CONTOUR_TYPE::MAIN_CONTOUR);
    ContourPair(const ContourPair& other);
    ContourPair(const ContourPair& other, CONTOUR_TYPE type);
    ContourPair(CONTOUR_TYPE type, const cut_t* const cut, const std::deque<GeneratorMode>* const modes);
    ContourPair(ContourPair&& other);
    ~ContourPair();

    ContourPair& operator=(const ContourPair& other);
    ContourPair& operator=(ContourPair&& other) noexcept;

    void swap(ContourPair& other);

    void setBot(const Dxf& ctr);
    Dxf* bot() const;

    void setTop(const Dxf& ctr);
    Dxf* top() const;

    void clear();

    // move DxfEntities
    void move_back(Dxf& bot, Dxf& top);

    void setType(CONTOUR_TYPE value) { m_type = value; }
    CONTOUR_TYPE type() const { return m_type; }

    void setCut(const cut_t& cut);
    void clearCut();
    const cut_t& cut() const;

    void setGeneratorModes(const std::deque<GeneratorMode>& modes);
    void setGeneratorModes(const ContourPair& contour);
    void clearModes();
    const GeneratorMode& generatorMode() const;
    void choiceMode(uint8_t mode_id);

    bool botEmpty() const;
    bool topEmpty() const;
    bool empty() const;
    bool isSorted() const;
    bool isLoop() const;

    bool hasOut() const;
    size_t getOutNum() const;
    fpoint_t getOutPoint() const;

    bool isCutline() const;
    void setCutline();

    size_t countBot() const;
    size_t countTop() const;
    size_t count() const;

    std::string toStringShort() const;
    std::string toString() const;
    static std::string toString(CONTOUR_TYPE type);
    static std::string xmlName(CONTOUR_TYPE type);
    std::string typeToString() const;
    std::string typeToXmlName() const;

    void reverse();

    void setFirst(size_t layer, size_t index);
    void setLast(size_t layer, size_t index);
    void moveUp(size_t layer, size_t index);
    void moveDown(size_t layer, size_t index);

    // Offset methods
    void getVerticesPoints(double len, size_t& bot_idx, size_t& top_idx);
    void split_rev(double tab, size_t& bot_idx, size_t& top_idx);

    void remove_before(size_t bot_idx, size_t top_idx);
    void remove_to(size_t bot_idx, size_t top_idx);
    void remove_from(size_t bot_idx, size_t top_idx);
    void remove_after(size_t bot_idx, size_t top_idx);
    void remove(size_t idx);

    void offset(OFFSET_SIDE m_side, double offset);

    fpoint_valid_t firstBot() const;
    fpoint_valid_t firstTop() const;
    fpoint_valid_t lastBot() const;
    fpoint_valid_t lastTop() const;

    void changeFirstPoint(const fpoint_valid_t& m_bot, const fpoint_valid_t& m_top);
    void changeLastPoint(const fpoint_valid_t& m_bot, const fpoint_valid_t& m_top);

    void shift(const fpoint_t& pt);
    void moveTo(const fpoint_t& pt = fpoint_t(0,0));

    std::vector<fpoint_t> getPointsBot() const;
    std::vector<fpoint_t> getPointsTop() const;

    double lengthBot() const;
    double lengthBot(size_t index, const fpoint_t& pt) const;

    void rotate(const RotateMatrix& mx);
    void flipX(double x);
    void flipY(double y);
    void scale(double pct, const fpoint_t& base);
    void scale(double pct);

    ContourRange range() const;

    ContourPair getMux(size_t in_seg, size_t out_seg) const;

    static ContourPair createCutline(double len, AXIS axis, DIR dir, bool top_ena = false);

    std::vector<size_t> getVerticesPoints() const;
};

#endif // CONTOUR_PAIR_H
