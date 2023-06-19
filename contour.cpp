#include "contour.h"
#include <QDebug>
#include <cstdint>
#include <cstring>

#include "string_parser.h"

using namespace std;

// DxfCodeValue
bool DxfCodeValue::equal(int16_t code, const string& value) const {
    return this->code == code && this->value.compare(value) == 0;
}

double DxfCodeValue::getDouble() const {
    StringParser sp(value);
    double data;
    bool OK = sp.getDouble(data);
    return OK ? data : 0.0;
}

Contour::Contour() : m_fp(nullptr), m_ch_reg(0), m_color(Qt::GlobalColor::blue) {}

Contour::Contour(const Contour& other) :
    m_fileDir( other.m_fileDir ), m_fileName( other.m_fileName ), m_fp( nullptr ), m_ch_reg( 0 ),
    m_blocks( std::deque<SegmentBlock*>(other.m_blocks.size()) ), m_entities( std::list<SegmentEntity*>() ),
    m_layers( other.m_layers ),
    m_color( other.m_color ),
    m_last_error( other.m_last_error ),
    m_outIndex( other.m_outIndex ),
    m_sorted( other.m_sorted )
{
    for (size_t i = 0; i < m_blocks.size(); i++) {
        if (other.m_blocks[i])
            m_blocks[i] = new SegmentBlock( *other.m_blocks[i] );
    }

    for (list<SegmentEntity*>::const_iterator it = other.m_entities.cbegin(); it != other.m_entities.cend(); ++it) {
        if (*it)
            m_entities.push_back( (*it)->clone() );
    }
}

Contour::Contour(Contour&& other) :
    m_fileDir( std::move(other.m_fileDir) ), m_fileName( std::move(other.m_fileName) ), m_fp(other.m_fp), m_ch_reg(other.m_ch_reg),
    m_blocks( std::deque<SegmentBlock*>(other.m_blocks.size()) ), m_entities( std::list<SegmentEntity*>() ),
    m_layers( std::move(other.m_layers) ),
    m_color( other.m_color ),
    m_last_error( std::move(other.m_last_error) ),
    m_outIndex( other.m_outIndex ),
    m_sorted( other.m_sorted )
{
    for (size_t i = 0; i < m_blocks.size(); i++) {
        if (other.m_blocks[i]) {
            m_blocks[i] = other.m_blocks[i];
            other.m_blocks[i] = nullptr;
        }
    }

    other.m_blocks.clear();

    for (list<SegmentEntity*>::iterator it = other.m_entities.begin(); it != other.m_entities.end(); ++it) {
        if (*it) {
            m_entities.push_back(*it);
            *it = nullptr;
        }
    }

    other.m_entities.clear();

    other.m_fp = nullptr;
    other.m_ch_reg = 0;
    other.m_color = Qt::GlobalColor::blue;
    other.m_outIndex = data_valid_t<size_t>();
    other.m_sorted = false;
}

Contour::Contour(std::list<SegmentEntity*>& entities) {
    m_entities = entities;
    entities.clear();
}

Contour::~Contour() {
    clear();
    close();
}

Contour& Contour::operator=(const Contour& other) {
    if (this != &other) {
        clear();
        close();

        m_fileDir = other.m_fileDir;
        m_fileName = other.m_fileName;
        m_fp = nullptr;
        m_ch_reg = 0;
        m_color = other.m_color;
        m_last_error = "";

        m_outIndex = other.m_outIndex;

        m_sorted = other.m_sorted;

        m_blocks.resize(other.m_blocks.size());

        for (size_t i = 0; i < m_blocks.size(); i++) {
            if (other.m_blocks[i])
                m_blocks[i] = new SegmentBlock(*other.m_blocks[i]);
        }

        for (list<SegmentEntity*>::const_iterator it = other.m_entities.cbegin(); it != other.m_entities.cend(); ++it) {
            if (*it)
                m_entities.push_back( (*it)->clone() );
        }
    }

    return *this;
}

Contour& Contour::operator=(Contour&& other) noexcept {
    if (this != &other) {
        clear();
        close();

        m_fileDir = std::move(other.m_fileDir);
        m_fileName = std::move(other.m_fileName);
        m_last_error = std::move(other.m_last_error);

        m_blocks.resize(other.m_blocks.size());

        for (size_t i = 0; i < m_blocks.size(); i++) {
            if (other.m_blocks[i]) {
                m_blocks[i] = other.m_blocks[i];
                other.m_blocks[i] = nullptr;
            }
        }

        other.m_blocks.clear();

        for (list<SegmentEntity*>::iterator it = other.m_entities.begin(); it != other.m_entities.end(); ++it) {
            if (*it) {
                m_entities.push_back(*it);
                *it = nullptr;
            }
        }

        other.m_entities.clear();

        m_fp = other.m_fp;
        other.m_fp = nullptr;

        m_ch_reg = other.m_ch_reg;
        other.m_ch_reg = 0;

        m_color = other.m_color;
        other.m_color = Qt::GlobalColor::blue;

        m_outIndex = other.m_outIndex;
        other.m_outIndex = data_valid_t<size_t>();

        m_sorted = other.m_sorted;
        other.m_sorted = false;
    }

    return *this;
}

void Contour::clearEntities() {
    for (list<SegmentEntity*>::iterator it = m_entities.begin(); it != m_entities.end(); ++it)
        if (*it) {
            delete *it;
            *it = nullptr;
        }

    m_entities.clear();
}

void Contour::clearBlocks() {
    for (size_t i = 0; i < m_blocks.size(); i++)
        if (m_blocks[i]) {
            delete m_blocks[i];
            m_blocks[i] = nullptr;
        }

    m_blocks.clear();
}

void Contour::clear() {
    clearBlocks();
    clearEntities();
    m_layers.clear();
    m_last_error.clear();

    m_outIndex = data_valid_t<size_t>();

    m_sorted = false;
    m_ch_reg = 0;
}

Contour Contour::getLayer(const string &layer_name) const {
    Contour res;

    if (layer_name.length() > 0) {
        for (const SegmentEntity* ent: m_entities) {
            string s = ent->layerName();

            if ( s.compare(layer_name) == 0 ) {
                res.m_entities.push_back(ent->clone());
            }
        }
    }

    return res;
}

Contour Contour::cutLayer(const string &layer_name) {
    Contour res;

    for (list<SegmentEntity*>::iterator it = m_entities.begin(); it != m_entities.end(); ++it) {
        if ( (*it)->layerName().compare(layer_name) == 0 ) {
            res.m_entities.push_back(*it);
//            *it = nullptr;
            it = m_entities.erase(it);
        }
    }

    set<string>::iterator it = m_layers.find(layer_name);
    m_layers.erase(it);

    return res;
}

void Contour::init(const string &fileDir, const string &fileName) {
    clear();
    close();
    m_fileDir = fileDir;
    m_fileName = fileName;
}

bool Contour::open() {
    string file_name = m_fileDir + "/" + m_fileName;
    m_fp = fopen(file_name.c_str(), "r");
    m_ch_reg = 0;

    if (!m_fp) {
        qDebug() << QString("File \"%1\" isn't found\n").arg(file_name.c_str());
        return false;
    }

    qDebug() << QString("File \"%1\" is opened").arg(file_name.c_str());
    return true;
}

void Contour::close() {
    if (m_fp) {
        fclose(m_fp);
        m_fp = nullptr;
    }

    m_ch_reg = 0;
}

Contour::STATES Contour::next_entity(Contour::STATES state, const DxfCodeValue& pair) {
    if (pair.equal(0, "ENDSEC"))
        return STATES::SKIP;
    else if (pair.equal(0, "LINE"))
        return STATES::LINE;
    else if (pair.equal(0, "LWPOLYLINE"))
        return STATES::LWPOLYLINE;
    else if (pair.equal(0, "ARC"))
        return STATES::ARC;
    else if (pair.equal(0, "CIRCLE"))
        return STATES::CIRCLE;
    else if (pair.equal(0, "POINT"))
        return STATES::POINT;
    else if (pair.equal(0, "INSERT"))
        return STATES::INSERT;
    else if (pair.code == 0)
        return STATES::ENTITY;

    return state;
}

Contour::STATES Contour::next_block_entity(Contour::STATES state, const DxfCodeValue& pair, SegmentBlock*& block) {
    if (pair.equal(0, "ENDBLK")) {
        if (block)
            if (block->check()) {
                m_blocks.push_back(block);
                block = new SegmentBlock;
            }
            else
                block->clear();
        else
            block = new SegmentBlock;

        return STATES::BLOCKS;
    }
    else if (pair.equal(0, "LINE"))
        return STATES::BLOCK_LINE;
    else if (pair.equal(0, "LWPOLYLINE"))
        return STATES::BLOCK_LWPOLYLINE;
    else if (pair.equal(0, "ARC"))
        return STATES::BLOCK_ARC;
    else if (pair.equal(0, "CIRCLE"))
        return STATES::BLOCK_CIRCLE;
    else if (pair.equal(0, "POINT"))
        return STATES::BLOCK_POINT;
    else if (pair.code == 0)
        return STATES::BLOCK_ENTITY;

    return state;
}

