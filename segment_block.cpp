#include "segment_block.h"
#include <cmath>

using namespace std;

// DxfInsert
SegmentInsertBlock::SegmentInsertBlock() : SegmentEntity(DXF_ENTITY_TYPE::INSERT), flags({0,0,0}), pt(fpoint_t()), block_name(std::string()) {}
SegmentInsertBlock::SegmentInsertBlock(const SegmentInsertBlock& other) : SegmentEntity(other, DXF_ENTITY_TYPE::INSERT), flags(other.flags), pt(other.pt), block_name(other.block_name) {}
SegmentInsertBlock::~SegmentInsertBlock() {}

SegmentEntity* SegmentInsertBlock::clone() const { return new SegmentInsertBlock(*this); }

void SegmentInsertBlock::setX(double value) {
    flags.valid = 0;
    flags.X = 1;
    pt.x = value;
}

void SegmentInsertBlock::setY(double value) {
    flags.valid = 0;
    flags.Y = 1;
    pt.y = value;
}

bool SegmentInsertBlock::check() {
    if (!flags.valid && flags.X && flags.Y)
        flags = {0,0,1};

    return flags.valid && !block_name.empty();
}

string SegmentInsertBlock::toString() const {
    return flags.valid ? "Insert Block: " + block_name + " at " + pt.toString() : "Not valid";
}

string SegmentInsertBlock::toString2() const {
    return flags.valid ? "Insert Block: " + block_name + " at " + pt.toString() : "Not valid";
}

// DxfBlock
void SegmentBlock::clearEntities() {
    for (SegmentEntity* o: m_entities)
        if (o) {
            delete o;
            o = nullptr;
        }

    m_entities.clear();
}

SegmentBlock::SegmentBlock() : flags({0,0,0}), m_base(fpoint_t(0, 0)) {}

SegmentBlock::SegmentBlock(const SegmentBlock& other) :
    flags(other.flags),
    m_base(other.m_base),
    m_name(other.m_name)
{
    for (const SegmentEntity* entity: other.m_entities)
        if (entity)
            m_entities.push_back(entity->clone());
}

SegmentBlock::SegmentBlock(SegmentBlock&& other) :
    flags(other.flags),
    m_base(other.m_base),
    m_name(move(other.m_name)),
    m_entities(move(other.m_entities))
{
}

SegmentBlock::~SegmentBlock() { clearEntities(); }

SegmentBlock& SegmentBlock::operator=(const SegmentBlock& other) {
    if (this != &other) {
        flags = other.flags;
        m_base = other.m_base;
        m_name = other.m_name;

        clearEntities();
        for (const SegmentEntity* entity: other.m_entities)
            if (entity)
                m_entities.push_back(entity->clone());
    }
    return *this;
}

SegmentBlock& SegmentBlock::operator=(SegmentBlock&& other) noexcept {
    if (this != &other) {
        flags = other.flags;
        m_base = other.m_base;
        m_name = move(other.m_name);

        clearEntities();
        m_entities = move(other.m_entities);
    }
    return *this;
}

void SegmentBlock::setX(double value) {
    flags.valid = 0;
    flags.X = 1;
    m_base.x = value;
}

void SegmentBlock::setY(double value) {
    flags.valid = 0;
    flags.Y = 1;
    m_base.y = value;
}

void SegmentBlock::append(const SegmentEntity& entity) {
    m_entities.push_back(entity.clone());
}

void SegmentBlock::clear() {
    flags = {0,0,0};
    m_base = fpoint_t();
    m_name.clear();
    clearEntities();
}

bool SegmentBlock::empty() const { return !flags.valid || m_name.empty() || m_entities.empty(); }

bool SegmentBlock::check() {
    if (!flags.valid && flags.X && flags.Y)
        flags = {0,0,1};

    return flags.valid && !m_name.empty() && !m_entities.empty();
}
