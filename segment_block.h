#ifndef SEGMENT_BLOCK_H
#define SEGMENT_BLOCK_H

#include "segment_entity.h"
#include "segment_line.h"
#include "segment_arc.h"
#include "segment_circle.h"
#include "segment_point.h"
#include "segment_intersection.h"

// Block entity in DXF
class SegmentBlock {
    struct {
        uint8_t X:1;
        uint8_t Y:1;
        uint8_t valid:1;
    } flags;
    fpoint_t m_base;
    std::string m_name;
    std::deque<SegmentEntity*> m_entities;

    void clearEntities();

public:
    SegmentBlock();
    SegmentBlock(const SegmentBlock& other);
    SegmentBlock(SegmentBlock&& other);
    ~SegmentBlock();

    SegmentBlock& operator=(const SegmentBlock& other);
    SegmentBlock& operator=(SegmentBlock&& other) noexcept;

    inline void setName(const std::string& value) { m_name = value; }
    inline const std::string& name() const { return m_name; }

    void setX(double value);
    void setY(double value);

    bool check();
    bool valid() const { return flags.valid; }

    const fpoint_t& base() const { return m_base; }

    void append(const SegmentEntity& entity);
    const std::deque<SegmentEntity*>& entities() const { return m_entities; }

    void clear();
    bool empty() const;
};

// Insert entiry in DXF
class SegmentInsertBlock : public SegmentEntity {
    struct {
        uint8_t X:1;
        uint8_t Y:1;
        uint8_t valid:1;
    } flags;
    fpoint_t pt;
    std::string block_name;

public:
    SegmentInsertBlock();
    SegmentInsertBlock(const SegmentInsertBlock& other);
    ~SegmentInsertBlock() override;

    SegmentEntity* clone() const override;

    void setX(double value);
    void setY(double value);

    bool check() override;
    bool valid() const override { return flags.valid; }
    void clear() override {
        SegmentEntity::clear();
        flags = {0,0,0};
        pt = fpoint_t();
        block_name.clear();
    }

    void setBlockName(const std::string& value) { block_name = value; }
    std::string blockName() const override { return block_name; }

    const fpoint_t& pos() const { return pt; }
    fpoint_t point_0() const override { return pt; }
    fpoint_t point_1() const override { return pt; }
    fpoint_t center() const override { return pt; }
    ContourRange range() const override { return ContourRange(pt.x, pt.x, pt.y, pt.y); }

    void shift(const fpoint_t& offset) override { pt.shift(offset); }
    void reverse() override {}

    std::string toString() const override;
    std::string toString2() const override;

    std::vector<fpoint_t> getPoints() const override { return std::vector<fpoint_t>({pt}); }
    bool isPoint() const override { return true; }
};

#endif // SEGMENT_BLOCK_H