void Contour::init_entity(SegmentLine& line, const DxfCodeValue& pair) {
    switch (pair.code) {
    case 8:
        line.setLayerName(pair.value);
        m_layers.insert(pair.value);
        break;
    case 10:
        line.setX0(pair.getDouble());
        break;
    case 20:
        line.setY0(pair.getDouble());
        break;
    case 11:
        line.setX1(pair.getDouble());
        break;
    case 21:
        line.setY1(pair.getDouble());
        break;
    }
}

void Contour::init_polyline(SegmentPoint& prev_point, SegmentLine& line, const DxfCodeValue& pair) {
    switch (pair.code) {
    case 8:
        line.setLayerName(pair.value);
        break;        
    case 10:
        if (prev_point.isX()) {
            line.setX0(prev_point.X());
            line.setX1(pair.getDouble());
            prev_point.setX(pair.getDouble());
        }
        else {
            prev_point.setX(pair.getDouble());
        }

        break;
    case 20:
        if (prev_point.isY()) {
            line.setY0(prev_point.Y());
            line.setY1(pair.getDouble());
            prev_point.setY(pair.getDouble());
        }
        else {
            prev_point.setY(pair.getDouble());
        }

        break;
    }

    if (line.check())
        m_layers.insert(line.layerName());
}

void Contour::init_entity(SegmentArc& arc, const DxfCodeValue& pair) {
    switch (pair.code) {
    case 8:
        arc.setLayerName(pair.value);
        m_layers.insert(pair.value);
        break;
    case 10:
        arc.setCenterX(pair.getDouble());
        break;
    case 20:
        arc.setCenterY(pair.getDouble());
        break;
    case 40:
        arc.setRadius(pair.getDouble());
        break;
    case 50:
        arc.setStartAngle(pair.getDouble() * (M_PI / 180));
        break;
    case 51:
        arc.setEndAngle(pair.getDouble() * (M_PI / 180));
        break;
    case 230:
        arc.setExtrusionDirectionZ(pair.getDouble());
        break;
    }
}

void Contour::init_entity(SegmentCircle& circle, const DxfCodeValue& pair) {
    switch (pair.code) {
    case 8:
        circle.setLayerName(pair.value);
        m_layers.insert(pair.value);
        break;
    case 10:
        circle.setCenterX(pair.getDouble());
        break;
    case 20:
        circle.setCenterY(pair.getDouble());
        break;
    case 40:
        circle.setRadius(pair.getDouble());
        break;
    case 230:
        circle.setExtrusionDirectionZ(pair.getDouble());
        break;
    }
}

void Contour::init_entity(SegmentPoint& point, const DxfCodeValue& pair) {
    switch (pair.code) {
    case 8:
        point.setLayerName(pair.value);
        m_layers.insert(pair.value);
        break;
    case 10:
        point.setX(pair.getDouble());
        break;
    case 20:
        point.setY(pair.getDouble());
        break;
    }
}

bool Contour::parse() {
    STATES state = STATES::SKIP;
    SegmentBlock* block = new SegmentBlock;
    SegmentLine line;
    SegmentArc arc;
    SegmentCircle circle;
    SegmentPoint point;
    SegmentInsertBlock insert;

    clear();
    open();

    if (m_fp) {
         while (!feof(m_fp)) {
            DxfCodeValue pair = getCodeValue();

            if (pair.code == INT16_MIN) { // error
                if (feof(m_fp))
                    break;
                else {
                    if (block)
                        delete block;

                    return false;
                }
            }

            switch (state) {
            case STATES::SKIP:
                if (pair.equal(0, "SECTION"))
                    state = STATES::SECTION;
                break;

            case STATES::SECTION:
                if (pair.equal(2, "BLOCKS"))
                    state = STATES::BLOCKS;
                else if (pair.equal(2, "ENTITIES"))
                    state = STATES::ENTITIES;
                else
                    state = STATES::SKIP;
                break;

            case STATES::BLOCKS:
                if (pair.equal(0, "ENDSEC"))
                    state = STATES::SKIP;
                else if (pair.equal(0, "BLOCK"))
                    state = STATES::BLOCK;
                break;

            case STATES::BLOCK:
                switch (pair.code) {
                case 2: case 3:
                    if (block) block->setName(pair.value);
                    break;
                case 10:
                    if (block) block->setX(pair.getDouble());
                    break;
                case 20:
                    if (block) block->setY(pair.getDouble());
                    break;
                case 0:
                    state = next_block_entity(state, pair, block);
                    break;
                }
                break;

            case STATES::BLOCK_LINE:
                init_entity(line, pair);

                if (pair.code == 0 && block) {
                    if (line.check())
                        block->append(line);

                    line.clear();
                    state = next_block_entity(state, pair, block);
                }
                break;

            case STATES::BLOCK_LWPOLYLINE:
                init_polyline(point, line, pair);

                if (line.check() && block) {
                    block->append(line);
                    line.clearWithoutLayer();
                }

                if (pair.code == 0) {
                    point.clear();
                    line.clear();
                    state = next_block_entity(state, pair, block);
                }
                break;

            case STATES::BLOCK_ARC:
                init_entity(arc, pair);

                if (pair.code == 0 && block) {
                    if (arc.check())
                        block->append(arc);

                    arc.clear();
                    state = next_block_entity(state, pair, block);
                }
                break;

            case STATES::BLOCK_CIRCLE:
                init_entity(circle, pair);

                if (pair.code == 0 && block) {
                    if (circle.check())
                        block->append(circle);

                    circle.clear();
                    state = next_block_entity(state, pair, block);
                }
                break;

            case STATES::BLOCK_POINT:
                init_entity(point, pair);

                if (pair.code == 0 && block) {
                    if (point.check())
                        block->append(point);

                    point.clear();
                    state = next_block_entity(state, pair, block);
                }
                break;

            case STATES::BLOCK_ENTITY:
                if (block)
                    state = next_block_entity(state, pair, block);
                break;

            case STATES::ENTITIES:
                state = next_entity(state, pair);
                break;

            case STATES::LINE:
                init_entity(line, pair);

                if (pair.code == 0) {
                    if (line.check())
                        push_back(line);

                    line.clear();
                    state = next_entity(state, pair);
                }
                break;

            case STATES::LWPOLYLINE:
                init_polyline(point, line, pair);

                if (line.check()) {
                    push_back(line);
                    line.clearWithoutLayer();
                }

                if (pair.code == 0) {
                    point.clear();
                    line.clear();
                    state = next_entity(state, pair);
                }
                break;

            case STATES::ARC:
                init_entity(arc, pair);

                if (pair.code == 0) {
                    if (arc.check())
                        push_back(arc);

                    arc.clear();
                    state = next_entity(state, pair);
                }
                break;

            case STATES::CIRCLE:
                init_entity(circle, pair);

                if (pair.code == 0) {
                    if (circle.check())
                        push_back(circle);

                    circle.clear();
                    state = next_entity(state, pair);
                }
                break;

            case STATES::POINT:
                init_entity(point, pair);

                if (pair.code == 0) {
                    if (point.check())
                        push_back(point);

                    point.clear();
                    state = next_entity(state, pair);
                }
                break;

            case STATES::INSERT:
                switch (pair.code) {
                case 2:
                    insert.setBlockName(pair.value);
                    break;
                case 10:
                    insert.setX(pair.getDouble());
                    break;
                case 20:
                    insert.setY(pair.getDouble());
                    break;
                case 0:
                    if (insert.check()) {
                        addInsert(insert);
                        insert.clear();
                    }

                    state = next_entity(state, pair);

                    break;
                }
                break;

            case STATES::ENTITY:
                state = next_entity(state, pair);
                break;

//            default:
//                return false;
            }

         }
    }

    if (block)
        delete block;

    insertBlocks();

    printDebug();
    //    qDebug("m_entities.size=%d %d", (int)size(m_entities), (int)m_entities.size());
    //    qDebug() << toString().c_str();

    return true;
}

void Contour::cutUnconnected(list<SegmentEntity*>& free) {
    free.clear();
    bool find = false;
    fpoint_t A1{0,0}, B1{0,0}, A2{0,0}, B2{0,0};

    if (m_entities.size() < 2)
        return;

    for (list<SegmentEntity*>::iterator i = m_entities.begin(); i != m_entities.end();) {
        if (*i == nullptr) {
            i = m_entities.erase(i);
            continue;
        }

        list<SegmentEntity*>::iterator j = i;
        A1 = (*i)->point_0();
        B1 = (*i)->point_1();

        ++j;
        if (j == m_entities.end())
            j = m_entities.begin();

        find = false;
        while (j != i) {
            if (*j == nullptr) {
                ++j;
                continue;
            }

            A2 = (*j)->point_0();
            B2 = (*j)->point_1();

            if (A1 == A2 || A1 == B2 || B1 == A2 || B1 == B2) {
                find = true;
                break;
            }

            ++j;
            if (j == m_entities.end())
                j = m_entities.begin();
        }

        if (!find) {
            free.push_back(*i);
            i = m_entities.erase(i);
        }
        else
            ++i;
    }
}

