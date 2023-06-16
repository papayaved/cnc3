#ifndef CONTOUR_H
#define CONTOUR_H

#include "segment_block.h"
#include <algorithm>
#include "main.h"
#include <QVector>
#include <QPointF>
#include "frect_t.h"
#include "contour_range.h"
#include "my_types.h"
#include "my_lib.h"
#include <QList>
#include <set>

struct DxfCodeValue {
    int16_t code;
    std::string value;

    bool equal(int16_t code, const std::string& value) const;
    double getDouble() const;
};

// Contour
class Contour {
    constexpr static const double M_PRECISION = CncParam::PRECISION; // mm

    enum class STATES {SKIP, SECTION, BLOCKS, BLOCK, BLOCK_LINE, BLOCK_LWPOLYLINE, BLOCK_ARC, BLOCK_CIRCLE, BLOCK_POINT, BLOCK_ENTITY, ENTITIES, LINE, LWPOLYLINE, ARC, CIRCLE, POINT, INSERT, ENTITY};

    std::string m_fileDir, m_fileName;
    FILE* m_fp {nullptr};
    int m_ch_reg {0};

    std::deque<SegmentBlock*> m_blocks;
    std::list<SegmentEntity*> m_entities; // contour
    std::set<std::string> m_layers;

    QColor m_color {Qt::GlobalColor::blue};

    std::string m_last_error;

    data_valid_t<size_t> m_outIndex {data_valid_t<size_t>()};

    bool m_sorted {false};

    static STATES next_entity(Contour::STATES state, const DxfCodeValue& pair);
    STATES next_block_entity(Contour::STATES state, const DxfCodeValue& pair, SegmentBlock*& block);

    void init_entity(SegmentLine& line, const DxfCodeValue& pair);
    void init_polyline(SegmentPoint& point, SegmentLine& line, const DxfCodeValue& pair);

    void init_entity(SegmentArc& arc, const DxfCodeValue& pair);
    void init_entity(SegmentCircle& circle, const DxfCodeValue& pair);
    void init_entity(SegmentPoint& point, const DxfCodeValue& pair);

//    double x_min, x_max, y_min, y_max;

    std::string readString();
    DxfCodeValue getCodeValue();

    void clearEntities();
    void clearBlocks();

    void addInsert(const SegmentInsertBlock& insert);
    const SegmentBlock* getBlockByName(const std::string& blockName) const;

    std::list<SegmentEntity*>::iterator insertBlock(std::list<SegmentEntity*>::iterator it);
    void insertBlocks();

    static bool searchNext(
            const fpoint_t& pt1,
            std::list<SegmentEntity*>::iterator& it,
            const std::list<SegmentEntity*>::iterator& begin,
            const std::list<SegmentEntity*>::iterator& end
    );

    static bool searchPrev(
            const fpoint_t& pt0,
            std::list<SegmentEntity*>::iterator& it,
            const std::list<SegmentEntity*>::iterator& begin,
            const std::list<SegmentEntity*>::iterator& end
    );

    static size_t size(const std::list<SegmentEntity*> entities);

public:
    Contour();
    Contour(const Contour& other);
    Contour(Contour&& other);
    Contour(std::list<SegmentEntity*>& entities);
    ~Contour();

    Contour& operator=(const Contour& other);
    Contour& operator=(Contour&& other) noexcept;

    void init(const std::string& fileDir, const std::string& fileName);
    inline const std::string& fileName() const { return m_fileName; }

    bool open();
    inline bool isOpen() { return m_fp != nullptr; }
    void close();
    bool parse();
    void clear();

    size_t numberLayers() const { return m_layers.size(); }
    const std::set<std::string>& getLayers() const { return m_layers; }
    Contour getLayer(const std::string& layer_name) const;
    Contour cutLayer(const std::string& layer_name);

    void cutUnconnected(std::list<SegmentEntity*>& free);
    void cutTails(std::list<SegmentEntity*>& tails);
    void moveTwoTails(std::list<SegmentEntity*>& tails);
    static void shiftToBegin(std::list<SegmentEntity*>& entities, const fpoint_t* const first);
    static void shiftToBegin(std::list<SegmentEntity*>& entities, const fpoint_valid_t& first);

    void reversePrev(Contour* const prev);
    bool sort(Contour& free, Contour& tails, Contour* const prev = nullptr, bool prev_first = false);
//    bool sort(Contour& free, Contour& unused, const fpoint_valid_t& prev_pt = fpoint_valid_t(false));
    bool sort(Contour* const prev = nullptr, bool prev_first = false);
    bool trySort(Contour* const prev = nullptr, bool prev_first = false);

    bool checkSorted(fpoint_valid_t prev_pt = fpoint_valid_t(false), const fpoint_valid_t& next_pt = fpoint_valid_t(false));

