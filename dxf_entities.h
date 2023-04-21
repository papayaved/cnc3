#ifndef DXF_ENTITIES_H
#define DXF_ENTITIES_H

#include "dxf_intersect.h"

#include "dxf_entity.h"
#include "dxf_line.h"
#include "dxf_arc.h"
#include "dxf_circle.h"
#include "dxf_point.h"

class DxfInsert : public DxfEntity {
    struct {
        uint8_t X:1;
        uint8_t Y:1;
        uint8_t valid:1;
    } flags;
    fpoint_t pt;
    std::string block_name;

public:
    DxfInsert();
    DxfInsert(const DxfInsert& other);
    ~DxfInsert() override;

    DxfEntity* clone() const override;

    void setX(double value);
    void setY(double value);

    bool check() override;
    bool valid() const override { return flags.valid; }
    void clear() override {
        DxfEntity::clear();
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

class DxfBlock {
    struct {
        uint8_t X:1;
        uint8_t Y:1;
        uint8_t valid:1;
    } flags;
    fpoint_t m_base;
    std::string m_name;
    std::deque<DxfEntity*> m_entities;

    void clearEntities();

public:
    DxfBlock();
    DxfBlock(const DxfBlock& other);
    DxfBlock(DxfBlock&& other);
    ~DxfBlock();

    DxfBlock& operator=(const DxfBlock& other);
    DxfBlock& operator=(DxfBlock&& other) noexcept;

    inline void setName(const std::string& value) { m_name = value; }
    inline const std::string& name() const { return m_name; }

    void setX(double value);
    void setY(double value);

    bool check();
    bool valid() const { return flags.valid; }

    const fpoint_t& base() const { return m_base; }

    void append(const DxfEntity& entity);
    const std::deque<DxfEntity*>& entities() const { return m_entities; }

    void clear();
    bool empty() const;    
};

#endif // DXF_ENTITIES_H