// if more than 2 entities
void Contour::cutTails(list<SegmentEntity*>& tails) {
    tails.clear();
    bool a = false, b = false;
    fpoint_t A1{0,0}, B1{0,0}, A2{0,0}, B2{0,0};

    if (m_entities.size() < 2)
        return;

    bool incut = true;
    for (list<SegmentEntity*>::iterator i = m_entities.begin(); i != m_entities.end();) {
        if (*i == nullptr) {
            ++i;
            continue;
        }

        list<SegmentEntity*>::iterator j = i;
        A1 = (*i)->point_0();
        B1 = (*i)->point_1();

        ++j;
        if (j == m_entities.end())
            j = m_entities.begin();

        a = b = false;
        while (j != i) {
            if (*j == nullptr) {
                ++j;
                continue;
            }

            A2 = (*j)->point_0();
            B2 = (*j)->point_1();

            if (!a && (A1 == A2 || A1 == B2))
                a = true;

            if (!b && (B1 == A2 || B1 == B2))
                b = true;

            if (a && b)
                break;

            ++j;
            if (j == m_entities.end())
                j = m_entities.begin();
        }

        for (list<SegmentEntity*>::iterator k = tails.begin(); k != tails.end() && !(a && b); ++k) {
            if (*k == nullptr)
                continue;

            A2 = (*k)->point_0();
            B2 = (*k)->point_1();

            if (!a && (A1 == A2 || A1 == B2))
                a = true;

            if (!b && (B1 == A2 || B1 == B2))
                b = true;
        }

        if (a ^ b) {
            if (incut && (*i)->type() == DXF_ENTITY_TYPE::LINE && a) { // A connected
                (*i)->reverse();
                incut = false;
            }

            tails.push_back(*i);
            i = m_entities.erase(i);
        }
        else
            ++i;
    }
}

// Move back two tails: head and tail is they exist
void Contour::moveTwoTails(std::list<SegmentEntity*>& tail) {
    if (!tail.empty() && tail.front()) {
        m_entities.push_front(tail.front());
        tail.erase(tail.begin());
    }

    if (!tail.empty() && tail.back()) {
        m_entities.push_back(tail.back());
        tail.erase(--tail.end());
    }
}

void Contour::shiftToBegin(list<SegmentEntity*>& entities, const fpoint_t* const first_ptr) {
    if (first_ptr)
        shiftToBegin(entities, fpoint_valid_t(first_ptr));
}

void Contour::shiftToBegin(std::list<SegmentEntity *> &entities, const fpoint_valid_t &first) {
    if (first.valid) {
        list<SegmentEntity*>::iterator it = entities.begin();

        for (; it != entities.end(); ++it) {
             if ((*it)->point_0() == first)
                 break;
             else if ((*it)->point_1() == first) {
                 (*it)->reverse();
                 break;
            }
        }

        if (it == entities.end())
            return;

        std::list<SegmentEntity*> head;

        head.splice(head.begin(), entities, entities.begin(), it); // transfer to the head

        it = entities.end();
        entities.splice(it, head); // add the head to the tail
    }
}

void Contour::reversePrevIf(Contour* const prev) {
    bool rev_req = false;

    if (!prev->isLoop()) {
        for (SegmentEntity* const ent : m_entities) {
            if (prev->last() == ent->point_0())
                return;
            else if (prev->last() == ent->point_1()) {
                ent->reverse();
                return;
            }
            else if (prev->first() == ent->point_0()) {
                rev_req = true;
                break;
            }
            else if (prev->first() == ent->point_1()) {
                ent->reverse();
                rev_req = true;
                break;
            }
        }

        if (rev_req)
            prev->reverse();
    }
}

fpoint_valid_t Contour::searchPrevLast(const Contour* const prev) const {
    if (prev && !prev->empty()) {
        if (prev->hasOut())
            return fpoint_valid_t(prev->getOutPoint());

        if (!prev->isLoop())
            return fpoint_valid_t(prev->last_point());

        for (const auto& p: prev->m_entities) {
            for (const auto& t: this->m_entities) {
                if (p->point_1() == t->point_0() || p->point_1() == t->point_1())
                    return fpoint_valid_t(p->point_1());
                else if (p->point_0() == t->point_0() || p->point_0() == t->point_1())
                    return fpoint_valid_t(p->point_0());
            }
        }
    }

    return fpoint_valid_t(false);
}

bool Contour::sort(Contour& free, Contour& tails, Contour* const prev, bool prev_first) {
    m_sorted = false;

    if (m_entities.empty())
        return false;    

    cutUnconnected(free.m_entities);
    qDebug() << "Free segments:";
    free.printDebug();
    qDebug("m_entities.size=%d %d", (int)size(m_entities), (int)m_entities.size());

    cutTails(tails.m_entities);
    qDebug() << "Tails segments:";
    tails.printDebug();
    qDebug("m_entities.size=%d %d", (int)size(m_entities), (int)m_entities.size());

    moveTwoTails(tails.m_entities); // move back a head and tail if they exist
    qDebug() << "Tails segments:";
    tails.printDebug();
    qDebug("m_entities.size=%d %d", (int)size(m_entities), (int)m_entities.size());

    // Sort
    list<SegmentEntity*> clone(0), sorted(0); // they are only copies of pointers
    list<SegmentEntity*>::iterator it;

    fpoint_valid_t prev_last(false);

    if (prev && !prev->empty()) {
        vector<fpoint_t> prev_pts;

        if (prev->isLoop())
            prev_pts = prev->vertices();
        else
            prev_pts = {prev->last()};

        // Reverse previous contour if need
        if (prev_first)
            reversePrevIf(prev);

        // Find fisrt common point
        prev_last = searchPrevLast(prev);

        if (m_entities.size() == 1) {
            if (prev_last == this->first())
                m_sorted = true;
            else if (prev_last == this->last()) {
                this->reverse();
                m_sorted = true;
            }

            return m_sorted;
        }

        shiftToBegin(m_entities, prev_last);

        clone = m_entities;
        sorted.clear();

        it = clone.begin();

        sorted.push_back(*it);
        it = clone.erase(it);

        while (!clone.empty()) {
            bool OK = sorted.back() ? searchNext(sorted.back()->point_1(), it, clone.begin(), clone.end()) : false;

            if (OK) {
                sorted.push_back(*it);
                it = clone.erase(it);
            }
            else
                break;
        }

        while (!clone.empty()) {
            bool OK = sorted.front() ? searchPrev(sorted.front()->point_0(), it, clone.begin(), clone.end()) : false;

            if (OK) {
                sorted.push_front(*it);
                it = clone.erase(it);
            }
            else
                break;
        }
    }
    else { // the first point is unknown
        if (m_entities.size() == 1) {
            m_sorted = true;
            return m_sorted;
        }

        for (size_t i = 0; i < m_entities.size(); i++) {
            for (int j = 0; j < 2; j++) {
                clone = m_entities;
                sorted.clear();

                shiftFirst(clone, i);

                it = clone.begin();

                if (j == 1)
                    (*it)->reverse();

                sorted.push_back(*it);
                it = clone.erase(it);

                while (!clone.empty()) {
                    bool OK = sorted.back() ? searchNext(sorted.back()->point_1(), it, clone.begin(), clone.end()) : false;

                    if (OK) {
                        sorted.push_back(*it);
                        it = clone.erase(it);
                    }
                    else
                        break;
                }

                while (!clone.empty()) {
                    bool OK = sorted.front() ? searchPrev(sorted.front()->point_0(), it, clone.begin(), clone.end()) : false;

                    if (OK) {
                        sorted.push_front(*it);
                        it = clone.erase(it);
                    }
                    else
                        break;
                }

                if (clone.empty())
                    break;

                // reverse first segment
            }

            if (clone.empty())
                break;

            // try to set as the first segment the next segment
        }
    }

    if (clone.empty()) {
        m_entities = sorted;
        qDebug() << "Sorted:";
        qDebug() << toString().c_str();

        return checkSorted(prev_last);
    }

    m_last_error = clone.front() ? "Error at " + clone.front()->toString() : "Error: DxfEntity is NULL";
    qDebug() << toString().c_str();

    return false;
}

//bool Contour::sort(Contour& free, Contour& tails, const fpoint_valid_t& prev_pt) {
//    if (m_entities.empty())
//        return false;

//    if (m_entities.size() == 1) {
//        if (prev_pt.valid) {
//            if (m_entities.front()->point_1() == prev_pt)
//                m_entities.front()->reverse();

//            return m_entities.front()->point_0() == prev_pt;
//        }
//        else
//            return true;
//    }

//    cutUnconnected(free.m_entities);
//    qDebug() << "Free segments:";
//    free.printDebug();
//    qDebug("m_entities.size=%d %d", (int)size(m_entities), (int)m_entities.size());

//    cutTails(tails.m_entities);
//    qDebug() << "Tails segments:";
//    tails.printDebug();
//    qDebug("m_entities.size=%d %d", (int)size(m_entities), (int)m_entities.size());