    void reverse();
    static void shiftFirst(std::list<SegmentEntity*>& entities, size_t index);
    void shiftFirst(size_t index);
    void shiftLast(size_t index);

    void push_back(SegmentEntity* const entity);    
    void push_back(const SegmentEntity& entity);

    void push_back(const std::list<SegmentEntity*>& entities);

    // Move DxfEntities into currnet Dxf
    void move_back(Contour& ent);
    void move_back(Contour* const ent);

    void push_front(SegmentEntity* const entity);
    void push_front(const SegmentEntity& entity);

    fpoint_valid_t first_point() const;
    fpoint_valid_t last_point() const;
    const SegmentEntity* at(size_t index) const;

    inline const std::list<SegmentEntity*>& entities() const { return m_entities; }
    bool isLoop() const;

    bool hasOut() const;
    size_t getOutNum() const;
    fpoint_t getOutPoint() const;

    std::string lastError();

    bool isSorted() const { return m_sorted; }
//    GCode moveToGCode();

    static ContourRange contourRange(const std::list<SegmentEntity*>& contour);
    ContourRange contourRange() const;

    void printDebug() const;
    std::string toString() const;

    const QColor& color() const { return m_color; }
    size_t count() const;
    bool empty() const { return m_entities.empty(); }

    void moveFirst(size_t index);
    void moveLast(size_t index);
    void moveUp(size_t index);
    void moveDown(size_t index);

    Contour cut_front();
    Contour cut_back();
    Contour cut_at(size_t index);

    SegmentEntity* front() const { return m_entities.empty() ? nullptr : m_entities.front(); }
    SegmentEntity* back() const { return m_entities.empty() ? nullptr : m_entities.back(); }

    fpoint_valid_t first() const { return m_entities.empty() ? fpoint_valid_t(false) : m_entities.front()->point_0(); }
    fpoint_valid_t last() const { return m_entities.empty() ? fpoint_valid_t(false) : m_entities.back()->point_1(); }

    // Offset methods
    double length() const;
    double length(size_t index, const fpoint_t& pt) const;

    Contour* copy_front(double length) const;
    Contour* copy_front_rev(double tab) const;
    Contour* copy_back(double length) const;

    bool offset(OFFSET_SIDE offset_side, double offset);
    bool intersectLine(SegmentEntity*& A, SegmentEntity*& B);
    bool intersect(bool loop, OFFSET_SIDE side, double offset);
    void connectLoop();
    void alignDxfPoints();
    static std::list<SegmentEntity*>::iterator nextLineOrArc(
            const std::list<SegmentEntity*>::iterator& init,
            const std::list<SegmentEntity*>::iterator& begin,
            const std::list<SegmentEntity*>::iterator& end,
            bool& stop_req,
            bool loop = false
        );
    static std::list<SegmentEntity*>::iterator prevLineOrArc(
            const std::list<SegmentEntity*>::iterator& init,
            const std::list<SegmentEntity*>::iterator& begin,
            const std::list<SegmentEntity*>::iterator& end,
            bool& stop_req,
            bool loop = false
        );

    // return first entity index after split in forward order
    size_t split(double length);

    // return first entity index after split in forward order
    size_t split_rev(double tab);

    void remove_before(size_t index);
    void remove_to(size_t index);
    void remove_from(size_t index);
    void remove_after(size_t index);
    void remove(size_t index);

    void changeFirstPoint(const fpoint_valid_t& pt);
    void changeLastPoint(const fpoint_valid_t& pt);

    void shift(const fpoint_t& pt);

    std::vector<fpoint_t> getPoints() const;

    size_t joinedCount() const;
    bool whole() const;

    frect_t getRectangle() const;

    void rotate(const RotateMatrix& mx);
    void flipX(double x);
    void flipY(double y);
    void scale(double k);

    bool setOut(const fpoint_valid_t& pt = fpoint_valid_t::null());
    bool setOut(int seg_num);
    void setOut(const Contour* const next);

    void setInOut(size_t in_seg, size_t out_seg);
    Contour createMux(size_t in_seg, size_t out_seg) const;

    // East North West South IDs
    std::vector<size_t> getVerticesPoints() const;

private:
    void addRectSpeedProfile(AXIS axis, double width, double speed, bool fwd, fpoint_t& A, fpoint_t& B);
    double addCircleSpeedProfile(const snake_t& par, bool fwd, fpoint_t& A, fpoint_t& B);
    double addSlice(const snake_t& par, bool fwd, bool last, fpoint_t& A, fpoint_t& B);
    double addComb(const comb_t& par, bool fwd, bool last, fpoint_t& A, fpoint_t& B);

public:
    void generate(const snake_t& par);
    void generate(const comb_t& par);

    const Contour& buildLine(double len, AXIS axis, DIR dir);
};

#endif // CONTOUR_H
