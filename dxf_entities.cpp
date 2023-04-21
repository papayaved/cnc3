#include "dxf_entities.h"
#include <cmath>

using namespace std;

// DxfInsert
DxfInsert::DxfInsert() : DxfEntity(ENTITY_TYPE::INSERT), flags({0,0,0}), pt(fpoint_t()), block_name(std::string()) {}
DxfInsert::DxfInsert(const DxfInsert& other) : DxfEntity(other, ENTITY_TYPE::INSERT), flags(other.flags), pt(other.pt), block_name(other.block_name) {}
DxfInsert::~DxfInsert() {}

DxfEntity* DxfInsert::clone() const { return new DxfInsert(*this); }

void DxfInsert::setX(double value) {
    flags.valid = 0;
    flags.X = 1;
    pt.x = value;
}

void DxfInsert::setY(double value) {
    flags.valid = 0;
    flags.Y = 1;
    pt.y = value;
}

bool DxfInsert::check() {
    if (!flags.valid && flags.X && flags.Y)
        flags = {0,0,1};

    return flags.valid && !block_name.empty();
}

string DxfInsert::toString() const {
    return flags.valid ? "Insert Block: " + block_name + " at " + pt.toString() : "Not valid";
}

string DxfInsert::toString2() const {
    return flags.valid ? "Insert Block: " + block_name + " at " + pt.toString() : "Not valid";
}

// DxfBlock
void DxfBlock::clearEntities() {
    for (DxfEntity* o: m_entities)
        if (o) {
            delete o;
            o = nullptr;
        }

    m_entities.clear();
}

DxfBlock::DxfBlock() : flags({0,0,0}), m_base(fpoint_t(0, 0)) {}

DxfBlock::DxfBlock(const DxfBlock& other) :
    flags(other.flags),
    m_base(other.m_base),
    m_name(other.m_name)
{
    for (const DxfEntity* entity: other.m_entities)
        if (entity)
            m_entities.push_back(entity->clone());
}

DxfBlock::DxfBlock(DxfBlock&& other) :
    flags(other.flags),
    m_base(other.m_base),
    m_name(move(other.m_name)),
    m_entities(move(other.m_entities))
{
}

DxfBlock::~DxfBlock() { clearEntities(); }

DxfBlock& DxfBlock::operator=(const DxfBlock& other) {
    if (this != &other) {
        flags = other.flags;
        m_base = other.m_base;
        m_name = other.m_name;

        clearEntities();
        for (const DxfEntity* entity: other.m_entities)
            if (entity)
                m_entities.push_back(entity->clone());
    }
    return *this;
}

DxfBlock& DxfBlock::operator=(DxfBlock&& other) noexcept {
    if (this != &other) {
        flags = other.flags;
        m_base = other.m_base;
        m_name = move(other.m_name);

        clearEntities();
        m_entities = move(other.m_entities);
    }
    return *this;
}

void DxfBlock::setX(double value) {
    flags.valid = 0;
    flags.X = 1;
    m_base.x = value;
}

void DxfBlock::setY(double value) {
    flags.valid = 0;
    flags.Y = 1;
    m_base.y = value;
}

void DxfBlock::append(const DxfEntity& entity) {
    m_entities.push_back(entity.clone());
}

void DxfBlock::clear() {
    flags = {0,0,0};
    m_base = fpoint_t();
    m_name.clear();
    clearEntities();
}

bool DxfBlock::empty() const { return !flags.valid || m_name.empty() || m_entities.empty(); }

bool DxfBlock::check() {
    if (!flags.valid && flags.X && flags.Y)
        flags = {0,0,1};

    return flags.valid && !m_name.empty() && !m_entities.empty();
}