//    moveTwoTails(tails.m_entities); // it can be only one
//    qDebug() << "Tails segments:";
//    tails.printDebug();
//    qDebug("m_entities.size=%d %d", (int)size(m_entities), (int)m_entities.size());

//    // Search first
//    shiftToBegin(m_entities, prev_pt);

//    // Sort
//    list<SegmentEntity*> clone(0), sorted(0); // they are only copies of pointers
//    list<SegmentEntity*>::iterator it;

//    for (size_t i = 0; i < m_entities.size(); i++) {
//        clone = m_entities;
//        sorted.clear();

//        shiftFirst(clone, i);

//        it = clone.begin();

//        sorted.push_back(*it);
//        it = clone.erase(it);

//        while (!clone.empty()) {
//            bool OK = sorted.back() ? searchNext(sorted.back()->point_1(), it, clone.begin(), clone.end()) : false;

//            if (OK) {
//                sorted.push_back(*it);
//                it = clone.erase(it);
//            }
//            else
//                break;
//        }

//        while (!clone.empty()) {
//            bool OK = sorted.front() ? searchPrev(sorted.front()->point_0(), it, clone.begin(), clone.end()) : false;

//            if (OK) {
//                sorted.push_front(*it);
//                it = clone.erase(it);
//            }
//            else
//                break;
//        }

//        if (clone.empty()) {
//            m_entities = sorted;
//            m_sorted = true;
//            qDebug() << "Sorted:";
//            qDebug() << toString().c_str();

//            return true;
//        }

//        // try to set as the first segment the next segment
//    }

//    m_last_error = clone.front() ? "Error at " + clone.front()->toString() : "Error: DxfEntity is NULL";
//    qDebug() << toString().c_str();

//    return false;
//}

// Sort and delete unused
bool Contour::sort(Contour* const prev, bool prev_first) {
    Contour free, tails;

    if (m_entities.empty())
        return false;

    bool OK = sort(free, tails, prev, prev_first);
    return OK && free.empty() && tails.empty();
}

// Sort and add to the back unsorted segments
bool Contour::trySort(Contour* const prev, bool prev_first) {
    Contour free, tails;

    if (m_entities.empty())
        return false;

    bool OK = sort(free, tails, prev, prev_first);
    OK &= free.empty() && tails.empty();

    this->move_back(tails);
    this->move_back(free);

    return OK;
}

bool Contour::fixReverse(fpoint_valid_t prev_last) {
    m_sorted = true;

    for (list<SegmentEntity*>::iterator it = m_entities.begin(); it != m_entities.end(); ++it) {
        if (prev_last.valid) {
            if (prev_last != (*it)->point_0())
                ;
            else if (prev_last != (*it)->point_1())
                (*it)->reverse();
            else {
                m_sorted = false;
                break;
            }
        }

        prev_last = (*it)->point_1();
        prev_last.valid = true;
    }

    return m_sorted;
}

bool Contour::checkSorted(fpoint_valid_t prev_last) {
    m_sorted = true;

    for (list<SegmentEntity*>::const_iterator it = m_entities.cbegin(); it != m_entities.cend(); ++it) {
        if (prev_last.valid && prev_last != (*it)->point_0()) {
            m_sorted = false;
            break;
        }

        prev_last = (*it)->point_1();
        prev_last.valid = true;
    }

    return m_sorted;
}

void Contour::reverse() {
    m_entities.reverse();
    for (SegmentEntity* ent: m_entities) {
        if (ent)
            ent->reverse();
    }
    //    sort();
}

fpoint_valid_t Contour::first_point() const {
    if (!m_entities.empty() && m_entities.front())
        return m_entities.front()->point_0();

    return fpoint_valid_t(false);
}

fpoint_valid_t Contour::last_point() const {
    if (!m_entities.empty() && m_entities.back())
        return m_entities.back()->point_1();

    return fpoint_valid_t(false);
}

SegmentEntity *Contour::at(size_t index) {
    m_sorted = false;
    size_t i = 0;

    if (index < m_entities.size())
        for (auto it = m_entities.begin(); it != m_entities.end(); ++it, i++)
            if (i == index)
                return *it;

    return nullptr;
}

const SegmentEntity *Contour::at(size_t index) const {
    size_t i = 0;

    if (index < m_entities.size())
        for (auto it = m_entities.cbegin(); it != m_entities.cend(); ++it, i++)
            if (i == index)
                return *it;

    return nullptr;
}

bool Contour::isLoop() const {
    return m_entities.size() > 1 && m_entities.back() && m_entities.front() && m_entities.back()->point_1() == m_entities.front()->point_0();
}

bool Contour::hasOut() const {
    return m_outIndex.valid && m_entities.size() != 0 && m_outIndex.data < (m_entities.size() - 1);
}

size_t Contour::getOutNum() const {
    if (m_outIndex.valid && isLoop() && m_outIndex.data < m_entities.size())
        return m_outIndex.data;
    else if (m_entities.empty())
        return 0;

    return m_entities.size() - 1;
}

fpoint_t Contour::getOutPoint() const {
    size_t out_num = getOutNum();
    const SegmentEntity* ent = at(out_num);

    if (ent)
        return ent->point_1();

    return fpoint_t(0, 0);
}

bool Contour::searchNext(
    const fpoint_t& pt1,
    list<SegmentEntity*>::iterator& it,
    const list<SegmentEntity*>::iterator& begin,
    const list<SegmentEntity*>::iterator& end
) {
    if (begin == end)
        return false;

    if (it == end)
        it = begin;

    list<SegmentEntity*>::iterator start = it;

    do {
        SegmentEntity*& ent = *it;

        if (ent) {
            fpoint_t A = ent->point_0();
            fpoint_t B = ent->point_1();

            if (A == pt1)
                return true;

            if (B == pt1) {
                ent->reverse();
                return true;
            }
        }

        ++it;

        if (it == end)
            it = begin;
    } while (it != start);

    return false;
}

bool Contour::searchPrev(
    const fpoint_t& pt0,
    list<SegmentEntity*>::iterator& it,
    const list<SegmentEntity*>::iterator& begin,
    const list<SegmentEntity*>::iterator& end
) {
    if (begin == end)
        return false;

    if (it == end)
        --it;

    list<SegmentEntity*>::iterator start = it;

    do {
        SegmentEntity*& ent = *it;

        if (ent) {
            fpoint_t A = ent->point_0();
            fpoint_t B = ent->point_1();

            if (B == pt0)
                return true;

            if (A == pt0) {
                ent->reverse();
                return true;
            }
        }

        if (it == begin)
            it = end;
    } while (--it != start);

    return false;
}

size_t Contour::size(const std::list<SegmentEntity*> entities) {
    size_t i = 0;

    for (std::list<SegmentEntity*>::const_iterator it = entities.cbegin(); it != entities.cend(); ++it)
        i++;

    return i;
}

string Contour::lastError() {
    string res = m_last_error;
    m_last_error.clear();
    return res;
}

DxfCodeValue Contour::getCodeValue() {
    DxfCodeValue res;

    StringParser sp_code(readString());
    sp_code.removeForwardRearSpaces();

    int data;
    bool OK = sp_code.getInt(data);

    if (OK && data >= INT16_MIN && data <= INT16_MAX) {
        res.code = static_cast<int16_t>(data);
        StringParser sp_value(readString());
        sp_value.removeForwardRearSpaces();
        res.value = sp_value.getString();
    }
    else
        res.code = INT16_MIN;

    return res;
}

void Contour::push_back(SegmentEntity* const entity) {
    if (entity)
        m_entities.push_back(entity);
}

void Contour::push_back(const SegmentEntity& entity) {
    push_back(entity.clone());
}

void Contour::push_back(const std::list<SegmentEntity *> &entities) {
    for (SegmentEntity* ent : entities) {
        if (ent)
            push_back(ent->clone());
    }
}

void Contour::move_back(Contour* const ent) {
    if (ent) {
        m_entities.splice(m_entities.end(), ent->m_entities);
//        ent->m_entities.clear();
    }
}

void Contour::move_back(Contour& ent) { move_back(&ent); }

//void Dxf::move_back(Dxf& ent) {
//    for (DxfEntity*& ent : ent.m_entities)
//        if (ent) {
//            m_entities.push_back(ent);
//            ent = nullptr;
//        }

//    ent.m_entities.clear();
//}

void Contour::push_front(SegmentEntity* const entity) {
    if (entity)
        m_entities.push_front(entity);
}

void Contour::push_front(const SegmentEntity& entity) {
    push_front(entity.clone());
}

void Contour::addInsert(const SegmentInsertBlock &insert) {
    SegmentInsertBlock* const new_insert = new SegmentInsertBlock(insert);
    m_entities.push_back(new_insert);
}

