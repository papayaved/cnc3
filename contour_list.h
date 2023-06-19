#ifndef CONTOURS_H
#define CONTOURS_H

#include <deque>
#include "contour_pair.h"
//#include "gcode.h"

class ContourList {
    std::deque<ContourPair> m_contours, m_contours_old;
    double m_thickness {0};
//    int contourSelected, entitySelected;

    int m_cur_ctr {-1}, m_cur_seg {-1};

    fpoint_t m_xyPos, m_uvPos; // motors
    fpoint_t m_botPos; // workpiece
    bool m_xyValid {false}, m_uvValid {false}, m_botValid {false};
    double m_botLength {-1};

    size_t m_ctr {0};
    std::map<size_t, uint8_t> m_seg_map;
    enum class CONTOUR_SELECT { NONE, CONTOUR, SEGMENT } m_sel = CONTOUR_SELECT::NONE;    

    std::string m_error;    

public:
    ContourList();
    ContourList(const ContourList& other);
    ContourList(const std::deque<ContourPair>& contours);
    ContourList(ContourList&& other);
    ContourList(std::deque<ContourPair>&& contours);

    ContourList& operator=(const ContourList& other);
    ContourList& operator=(ContourList&& other) noexcept;
    ~ContourList();

    void replace(std::deque<ContourPair>& contours);
//    void setCut(const cut_t& cut);
    void setCutParam(const cut_t& cut, const std::deque<GeneratorMode>& modes);
//    void setGeneratorModes(const std::deque<GeneratorMode>& modes);

    bool check();

    void push_front(const ContourPair& pair);
    void push_back(const ContourPair& pair);
    void set(size_t index, const ContourPair& pair);

    void insert_before(size_t index, const ContourPair& pair);
    void insert_after(size_t index, const ContourPair& pair);

    void new_insert(size_t index, CONTOUR_TYPE type = CONTOUR_TYPE::MAIN_CONTOUR);
    void new_front(CONTOUR_TYPE type = CONTOUR_TYPE::MAIN_CONTOUR);
    void new_back(CONTOUR_TYPE type = CONTOUR_TYPE::MAIN_CONTOUR);

    void clear();
    void leaveOne(size_t ctr_num);
private:
    void clear_properties();

public:
    bool remove(size_t index);

//    ContourPair* at(int index);
//    const ContourPair* at(int index) const;
    void shiftFirst(size_t index);
    void shiftLast(size_t index);
    void moveUp(size_t index);
    void moveDown(size_t index);

    void merge();

    ContourPair* at(size_t index);
    const ContourPair* at(size_t index) const;
    const SegmentEntity* at(size_t ctr, size_t row, size_t col);

    ContourPair* front();
    ContourPair* back();

    bool empty() const;
    size_t size() const;

    bool hasAnyTop() const;
    bool verify();
    const std::string& lastError();

    ContourRange range() const;
    void scale(double k, const fpoint_t& base);

    void select(size_t ctr_num);
//    void select(size_t ctr_num, size_t row_num, size_t col_num);

    // Contour number, segment number, {top, bot}
    void select(size_t ctr_num, const std::map<size_t, uint8_t>& seg_map);
    void select(const std::pair<size_t, size_t>& ctr_ent);
    void clearSelected();

    bool isContourSelected() const;
    bool isSegmentSelected() const;
    size_t selectedContour() const;
//    size_t selectedRow() const;
//    size_t selectedColumn() const;

    bool isSelected(size_t ctr, size_t row, size_t col) const;

    bool sort();
    bool isSorted() const;

    std::string toString() const;

    inline const std::deque<ContourPair>& contours() const { return m_contours; }

    static void shift(std::deque<ContourPair>& contours, const fpoint_t& pt);
    void shift(const fpoint_t& pt);
    void moveTo(const fpoint_t& pt = fpoint_t(0,0));
    fpoint_valid_t firstBot() const;
    fpoint_valid_t firstTop() const;

    void setXYPos(const fpoint_t& pt);
    void setUVPos(const fpoint_t& pt);
    void setBotPos(const fpoint_t& pt); // workpiece
    void clearPos();
    bool hasXYPos() const;
    bool hasUVPos() const;
    bool hasBotPos() const;
    const fpoint_t* xyPos() const;
    const fpoint_t* uvPos() const;
    const fpoint_t* botPos() const; // workpiece

    void setCurrent(int contour_num, int segment_num);
    int currentContourNumber() const { return m_cur_ctr; }
    int currentSegmentNumber() const { return m_cur_seg; }

    double botLengthFull();
    double botLength() const;
    double pct();

    void rotate(double phi);
    void flipX();
    void flipY();

    fpoint_t intersectUV() const;

    bool generateMux(
            const ContourPair* const pair,
            size_t items_num, size_t lanes_num,
            AXIS axis, DIR items_dir, DIR lanes_dir,
            const std::vector<size_t>& split_pts,
            double items_gap, double lanes_gap,
            double incut_len, double outcut_len
    );

    std::vector<size_t> getSplitPoints(size_t ctr_num, AXIS axis, DIR items_dir, DIR lanes_dir) const;

    static std::vector<size_t> ENWS_to_dir(const std::vector<size_t>& seg_num, AXIS axis, DIR items_dir, DIR lanes_dir);

    bool saveUndo();
    void clearUndo();
    void undo();
    bool hasUndo() const;

    bool find(const fpoint_t& pt, const ContourRange& range, size_t& ctr_num, size_t& row_num, size_t& col_num) const;

private:
    void connectContours();
};

#endif // CONTOURS_H