list<SegmentEntity*>::iterator Contour::insertBlock(list<SegmentEntity*>::iterator it) {
    SegmentEntity*& o = *it;
    SegmentInsertBlock* insert = nullptr;

    if (o && o->type() == DXF_ENTITY_TYPE::INSERT) {
        insert = dynamic_cast<SegmentInsertBlock*>(o); // copy

        if (insert) {
            it = m_entities.erase(it);
            const SegmentBlock* block = getBlockByName(insert->blockName());

            if (block) {
                const fpoint_t& block_pos = insert->pos();
                const fpoint_t& block_base = block->base();

                fpoint_t offset;
                offset.x = block_pos.x - block_base.x;
                offset.y = block_pos.y - block_base.y;

                for (const SegmentEntity* entity: block->entities()) {
                    if (entity && entity->type() == DXF_ENTITY_TYPE::LINE) {
                        if (const SegmentLine* line = dynamic_cast<const SegmentLine*>(entity)) {
                            SegmentLine* const new_line = new SegmentLine(*line);
                            new_line->shift(offset);
                            it = m_entities.insert(it, new_line);
                        }
                    }
                }
            }

            delete insert;
        }
    }

    return it;
}

void Contour::insertBlocks() {
    for (auto it = m_entities.begin(); it != m_entities.end(); ++it)
        it = insertBlock(it);

    clearBlocks();
}

const SegmentBlock* Contour::getBlockByName(const string &blockName) const {
    for (auto it = m_blocks.cbegin(); it != m_blocks.cend(); ++it) {
        if (*it && (*it)->name().compare(blockName) == 0)
            return *it;
    }

    return nullptr;
}

string Contour::readString() {
    string s;
    s.resize(256);
    s.clear();

    if (m_fp) {
        while (!feof(m_fp)) {
            int ch = fgetc(m_fp);

            if (ferror(m_fp)) {
                qDebug("Read file error\n");
                s.clear();
                break;
            }
            else if (m_ch_reg == '\r') { // Windows or Mac
                m_ch_reg = 0;

                if (ch == '\n') // Windows skip char
                    ;
                else if (ch == '\0' || ch == '\r') // exit
                    break;
                else
                    s += static_cast<char>(ch);
            }
            else {
                if (ch == '\r') {
                    m_ch_reg = ch;
                    break;
                }
                else if (ch == '\n' || ch == '\0')
                    break;
                else
                    s += static_cast<char>(ch);
            }
        }
    }

    return s;
}

void Contour::printDebug() const {
    int i = 0;

    for (const SegmentEntity* entity: m_entities) {
        if (entity)
            qDebug("%d: %s", i, entity->toString().c_str());
        else
            qDebug("%d: DxfEntity is NULL", i);

        i++;
    }
}

string Contour::toString() const {
    string s;
    int i = 0;

    for (const SegmentEntity* entity: m_entities) {
//        if (i == 13)
//            qDebug() << "Stop";

        if (entity)
            s += "\t" + to_string(i++) + " " + entity->toString() + "\n";
        else
            s += "\t" + to_string(i++) + " Error of toString(): DxfEntity is NULL\n";
    }

    return s;
}

size_t Contour::count() const { return m_entities.size(); }

// round shift to first position
void Contour::shiftFirst(list<SegmentEntity*>& entities, size_t index) {
    if (index != 0 && index < entities.size()) {
        std::list<SegmentEntity*> head;

        auto it = entities.begin();
        std::advance(it, index); // move iterator to index

        head.splice(head.begin(), entities, entities.begin(), it); // transfer to the head

        it = entities.end();
        entities.splice(it, head); // add the head to the tail
    }
}

void Contour::shiftFirst(size_t index) {
    shiftFirst(m_entities, index);
}

void Contour::shiftLast(size_t index) {
    if (index < m_entities.size() - 1) {
        std::list<SegmentEntity*> head;

        auto it = m_entities.begin();
        std::advance(it, index + 1); // move iterator to index

        head.splice(head.begin(), m_entities, m_entities.begin(), it); // the head transfer to  the tail

        it = m_entities.end();
        m_entities.splice(it, head); // add tail to back
    }
}

void Contour::moveFirst(size_t index) {
    if (index != 0 && index < m_entities.size()) {
        auto it = m_entities.begin();
        std::advance(it, index); // move iterator to index
        m_entities.push_front(*it);
        it = m_entities.erase(it);
    }
}

void Contour::moveLast(size_t index) {
    auto it = m_entities.begin();
    std::advance(it, index); // move iterator to index
    m_entities.push_back(*it);
    it = m_entities.erase(it);
}

void Contour::moveUp(size_t index) {
    if (index != 0 && index < m_entities.size()) {
        auto it = m_entities.begin();
        std::advance(it, index); // move iterator to index
        SegmentEntity* A = *it;
        --it;
        SegmentEntity* B = *it;
        *it = A;
        ++it;
        *it = B;
    }
}

void Contour::moveDown(size_t index) {
    if (index < m_entities.size() - 1) {
        auto it = m_entities.begin();
        std::advance(it, index); // move iterator to index
        SegmentEntity* A = *it;
        ++it;
        SegmentEntity* B = *it;
        *it = A;
        --it;
        *it = B;
    }
}

Contour Contour::cut_front() {
    Contour ctr;

    if (!m_entities.empty()) {
        SegmentEntity* const entity = m_entities.front();
        m_entities.pop_front();
        ctr.m_entities.push_back(entity);
    }

    return ctr;
}

Contour Contour::cut_back() {
    Contour ctr;

    if (!m_entities.empty()) {
        SegmentEntity* const entity = m_entities.back();
        m_entities.pop_back();
        ctr.m_entities.push_back(entity);
    }

    return ctr;
}

Contour Contour::cut_at(size_t index) {
    if (index == 0)
        return cut_front();
    else if (index == m_entities.size() - 1)
        return cut_back();
    else if (index < m_entities.size()) {
        moveFirst(index);
        return cut_front();
    }

    return Contour();
}

//void Dxf::offset(OFFSET_SIDE side, double offset) {
//    offset = abs(offset);

////    if (dir) {
////        offset_side = (offset_side == OFFSET_SIDE::LEFT_OFFSET) ? OFFSET_SIDE::RIGHT_OFFSET : OFFSET_SIDE::LEFT_OFFSET;
////        reverse();
////    }

//    if (m_entities.empty())
//        return;
//    else if (m_entities.size() == 1)
//        m_entities.front()->offset(side, offset, nullptr, nullptr);
//    else if (isLoop())
//        for (auto it = m_entities.begin(); it != m_entities.end(); ++it) {
//            if (it == m_entities.begin())
//                (*it)->offset(side, offset, m_entities.back(), *next(it));
//            else if (next(it) == m_entities.end())
//                (*it)->offset(side, offset, *prev(it), m_entities.front());
//            else
//                (*it)->offset(side, offset, *prev(it), *next(it));
//        }
//    else
//        for (auto it = m_entities.begin(); it != m_entities.end(); ++it) {
//            if (it == m_entities.begin())
//                (*it)->offset(side, offset, nullptr, *next(it));
//            else if (next(it) == m_entities.end())
//                (*it)->offset(side, offset, *prev(it), nullptr);
//            else
//                (*it)->offset(side, offset, *prev(it), *next(it));
//        }
//}

bool Contour::offset(OFFSET_SIDE side, double offset) {
    if (m_entities.empty() || offset <= 0)
        return true;

    bool loop = isLoop();

    for (SegmentEntity*& ent: m_entities) {
        if (!ent)
            continue;

        ent->offset(side, offset);
        if (ent->isPoint()) {
            delete ent;
            ent = new SegmentPoint;
        }
    }

    size_t i = 0;
    size_t n = m_entities.size();

    while (!intersect(loop, side, offset)) {
        // segment chandged type
        if (++i >= n) {
            m_last_error = "Offset error";
            return false;
        }
    }

    alignDxfPoints();

    return true;
}

// return false if segment changed
bool Contour::intersectLine(SegmentEntity*& A, SegmentEntity*& B) {
    static int i = 0;
    SegmentLine& LA = dynamic_cast<SegmentLine&>(*A);
    SegmentLine& LB = dynamic_cast<SegmentLine&>(*B);

//    if (i == 7)
//        qDebug("Debug");

    fpoint_t pt;
    SegmentIntersection::intersect(LA, LB, pt);

    {
        qDebug("%d Intersect Lines:", i++);
        qDebug("%s", LA.toString().c_str());
        qDebug("%s", LB.toString().c_str());
        qDebug("point: %s", pt.toString().c_str());
    }

    SegmentLine new_LA(LA.point_0(), pt);
    SegmentLine new_LB(pt, LB.point_1());

    if (!new_LA.otherDir180(LA) && !new_LB.otherDir180(LB)) {
        LA.setPoint1(pt);
        LB.setPoint0(pt);
    }
    else if (new_LA.otherDir180(LA) || new_LA.point_0() == pt) {
        SegmentPoint* P = new SegmentPoint(pt, LA.isAdditional());

        if (A)
            delete A;

        A = P;
        LB.setPoint0(pt);
        return false;
    }
    else {
        LA.setPoint1(pt);
        SegmentPoint* P = new SegmentPoint(pt, LB.isAdditional());

        if (B)
            delete B;

        B = P;
        return false;
    }

    return true;
}

// connect all segments
bool Contour::intersect(bool loop, OFFSET_SIDE side, double offset) {
    bool empty = false;

    for (auto it = m_entities.begin(); it != m_entities.end();) {
        auto next_it = nextLineOrArc(it, m_entities.begin(), m_entities.end(), empty, loop);

        if (empty || next_it == m_entities.end() || next_it == it) // next empty
            break;

        SegmentEntity*& A = *it;
        SegmentEntity*& B = *next_it;

        if (A && B && A->point_1() != B->point_0()) {
            if (A->type() == DXF_ENTITY_TYPE::LINE) {
                if (B->type() == DXF_ENTITY_TYPE::LINE) {
                    if (!intersectLine(A, B))
                        return false; // A or B changed type to Point
                }
                else if (B->type() == DXF_ENTITY_TYPE::ARC) {
                    SegmentLine& LA = dynamic_cast<SegmentLine&>(*A);
                    SegmentArc& AB = dynamic_cast<SegmentArc&>(*B);
                    fpoint_t pt[2];
                    double angle[2];

                    if (SegmentIntersection::intersect(LA, AB, pt, angle)) {
                        if (pt[0] == pt[1]) {
                            LA.setPoint1(pt[0]);
                            AB.setStartAngle(angle[0]);
                            AB.setEndAngle(angle[0]);
                        }
                        else {
                            if (SegmentEntity::almost_same_dir(LA.point_0(), LA.point_1(), pt[0])) {
                                if (fabs(angle[0] - AB.startAngle()) < fabs(angle[1] - AB.startAngle())) {
                                    LA.setPoint1(pt[0]);
                                    AB.setStartAngle(angle[0]);
                                }
                                else {
                                    LA.setPoint1(pt[1]);
                                    AB.setStartAngle(angle[1]);
                                }
                            }
                            else {
                                LA.setPoint1(pt[1]);
                                AB.setStartAngle(angle[1]);
                            }

//                            if (!loop && it == m_entities.begin()) { // choose point in same direction as LA.point_1()
//                                if (DxfEntity::almost_same_dir(LA.point_0(), LA.point_1(), pt[0])) {
//                                    LA.setPoint1(pt[0]);
//                                    AB.setStartAngle(angle[0]);
//                                }
//                                else {
//                                    LA.setPoint1(pt[1]);
//                                    AB.setStartAngle(angle[1]);
//                                }
//                            }
//                            else if (AB.insideCircle(LA.point_0())) {
//                                DxfPoint* P = new DxfPoint(LA.point_0(), LA.additional());
//                                if (A)
//                                    delete A;

//                                A = P;
//                                return false;
//                            }
//                            else {
//                                double len1 = DxfEntity::length(LA.point_0(), pt[0]);
//                                double len2 = DxfEntity::length(LA.point_0(), pt[1]);

//                                if (len1 < len2) {
//                                    LA.setPoint1(pt[0]);
//                                    AB.setStartAngle(angle[0]);
//                                }
//                                else {
//                                    LA.setPoint1(pt[1]);
//                                    AB.setStartAngle(angle[1]);
//                                }
//                            }
                        }
                    }
                    else { // change to line
                        AB.offset(~side, offset); // restore
                        SegmentLine* LB = new SegmentLine(AB.point_0(), AB.point_1());
                        LB->offset(side, offset);
                        if (B)
                            delete B;

                        B = LB;
                        return false;
                    }
                }
            }
            else if (A->type() == DXF_ENTITY_TYPE::ARC) {
                if (B->type() == DXF_ENTITY_TYPE::LINE) {                    
                    SegmentArc& AA = dynamic_cast<SegmentArc&>(*A);
                    SegmentLine& LB = dynamic_cast<SegmentLine&>(*B);
                    fpoint_t pt[2];
                    double angle[2];

                    if (SegmentIntersection::intersect(LB, AA, pt, angle)) {
                        if (pt[0] == pt[1]) {
                            AA.setStartAngle(angle[0]);
                            AA.setEndAngle(angle[0]);
                            LB.setPoint0(pt[0]);
                        }
                        else {
                            if (SegmentEntity::almost_same_dir(LB.point_1(), LB.point_0(), pt[0])) {
                                if (fabs(angle[0] - AA.endAngle()) < fabs(angle[1] - AA.endAngle())) {
                                    LB.setPoint0(pt[0]);
                                    AA.setEndAngle(angle[0]);
                                }
                                else {
                                    LB.setPoint0(pt[1]);
                                    AA.setEndAngle(angle[1]);
                                }
                            }
                            else {
                                LB.setPoint0(pt[1]);
                                AA.setEndAngle(angle[1]);
                            }

//                            auto next2_it = nextLineOrArc(next_it, m_entities.begin(), m_entities.end(), empty, loop);

//                            if (!loop && next2_it == m_entities.end()) { // is the last
//                                if (DxfEntity::almost_same_dir(LB.point_1(), LB.point_0(), pt[0])) {
//                                    LB.setPoint0(pt[0]);
//                                    AA.setEndAngle(angle[0]);
//                                }
//                                else {
//                                    LB.setPoint0(pt[1]);
//                                    AA.setEndAngle(angle[1]);
//                                }
//                            }
//                            else if (AA.insideCircle(LB.point_1())) {
//                                DxfPoint* P = new DxfPoint(LB.point_1(), LB.additional());
//                                if (B)
//                                    delete B;

//                                B = P;
//                                return false;
//                            }
//                            else {
//                                double len1 = DxfEntity::length(LB.point_1(), pt[0]);
//                                double len2 = DxfEntity::length(LB.point_1(), pt[1]);

//                                if (len1 < len2) {
//                                    LB.setPoint0(pt[0]);
//                                    AA.setEndAngle(angle[0]);
//                                }
//                                else {
//                                    LB.setPoint0(pt[1]);
//                                    AA.setEndAngle(angle[1]);
//                                }
//                            }
                        }
                    }
                    else { // change to line
                        AA.offset(~side, offset); // restore
                        SegmentLine* LA = new SegmentLine(AA.point_0(), AA.point_1());
                        LA->offset(side, offset);
                        if (A)
                            delete A;

                        A = LA;
                        return false;
                    }
                }
                else if (B->type() == DXF_ENTITY_TYPE::ARC) {
                    SegmentArc& AA = dynamic_cast<SegmentArc&>(*A);
                    SegmentArc& AB = dynamic_cast<SegmentArc&>(*B);
                    double angleA[2], angleB[2];

                    if (SegmentIntersection::intersect(AA, AB, angleA, angleB)) {
                        AA.setEndAngle(angleA[0]);
                        AB.setStartAngle(angleB[0]);
                    }
                    else {
                        AA.offset(~side, offset);
                        AB.offset(~side, offset);

                        SegmentLine* LA = new SegmentLine(AA.point_0(), AA.point_1(), true);
                        SegmentLine* LB = new SegmentLine(AB.point_0(), AB.point_1(), true);

                        LA->offset(side, offset);
                        LB->offset(side, offset);
                        AA.offset(side, offset);
                        AB.offset(side, offset);

                        LA->setPoint0(AA.point_1());
                        LB->setPoint1(AB.point_0());

                        ++it;
                        it = m_entities.insert(it, LA); // insert before
                        it = m_entities.insert(it, LB);

                        return false;
                    }
                }
            }
            else if (A->type() == DXF_ENTITY_TYPE::POINT) // first item is point
                dynamic_cast<SegmentPoint*>(A)->set(B->point_0());
        }

        it = next_it;
        if (loop && it == m_entities.begin()) // connected with begin, first begin OK
            break;
    }

    return true;
}

void Contour::alignDxfPoints() {
    if (m_entities.empty()) return;

    fpoint_t base = m_entities.front() ? m_entities.front()->point_1() : fpoint_t();

    for (SegmentEntity* ent: m_entities) {
        if (ent) {
            if (ent->type() == DXF_ENTITY_TYPE::POINT)
                dynamic_cast<SegmentPoint*>(ent)->set(base);
            else
                base = ent->point_1();
        }
    }
}


// return next not empty segment
list<SegmentEntity*>::iterator Contour::nextLineOrArc(
    const list<SegmentEntity*>::iterator& init,
    const list<SegmentEntity*>::iterator& begin,
    const list<SegmentEntity*>::iterator& end,
    bool& empty,
    bool loop
){
    if (begin == end) {
        empty = true;
        return init;
    }

    list<SegmentEntity*>::iterator it = init;
    ++it;

    if (loop) {
        for (; ; ++it) {
            if (it == end)
                it = begin;

            if (it == init)
                break;

            if (*it && ((*it)->type() == DXF_ENTITY_TYPE::LINE || (*it)->type() == DXF_ENTITY_TYPE::ARC))
                return it;
        }

        empty = true; // no more none empty segments
        return init;
    }

    // else
    for (; it != end; ++it) {
        if (*it && ((*it)->type() == DXF_ENTITY_TYPE::LINE || (*it)->type() == DXF_ENTITY_TYPE::ARC))
            return it;
    }

    empty = true;
    return end;
}

list<SegmentEntity*>::iterator Contour::prevLineOrArc(
    const list<SegmentEntity*>::iterator& init,
    const list<SegmentEntity*>::iterator& begin,
    const list<SegmentEntity*>::iterator& end,
    bool& empty,
    bool loop
){
    if (begin == end) {
        empty = true;
        return init;
    }

    list<SegmentEntity*>::iterator it = init;

    if (loop) {
        do {
            if (it == begin)
                it = end;

            --it;

            if (it == init)
                break;

           if (*it && ((*it)->type() == DXF_ENTITY_TYPE::LINE || (*it)->type() == DXF_ENTITY_TYPE::ARC))
                return it;
        } while(1);

        empty = true;
        return init;
    }

    if (it == begin) return begin;
    --it;

    while (it != begin) {
        if (*it && ((*it)->type() == DXF_ENTITY_TYPE::LINE || (*it)->type() == DXF_ENTITY_TYPE::ARC))
            return it;

        --it;
    }

    empty = true;
    return begin;
}

// return first entity after split in forward order
size_t Contour::split(double len) {
    double sum = 0;
    size_t i = 0;

    if (len >= M_PRECISION) {
        if (!m_entities.empty()) {
            for (list<SegmentEntity*>::iterator it = m_entities.begin(); it != m_entities.end(); ++it, i++) {
                sum += *it ? (*it)->length() : 0;

                if (sum >= len - M_PRECISION) {
                    SegmentEntity* rem = *it ? (*it)->trim_back(sum - len, true) : nullptr;
                    if (rem) {
                        m_entities.insert(++it, rem);
                        return ++i;
                    }
                    else
                        return i;
                }
            }
        }
    }
    return 0;
}

// return first entity after split in reverse order
size_t Contour::split_rev(double tab) {
    double sum = 0;
    size_t i = m_entities.size() - 1;

    if (tab >= M_PRECISION) {
        if (!m_entities.empty()) {
            for (list<SegmentEntity*>::reverse_iterator it = m_entities.rbegin(); it != m_entities.rend(); ++it, i--) {
                sum += *it ? (*it)->length() : 0;

                if (sum >= tab - M_PRECISION) {
                    SegmentEntity* rem = *it ? (*it)->trim_front(sum - tab, true) : nullptr;
                    if (rem) {
                        m_entities.insert(--it.base(), rem);
                        return ++i;
                    }
                    else
                        return i;
                }
            }
        }
    }
    return m_entities.size();
}

void Contour::remove_before(size_t index) {
    size_t i = 0;

    for (auto it = m_entities.begin(); it != m_entities.end(); ) {
        if (i >= index)
            return;

        if (!( (*it)->isAdditional() && (*it)->type() != DXF_ENTITY_TYPE::POINT ))
            i++;

        if (*it) {
            delete *it;
            *it = nullptr;
        }
        it = m_entities.erase(it);
    }
}

void Contour::remove_to(size_t index) {
    size_t i = 0;

    for (auto it = m_entities.begin(); it != m_entities.end(); ) {
        if (i > index)
            return;

        if (!( (*it)->isAdditional() && (*it)->type() != DXF_ENTITY_TYPE::POINT ))
            i++;

        if (*it) {
            delete *it;
            *it = nullptr;
        }        
        it = m_entities.erase(it);
    }
}

void Contour::remove_from(size_t index) {
    size_t i = 0;

    for (auto it = m_entities.begin(); it != m_entities.end(); ) {
        if (i >= index) {
            if (*it) {
                delete *it;
                *it = nullptr;
            }
            it = m_entities.erase(it);
        }
        else {
            if (!( (*it)->isAdditional() && (*it)->type() != DXF_ENTITY_TYPE::POINT ))
                i++;

            ++it;
        }
    }
}

void Contour::remove_after(size_t index) {
    size_t i = 0;

    for (auto it = m_entities.begin(); it != m_entities.end(); ) {
        if (i > index) {
            if (*it) {
                delete *it;
                *it = nullptr;
            }
            it = m_entities.erase(it);
        }
        else {
            if (!( (*it)->isAdditional() && (*it)->type() != DXF_ENTITY_TYPE::POINT ))
                i++;

            ++it;
        }
    }
}

void Contour::remove(size_t index) {
    size_t i = 0;

    if (index < m_entities.size()) {
        for (auto it = m_entities.begin(); it != m_entities.end(); ++it, i++) {
            if (i == index) {
                if (*it) {
                    delete *it;
                    *it = nullptr;
                }
                m_entities.erase(it);

                break;
            }
        }
    }
}

void Contour::changeFirstPoint(const fpoint_valid_t& first_pt) {
    if (first_pt.valid && !m_entities.empty()) {
        const fpoint_t& last_pt = m_entities.front()->point_1();
        SegmentLine* line = new SegmentLine(first_pt, last_pt);

        if (m_entities.front())
            delete m_entities.front();

        m_entities.front() = line;
    }
}

void Contour::changeLastPoint(const fpoint_valid_t& last_pt) {
    if (last_pt.valid && !m_entities.empty()) {
        const fpoint_t& first_pt = m_entities.back()->point_0();
        SegmentLine* line = new SegmentLine(first_pt, last_pt);

        if (m_entities.back())
            delete m_entities.back();

        m_entities.back() = line;
    }
}

void Contour::shift(const fpoint_t& pt) {
    for (SegmentEntity* entity: m_entities)
        if (entity)
            entity->shift(pt);
}

vector<fpoint_t> Contour::getPoints() const {
    vector<fpoint_t> res;
    bool valid = false;
    fpoint_t cur;

    if (!m_entities.empty()) {
        for (const SegmentEntity* ent: m_entities) {
            vector<fpoint_t> pts = ent ? ent->getPoints() : vector<fpoint_t>();

            for (const fpoint_t& pt: pts) {
                if (valid) {
                    if (pt != cur) {
                        res.push_back(pt);
                        cur = pt;
                    }
                }
                else {
                    res.push_back(pt);
                    cur = pt;
                }
            }
        }
    }

    return res;
}

double Contour::length() const {
    double sum = 0;
    for (const SegmentEntity* entity: m_entities) {
        sum += entity ? entity->length() : 0;
    }
    return sum;
}

double Contour::length(size_t index, const fpoint_t& pt) const {
    double sum = 0;
    auto it = m_entities.cbegin();

    for (size_t i = 0; i < index && it != m_entities.cend(); ++it, i++)
        sum += *it ? (*it)->length() : 0;

    if (it != m_entities.cend())
        sum += *it ? (*it)->length(pt) : 0;

    return sum;
}

Contour* Contour::copy_front(double len) const {
    if (len < M_PRECISION) return nullptr;

    double sum = 0;
    Contour* res = new Contour();

    if (!res)
        return nullptr;

    for (SegmentEntity* entity: m_entities) {
        if (entity) {
            sum += entity->length();
            res->push_back(*entity);
        }

        if (sum >= len - M_PRECISION)
            break;
    }

    if (res->empty()) {
        delete res;
        return res = nullptr;
    }

    if (res->back())
        res->back()->trim_back(sum - len);

    return res;
}

Contour* Contour::copy_front_rev(double tab) const {
    if (tab < M_PRECISION)
        return new Contour(*this);

    Contour* res = new Contour();

    if (!res)
        return nullptr;

    for (auto it = m_entities.rbegin(); it != m_entities.rend(); ++it) {
        double len = *it ? (*it)->length() : 0;

        if (tab > len)
            tab -= len;
        else if (*it)
            res->push_front(**it);
    }

    if (res->empty()) {
        delete res;
        return res = nullptr;
    }

    if (res->back())
        res->back()->trim_back(tab);

    return res;
}

Contour* Contour::copy_back(double len) const {
    if (len < M_PRECISION) return nullptr;

    double sum = 0;
    Contour* res = new Contour();

    for (auto it = m_entities.rbegin(); it != m_entities.rend(); ++it) {
        double ent_len = *it ? (*it)->length() : 0;

        if (*it)
            res->push_front(**it);

        if (sum + ent_len >= len - M_PRECISION)
            break;

        sum += ent_len;
    }

    if (res->empty()) {
        delete res;
        return res = nullptr;
    }

    if (res->front())
        res->front()->trim_front_rev(len - sum);

    return res;
}

ContourRange Contour::contourRange(const list<SegmentEntity*>& contour) {
    ContourRange range = ContourRange();

    range.valid = !contour.empty();

    for (const SegmentEntity* ent: contour)
        if (ent)
            range.scale(ent->range());

//    double x, y;

//    for (const auto& pt: contour) {
//        x = pt->point_0().x;
//        y = pt->point_0().y;

//        if (x < range.x_min) range.x_min = x;
//        else if (x > range.x_max) range.x_max = x;

//        if (y < range.y_min) range.y_min = y;
//        else if (y > range.y_max) range.y_max = y;

//        x = pt->point_1().x;
//        y = pt->point_1().y;

//        if (x < range.x_min) range.x_min = x;
//        else if (x > range.x_max) range.x_max = x;

//        if (y < range.y_min) range.y_min = y;
//        else if (y > range.y_max) range.y_max = y;
//    }

    return range;
}

ContourRange Contour::contourRange() const { return Contour::contourRange(m_entities); }

size_t Contour::joinedCount() const {
    size_t i = 0;

    for (auto it = m_entities.begin(); it != m_entities.end() && next(it) != m_entities.end(); ++it, i++) {
        if (*it && *next(it) && (*it)->point_1() != (*next(it))->point_0())
            break;
    }

    return i + 1;
}

bool Contour::whole() const {
    for (auto it = m_entities.begin(); it != m_entities.end() && next(it) != m_entities.end(); ++it) {
        if (*it && *next(it) && (*it)->point_1() != (*next(it))->point_0())
            return false;
    }

    return true;
}

void Contour::rotate(const RotateMatrix &mx) {
    for (SegmentEntity*& ent: m_entities)
        if (ent)
            ent->rotate(mx);
}

void Contour::flipX(double x) {
    for (SegmentEntity*& ent: m_entities)
        if (ent)
            ent->flipX(x);
}

void Contour::flipY(double y) {
    for (SegmentEntity*& ent: m_entities)
        if (ent)
            ent->flipY(y);
}

void Contour::scale(double k) {
    for (SegmentEntity*& ent: m_entities)
        if (ent)
            ent->scale(k);
}

bool Contour::setOut(const fpoint_valid_t& pt) {
    size_t i = 0;

    m_outIndex = data_valid_t<size_t>();

    if (isLoop()) {
        if (pt.valid) {
            for (auto it = m_entities.begin(); it != m_entities.end(); ++it, i++) {
                if ((*it)->point_1() == pt) {
                    m_outIndex.data = i;
                    m_outIndex.valid = true;
                    break;
                }
            }
        }
    }
    else {
        m_outIndex.data = m_entities.size() - 1;
        m_outIndex.valid = true;
    }

    return m_outIndex.valid;
}

bool Contour::setOut(int seg_num) {
    if (seg_num < 0)
        seg_num += m_entities.size();

    if (seg_num >= 0 && (size_t)seg_num < m_entities.size()) {
        m_outIndex.valid = true;
        m_outIndex.data = (size_t)seg_num;
        return true;
    }

    return false;
}

// Use it after sorting the ContourList
void Contour::setOut(const Contour* const next) {
    setOut(next ? next->first() : fpoint_valid_t());
}

void Contour::setInOut(size_t in_seg, size_t out_seg) {
    shiftFirst(in_seg);
    int delta = out_seg - in_seg;
    setOut(delta - 1);
}

Contour Contour::createMux(size_t in_seg, size_t out_seg) const {
    Contour res;

    for (list<SegmentEntity*>::const_iterator it = m_entities.cbegin(); it != m_entities.cend(); ++it) {
        if (*it)
            res.m_entities.push_back( (*it)->clone() );
    }

    res.m_sorted = this->m_sorted;

    res.setInOut(in_seg, out_seg);

    return res;
}

// East North West South indeces
std::vector<size_t> Contour::getVerticesPoints() const {
    std::vector<size_t> res(4, 0);
    fpoint_t e, n, w, s;

    if (m_entities.empty())
        return std::vector<size_t>();

    e.x = std::numeric_limits<double>::min();
    w.x = std::numeric_limits<double>::max();

    n.y = std::numeric_limits<double>::min();
    s.y = std::numeric_limits<double>::max();

    size_t i = 0;
    for (auto it = m_entities.begin(); it != m_entities.end(); ++it, i++) {
        fpoint_t pt = (*it)->point_0();

        if (pt.x > e.x) {
            e = pt;
            res[0] = i;
        }

        if (pt.y > n.y) {
            n = pt;
            res[1] = i;
        }

        if (pt.x < w.x) {
            w = pt;
            res[2] = i;
        }

        if (pt.y < s.y) {
            s = pt;
            res[3] = i;
        }
    }

    return res;
}

vector<fpoint_t> Contour::vertices() const {
    vector<fpoint_t> res;

    for (const auto& ent: m_entities) {
        res.push_back(ent->point_0());
        res.push_back(ent->point_1());
    }

    return res;
}

// Slicing
void Contour::addRectSpeedProfile(AXIS axis, double width, double speed, bool fwd, fpoint_t &A, fpoint_t &B) {
    if (axis == AXIS::AXIS_Y)
        B = fpoint_t(fwd ? A.x + width : A.x - width, A.y);
    else
        B = fpoint_t(A.x, fwd ? A.y + width : A.y - width);

    SegmentLine line = SegmentLine(A, B);
    line.setSpeed(speed);

    push_back(line);
    A = B;
}

// return initial velocity
// result < 0 - error
double Contour::addCircleSpeedProfile(const snake_t& par, bool fwd, fpoint_t& A, fpoint_t& B) {
    using namespace my_lib;
    vector<pair<double, double>> len_vel = splitCircle(par.width / 2, par.sections_num, par.speed_avg);

    double vel0 = len_vel.size() > 0 ? len_vel[0].second : -1;

    for (const pair<double, double>& o: len_vel)
        addRectSpeedProfile(par.axis, o.first, o.second, fwd, A, B);

    return vel0;
}

double Contour::addSlice(const snake_t& par, bool fwd, bool last, fpoint_t& A, fpoint_t& B) {
    double vel0 {0};

    double delta = fwd ? par.spacing : -par.spacing;

    if (par.axis == AXIS::AXIS_Y)
        B = fpoint_t(A.x + delta, A.y);
    else
        B = fpoint_t(A.x, A.y + delta);

    SegmentLine line = SegmentLine(A, B);
    line.setSpeed(par.speed_avg);
    line.setRollVel(par.roll_vel);
    push_back(line);

    A = B;

    if (par.profile == SLICING_PROFILE::ROUND)
        vel0 = addCircleSpeedProfile(par, fwd, A, B);
    else {
        addRectSpeedProfile(par.axis, par.width, par.speed_avg, fwd, A, B);
        vel0 = par.speed_avg;
    }

    if (par.axis == AXIS::AXIS_Y)
        B = fpoint_t(A.x + delta, A.y);
    else
        B = fpoint_t(A.x, A.y + delta);

    line = SegmentLine(A, B);
    if (par.pause_ena && par.pause != 0)
        line.setPause(par.pause);

    push_back(line);
    A = B;

    if (!last) {
        double step = par.step > 0 ? par.step + par.wireD : par.step - par.wireD;

        if (par.axis == AXIS::AXIS_Y)
            B = fpoint_t(A.x, A.y + step);
        else
            B = fpoint_t(A.x + step, A.y);

        SegmentLine line = SegmentLine(A, B);
        line.setSpeed(par.speed_idle);
        line.setRollVel(par.roll_vel_idle);
        push_back(line);

        A = B;
    }

    return vel0;
}

void Contour::generate(const snake_t& par) {
    bool fwd = true;
    fpoint_t A {0,0}, B {0,0};
    clear();

    if (par.slices_num <= 0)
        return;

    double vel0 = addSlice(par, fwd, false, A, B);

    if (front())
        front()->setSpeed(vel0);

    fwd = !fwd;

    for (unsigned i = 0; i < par.slices_num; i++) {
        addSlice(par, fwd, i == par.slices_num - 1, A, B);
        fwd = !fwd;
    }
}

void Contour::generate(const comb_t& par) {
    fpoint_t A {0,0}, B {0,0};
    clear();

    double wireD = par.step < 0 ? -par.wireD : par.wireD;

    for (unsigned i = 0; i < par.slots_num; i++) {
        if (par.axis == AXIS::AXIS_Y) {
            B = fpoint_t(A.x + par.depth, A.y);
            SegmentLine line = SegmentLine(A, B);
            line.setSpeed(par.speed);
            line.setRollVel(par.roll_vel);

            if (par.pause_ena && par.pause != 0)
                line.setPause(par.pause);

            push_back(line);

            line = SegmentLine(B, A);
            line.setSpeed(par.speed_idle);
            line.setRollVel(par.roll_vel_idle);
            push_back(line);

            if (i != par.slots_num - 1) {
                B = fpoint_t(A.x, A.y + par.step + wireD);
                line = SegmentLine(A, B);
                line.setSpeed(par.speed_idle);
                line.setRollVel(par.roll_vel_idle);
                push_back(line);

                A = B;
            }
        }
        else {
            B = fpoint_t(A.x, A.y + par.depth);
            SegmentLine line = SegmentLine(A, B);
            line.setSpeed(par.speed);
            line.setRollVel(par.roll_vel);

            if (par.pause_ena && par.pause != 0)
                line.setPause(par.pause);

            push_back(line);

            line = SegmentLine(B, A);
            line.setSpeed(par.speed_idle);
            line.setRollVel(par.roll_vel_idle);
            push_back(line);

            if (i != par.slots_num - 1) {
                B = fpoint_t(A.x + par.step + wireD, A.y);
                line = SegmentLine(A, B);
                line.setSpeed(par.speed_idle);
                line.setRollVel(par.roll_vel_idle);
                push_back(line);

                A = B;
            }
        }
    }
}

const Contour& Contour::buildLine(double len, AXIS axis, DIR dir) {
    this->clear();

    SegmentLine line(len, axis, dir);
    push_back(line);

    return *this;
}
