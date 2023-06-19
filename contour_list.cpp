#include "contour_list.h"

using namespace std;

ContourList::ContourList() :
    m_contours(deque<ContourPair>()),
    m_thickness(0), m_cur_ctr(-1), m_cur_seg(-1), m_xyValid(false), m_uvValid(false), m_botValid(false), m_botLength(-1),
    m_ctr(0), m_sel(CONTOUR_SELECT::NONE),
    m_error("")
{
}
ContourList::ContourList(const ContourList& other) {
    *this = other;
}
ContourList::ContourList(const deque<ContourPair>& contours) :
    m_contours(contours),
    m_thickness(0), m_cur_ctr(-1), m_cur_seg(-1), m_xyValid(false), m_uvValid(false), m_botValid(false), m_botLength(-1),
    m_ctr(0), m_sel(CONTOUR_SELECT::NONE),
    m_error("")
{
}
ContourList::ContourList(ContourList&& other) {
    *this = move(other);
}
ContourList::ContourList(std::deque<ContourPair>&& contours) :
    m_contours(move(contours)),
    m_thickness(0), m_cur_ctr(-1), m_cur_seg(-1), m_xyValid(false), m_uvValid(false), m_botValid(false), m_botLength(-1),
    m_ctr(0), m_sel(CONTOUR_SELECT::NONE),
    m_error("")
{
}

ContourList::~ContourList() { clear(); }

void ContourList::clear() {
    for (ContourPair& pair: m_contours) {
        pair.clear();
    }

    clear_properties();
}

void ContourList::leaveOne(size_t ctr_num) {
    for (size_t i = 0; i < m_contours.size(); i++) {
        if (i != ctr_num)
            m_contours[i].clear();
    }

    clear_properties();
}

void ContourList::clear_properties() {
    m_contours.clear();
    m_thickness = 0;

    clearSelected();
    clearPos();
    m_botLength = -1;
    m_error.clear();
}

ContourList& ContourList::operator=(const ContourList& other) {
    if (this != &other) {
        clear();
        m_thickness = other.m_thickness;

        m_cur_ctr = other.m_cur_ctr;
        m_cur_seg = other.m_cur_seg;

        m_xyPos = other.m_xyPos;
        m_xyValid = other.m_xyValid;

        m_uvPos = other.m_uvPos;
        m_uvValid = other.m_uvValid;

        m_botPos = other.m_botPos;
        m_botValid = other.m_botValid;

        for (const ContourPair& pair: other.m_contours)
            m_contours.push_back(pair);

        m_botLength = -1;

        clearSelected();
    }
    return *this;
}
ContourList &ContourList::operator=(ContourList&& other) noexcept {
    if (this != &other) {
        clear();
        m_contours = move(other.m_contours);
        m_thickness = other.m_thickness;

        m_cur_ctr = other.m_cur_ctr;
        m_cur_seg = other.m_cur_seg;

        m_xyPos = other.m_xyPos;
        m_xyValid = other.m_xyValid;

        m_uvPos = other.m_uvPos;
        m_uvValid = other.m_uvValid;

        m_botPos = other.m_botPos;
        m_botValid = other.m_botValid;

        m_botLength = -1;

        m_ctr = other.m_ctr;
        m_seg_map = move(other.m_seg_map);
        m_sel = other.m_sel;

        m_error = move(other.m_error);
    }
    return *this;
}

void ContourList::replace(deque<ContourPair>& contours) {
    if (&m_contours != &contours) {
        clear();
        m_contours = contours;
        contours.clear();
    }
}

//void ContourList::setCut(const cut_t& cut) {
//    for (ContourPair& pair: m_contours)
//        pair.setCut(cut);
//}

void ContourList::setCutParam(const cut_t& cut, const deque<GeneratorMode>& modes) {
    for (ContourPair& pair: m_contours) {
        pair.setCut(cut);
        pair.setGeneratorModes(modes);

        if (pair.type() == CONTOUR_TYPE::CUTLINE_CONTOUR) {
            pair.choiceMode(pair.cut().cutline_mode_id);
        }
    }
}

//void ContourList::setGeneratorModes(const std::deque<GeneratorMode>& modes) {
//    for (ContourPair& pair: m_contours)
//        pair.setGeneratorModes(modes);
//}

void ContourList::push_front(const ContourPair &pair) {
    m_contours.push_front(pair);
    m_botLength = -1;
}

void ContourList::push_back(const ContourPair &pair) {
    m_contours.push_back(pair);
//    m_contours.push_back(ContourPair());
//    m_contours.back() = pair;
    m_botLength = -1;
}

void ContourList::set(size_t index, const ContourPair &pair) {
    if (index < m_contours.size()) {
        m_contours[index] = pair;
        m_botLength = -1;
    }
}

void ContourList::insert_before(size_t index, const ContourPair& pair) {
    size_t i = 0;

    for (auto it = m_contours.begin(); it != m_contours.end(); ++it, i++) {
        if (i == index) {
            m_contours.insert(it, pair);
            break;
        }
    }

    m_botLength = -1;
}

void ContourList::insert_after(size_t index, const ContourPair& pair) {
    if (index == m_contours.size() - 1)
        m_contours.push_back(pair);
    else {
        size_t i = 0;

        for (auto it = m_contours.begin(); it != m_contours.end(); ++it, i++) {
            if (i == index + 1) {
                m_contours.insert(it, pair);
                break;
            }
        }
    }
    m_botLength = -1;
}

void ContourList::new_insert(size_t index, CONTOUR_TYPE type) {
    insert_before(index, ContourPair(type));
}
void ContourList::new_front(CONTOUR_TYPE type) {                m_contours.push_front(ContourPair(type)); }
void ContourList::new_back(CONTOUR_TYPE type) {                 m_contours.push_back(ContourPair(type)); }

bool ContourList::remove(size_t index) {
    size_t i = 0;
    clearSelected();
    m_botLength = -1;

    for (auto it = m_contours.begin(); it != m_contours.end(); ++it, i++) {
        if (i == index) {
            it->clear();
            m_contours.erase(it);
            return true;
        }
    }
    return false;
}

//ContourPair* ContourList::at(int i) {
//    if (i < 0)
//        return nullptr;
//    else if (static_cast<size_t>(i) >= m_contours.size())
//        return nullptr;

//    m_botLength = -1;

//    return &m_contours[i];
//}

//const ContourPair* ContourList::at(int i) const {
//    return i >= 0 ? at(static_cast<size_t>(i)) : nullptr;
//}

// round shift to first position
void ContourList::shiftFirst(size_t index) {
    size_t size = m_contours.size();

    if (size > 1 && index != 0 && index < size) {
        deque<ContourPair> new_ctrs(size);

        for (size_t i = index, j = 0; i < size; i++, j++)
            new_ctrs[j].swap( m_contours[i] );

        for (size_t i = 0, j = size - index; j < size; i++, j++)
            new_ctrs[j].swap( m_contours[i] );

        m_contours.swap( new_ctrs );
    }
}

void ContourList::shiftLast(size_t index) {
    size_t size = m_contours.size();

    if (size > 1 && index < size - 1) {
        deque<ContourPair> new_ctrs(size);

        for (size_t i = 0, j = size - index - 1; j < size; i++, j++)
            new_ctrs[j].swap( m_contours[i] );

        for (size_t i = index + 1, j = 0; i < size; i++, j++)
            new_ctrs[j].swap( m_contours[i] );

        m_contours.swap( new_ctrs );
    }
}

void ContourList::moveUp(size_t i) {
    size_t size = m_contours.size();

    if (size > 1 && i != 0 && i < size)
        m_contours[i - 1].swap( m_contours[i] );
}

void ContourList::moveDown(size_t i) {
    size_t size = m_contours.size();

    if (size > 1 && i < size - 1)
        m_contours[i + 1].swap( m_contours[i] );
}

void ContourList::merge() {
    if (m_contours.size() <= 1)
        return;

    for (auto it = m_contours.begin() + 1; it != m_contours.end(); ) {
        m_contours.front().move_back(*it);
        it = m_contours.erase(it);
    }

    m_contours.front().setType(CONTOUR_TYPE::MAIN_CONTOUR);
}

ContourPair* ContourList::at(size_t i) {
    m_botLength = -1;
    return i < m_contours.size() ? &m_contours[i] : nullptr;
}

const ContourPair* ContourList::at(size_t i) const {
    return i < m_contours.size() ? &m_contours[i] : nullptr;
}

const SegmentEntity* ContourList::at(size_t ctr, size_t row, size_t col) {
    if (ctr < m_contours.size()) {
        const ContourPair& pair = m_contours[ctr];

        switch (col) {
            case 0: return pair.bot()->at(row);
            case 1: return pair.top()->at(row);
        }
    }

    return nullptr;
}

ContourPair* ContourList::front() {
    m_botLength = -1;
    return m_contours.empty() ? nullptr : &m_contours.front();
}
ContourPair* ContourList::back() {
    m_botLength = -1;
    return m_contours.empty() ? nullptr : &m_contours.back();
}

bool ContourList::empty() const { return m_contours.empty(); }

size_t ContourList::size() const { return m_contours.size(); }

const std::string &ContourList::lastError() { return m_error; }

ContourRange ContourList::range() const {
    ContourRange range, bot_range, top_range;

    for (const ContourPair& pair: m_contours) {
        bot_range = pair.bot() ? Contour::contourRange(pair.bot()->entities()) : ContourRange();
        top_range = pair.top() ? Contour::contourRange(pair.top()->entities()) : ContourRange();

        range.scale(bot_range);
        range.scale(top_range);
    }

    double w = range.width();
    double h  = range.height();

    if (w == 0)
        range.expandWidth(0.2 * h);
    else if (h == 0)
        range.expandHeight(0.2 * w);
    else if (w / h < 0.2)
        range.expandWidth(0.2 * h);
    else if (h / w < 0.2)
        range.expandHeight(0.2 * w);

    return range;
}

void ContourList::scale(double k, const fpoint_t &base) {
    for (ContourPair& pair: m_contours)
        pair.shift(-base);

    for (ContourPair& pair: m_contours) {
        pair.scale(k);
    }

    for (ContourPair& pair: m_contours)
        pair.shift(base);
}

void ContourList::select(size_t ctr_num) {
    clearSelected();

    if (ctr_num < m_contours.size()) {
        m_ctr = ctr_num;
        m_sel = CONTOUR_SELECT::CONTOUR;
    }
}

//void ContourList::select(size_t ctr_num, size_t row_num, size_t col_num) {
//    clearSelected();

//    if (ctr_num < m_contours.size()) {
//        const ContourPair& pair = m_contours[ctr_num];

//        switch (col_num) {
//            case 0:
//                if (row_num < pair.countBot()) {
//                    m_ctr = ctr_num;
//                    m_row = row_num;
//                    m_col = col_num;
//                    m_sel = CONTOUR_SELECT::SEGMENT;
//                }
//                break;
//            case 1:
//                if (row_num < pair.countTop()) {
//                    m_ctr = ctr_num;
//                    m_row = row_num;
//                    m_col = col_num;
//                    m_sel = CONTOUR_SELECT::SEGMENT;
//                }
//                break;
//        }
//    }
//}

void ContourList::select(size_t ctr_num, const map<size_t, uint8_t>& seg_map) {
    clearSelected();

    if (ctr_num < m_contours.size() && !seg_map.empty()) {
        const ContourPair& pair = m_contours[ctr_num];

        m_ctr = ctr_num;
        m_seg_map = seg_map;

        // check map
        size_t bot_size = pair.countBot();
        size_t top_size = pair.countTop();

        for (auto it = m_seg_map.begin(); it != m_seg_map.end(); ) {
            if ((it->second & 1) && it->first >= bot_size) // bot
                it = m_seg_map.erase(it);
            else if ((it->second & 2) && it->first >= top_size) // top
                it = m_seg_map.erase(it);
            else if (it->second == 0)
                it = m_seg_map.erase(it);
            else
                ++it;
        }

        m_sel = m_seg_map.empty() ? CONTOUR_SELECT::NONE : CONTOUR_SELECT::SEGMENT;
    }
}

void ContourList::select(const std::pair<size_t, size_t>& ctr_ent) {
    clearSelected();

    size_t ctr_num = ctr_ent.first;
    size_t seg_num = ctr_ent.second;

    if (ctr_num < m_contours.size() && seg_num < m_contours[ctr_num].count()) {
        m_ctr = ctr_num;
        m_seg_map[seg_num] = 3;
        m_sel = CONTOUR_SELECT::SEGMENT;

        setCurrent(ctr_num, seg_num);
    }
}

void ContourList::clearSelected() {    
    clearPos();

    m_ctr = 0;
    m_seg_map.clear();
    m_sel = CONTOUR_SELECT::NONE;

    m_cur_ctr = m_cur_seg = -1;
}

bool ContourList::isContourSelected() const { return m_sel == CONTOUR_SELECT::CONTOUR; }
bool ContourList::isSegmentSelected() const { return m_sel == CONTOUR_SELECT::SEGMENT; }

size_t ContourList::selectedContour() const { return m_ctr; }
//size_t ContourList::selectedRow() const     { return m_row; }
//size_t ContourList::selectedColumn() const  { return m_col; }

bool ContourList::isSelected(size_t ctr, size_t row, size_t col) const {
    if (!m_seg_map.empty() && m_sel == CONTOUR_SELECT::SEGMENT && ctr == m_ctr && (col == 0 || col == 1)) {
        const ContourPair* pair = at(ctr);

        if (pair && (row < pair->countBot() || row < pair->countTop())) {
            auto it = m_seg_map.find(row);
            return it != m_seg_map.end() && ( (col == 0 && (it->second & 1)) || (col == 1 && (it->second & 2)) );
        }
    }

    return false;
}

bool ContourList::sort() {
    bool OK = true;
    fpoint_valid_t prev_pt, next_pt;

    clearSelected();

    // sort contours
    for (int i = 0; i < (int)m_contours.size(); i++) {
        Contour* const bot = m_contours[i].bot();
        Contour* const top = m_contours[i].top();

        ContourPair* const prev_pair = i > 0 ? &m_contours[i - 1] : nullptr;

        if (bot && !bot->empty()) {
            Contour* const prev = prev_pair ? prev_pair->bot() : nullptr;
            bot->sort(prev, i == 1);
        }

        if (top && !top->empty()) {
            Contour* const prev = prev_pair ? prev_pair->top() : nullptr;
            top->sort(prev, i == 1);
        }
    }

    // Set outputs
    for (int i = 0; i < (int)m_contours.size(); i++) {
        const ContourPair* const next = i < (int)m_contours.size() - 1 ? &m_contours[i + 1] : nullptr;

        const Contour* const next_bot = next ? next->bot() : nullptr;
        const Contour* const next_top = next ? next->top() : nullptr;

        Contour* const bot = m_contours[i].bot();
        Contour* const top = m_contours[i].top();

        if (bot && !bot->empty()) {
            if (bot->isSorted()) {
                if (next_bot)
                    bot->setOut(next_bot->first_point());
                else
                    bot->setOut(bot->last_point());
            }
            else
                bot->setOut();
        }

        if (top && !top->empty()) {
            if (top->isSorted()) {
                if (next_top)
                    top->setOut(next_top->first_point());
                else
                    top->setOut(top->last_point());
            }
            else
                top->setOut();
        }
    }

    return OK;
}

bool ContourList::isSorted() const {
    bool OK = true;

    for (const auto& pair: m_contours)
        OK &= pair.isSorted();

    return OK;
}

void ContourList::setXYPos(const fpoint_t &pt) {
    m_xyPos = pt;
    m_xyValid = true;
}
void ContourList::setUVPos(const fpoint_t &pt) {
    m_uvPos = pt;
    m_uvValid = true;
}
void ContourList::setBotPos(const fpoint_t &pt) {
    m_botPos = pt;
    m_botValid = true;
}

void ContourList::clearPos() { m_xyValid = m_uvValid = m_botValid = false; }

bool ContourList::hasXYPos() const { return m_xyValid; }
bool ContourList::hasUVPos() const { return m_uvValid; }
bool ContourList::hasBotPos()  const { return m_botValid; }

const fpoint_t *ContourList::xyPos() const { return m_xyValid ? &m_xyPos : nullptr; }
const fpoint_t *ContourList::uvPos() const { return m_uvValid ? &m_uvPos : nullptr; }
const fpoint_t *ContourList::botPos()  const { return m_botValid  ? &m_botPos : nullptr; }

string ContourList::toString() const {
    std::string s;

    s += "Size " + to_string(m_contours.size()) + "\n";
    int i = 0;
    for (const ContourPair& pair: m_contours) {
        s += to_string(i++) + " " + pair.toStringShort() + "\n";
    }

    return s;
}

void ContourList::shift(deque<ContourPair>& contours, const fpoint_t& pt) {
    for (ContourPair& pair: contours)
        pair.shift(pt);
}

void ContourList::shift(const fpoint_t& pt) {
    shift(m_contours, pt);
}

void ContourList::moveTo(const fpoint_t &pt) {
    fpoint_valid_t cur = firstBot();

    if (cur.valid)
        shift(pt - cur);
}

fpoint_valid_t ContourList::firstBot() const { return m_contours.empty() ? fpoint_valid_t(false) : m_contours.front().firstBot(); }
fpoint_valid_t ContourList::firstTop() const { return m_contours.empty() ? fpoint_valid_t(false) : m_contours.front().firstTop(); }

bool ContourList::hasAnyTop() const {
    for (const ContourPair& pair: m_contours) {
        if (!pair.topEmpty())
            return true;
    }
    return false;
}

bool ContourList::verify() {
    int i = 0;

    for (auto it = m_contours.cbegin(); ; i++) {
        if (it->botEmpty()) {
            m_error = "Bottom contour " + to_string(i + 1) + " is empty";
            return false;
        }

        auto nx = next(it);

        if (nx != m_contours.cend()) {
            if (it->bot()->front()->point_1() != nx->bot()->front()->point_0()) {
                m_error = "Last point of bottom contour " + to_string(i + 1) + " is not connected to first point of bottom contour " + to_string(i + 2);
                return false;
            }
        }
        else
            break;

        it = nx;
    }

    if (hasAnyTop()) {
        for (auto it = m_contours.cbegin(); ; i++) {
            if (it->topEmpty()) {
                m_error = "Top contour " + to_string(i + 1) + " is empty";
                return false;
            }
            else if (it->countTop() != it->countBot()) {
                m_error = "In contour, " + to_string(i+1) +
                        " bottom entities number " + to_string(it->countBot()) +
                        " is not equal top entities number " + to_string(it->countTop());
                return false;
            }

            auto nx = next(it);

            if (nx != m_contours.cend()) {
                if (it->bot()->front()->point_1() != nx->bot()->front()->point_0()) {
                    m_error = "Last point of top contour " + to_string(i) + " is not connected to first point of top contour " + to_string(i + 1);
                    return false;
                }
            }
            else
                break;

            it = nx;
        }
    }

    return true;
}

void ContourList::setCurrent(int ctr_num, int seg_num) {
    if (ctr_num >= 0 && size_t(ctr_num) < m_contours.size() && seg_num >= 0 && size_t(seg_num) < m_contours.at(size_t(ctr_num)).count()) {
        m_cur_ctr = ctr_num;
        m_cur_seg = seg_num;
        return;
    }

    m_cur_ctr = -1;
    m_cur_seg = -1;
}

// todo: calculate it ones
double ContourList::botLengthFull() {
    if (m_botLength <= 0) {
        m_botLength = 0;

        for (const ContourPair& pair: m_contours)
            m_botLength += pair.lengthBot();
    }

    return m_botLength;
}

double ContourList::botLength() const { // todo: check a formula
    double sum = 0;

    if (m_botValid && m_cur_ctr >= 0 && size_t(m_cur_ctr) < m_contours.size() && m_cur_seg >= 0 && size_t(m_cur_seg) < m_contours[size_t(m_cur_ctr)].count()) {
        if (m_cur_ctr > 0) {
            for (size_t i = 0; i < size_t(m_cur_ctr) - 1; i++)
                sum += m_contours[i].lengthBot();
        }

        sum += m_contours[size_t(m_cur_ctr)].lengthBot(size_t(m_cur_seg), m_botPos);
    }

    return sum;
}

double ContourList::pct() {
    return botLength() / botLengthFull() * 100;
}

void ContourList::rotate(double phi) {
    RotateMatrix mx(phi);

    for (ContourPair& pair: m_contours)
        pair.rotate(mx);
}

void ContourList::flipX() {
    ContourPair* pair = front();
    if (!pair || !pair->bot()) return;

    double x = pair->bot()->first().x;

    for (ContourPair& pair: m_contours)
        pair.flipX(x);
}

void ContourList::flipY() {
    ContourPair* pair = front();
    if (!pair || !pair->bot()) return;

    double y = pair->bot()->first().y;

    for (ContourPair& pair: m_contours)
        pair.flipY(y);
}

fpoint_t ContourList::intersectUV() const {
    if (!isSegmentSelected())
        return fpoint_t();

    if (m_xyPos == m_uvPos)
        return m_xyPos;

    // find selected
    const ContourPair* const pair = at(m_ctr);

    if (!pair || m_cur_seg < 0)
        return fpoint_t();

    const SegmentEntity* const selectedBot = pair->bot()->at(m_cur_seg);

    switch (selectedBot->type()) {
        case DXF_ENTITY_TYPE::LINE: {
            SegmentLine LZ(m_xyPos, m_uvPos);
            const SegmentLine* LXY = dynamic_cast<const SegmentLine*>(selectedBot);
            fpoint_t res;

            SegmentIntersection::intersect(LZ, *LXY, res);

            return res;
        }
        case DXF_ENTITY_TYPE::ARC: {
            SegmentLine line(m_xyPos, m_uvPos);
            const SegmentArc* arc = dynamic_cast<const SegmentArc*>(selectedBot);
            fpoint_t pts[2];
            double angle[2];

            SegmentIntersection::intersect(line, *arc, pts, angle);

            if (arc->between(angle[0]))
                return pts[0];

            return pts[1];
        }
        default:
            break;
    }

    return selectedBot->point_0();
}

bool ContourList::generateMux(
    const ContourPair* const _pair,
    size_t items_num, size_t lanes_num,
    AXIS axis, DIR items_dir, DIR lanes_dir,
    const std::vector<size_t>& split_pts,
    double items_gap, double lanes_gap,
    double incut_len, double outcut_len
){
    if (!_pair || items_num < 2 || lanes_num == 0 || split_pts.size() != 4)
        return false;

    const ContourPair pair = *_pair; // copy
    clear(); // all

//    uint n = items_num * lanes_num;
//    m_contours.resize(2 * n + 1);

    const bool top_ena = !pair.topEmpty();

    const ContourPair cutline_fwd = ContourPair::createCutline(items_gap, axis, items_dir, top_ena);
    const ContourPair cutline_rev = ContourPair::createCutline(items_gap, axis, -items_dir, top_ena);
    const ContourPair cutline_last = ContourPair::createCutline(lanes_gap, swap(axis), lanes_dir, top_ena);

    const ContourPair clone_fwd = pair.getMux(split_pts[0], split_pts[1]);
    const ContourPair clone_rev = pair.getMux(split_pts[1], split_pts[0]);

    const ContourPair clone_fwd_last = pair.getMux(split_pts[0], split_pts[3]);
    const ContourPair clone_rev_last = pair.getMux(split_pts[1], split_pts[3]);

    const ContourPair clone_rev_first = pair.getMux(split_pts[2], split_pts[0]);
    const ContourPair clone_fwd_first = pair.getMux(split_pts[2], split_pts[1]);

    const ContourPair incut = ContourPair::createCutline(incut_len, axis, items_dir, top_ena);
    const ContourPair outcut = ContourPair::createCutline(outcut_len, axis, (lanes_num & 1) ? items_dir : -items_dir, top_ena);

//    uint k = 0;
//    set(k++, incut);
//    set(k++, clone_fwd);
//    set(k++, cutline_fwd);

//    for (size_t i = 0; i < lanes_num; i++) {
//        if ((i & 1) == 0) { // forward
//            if (i != 0) {
//                set(k++, clone_fwd_first);
//                set(k++, cutline_fwd);
//            }

//            for (size_t j = 1; j < items_num - 1; j++) {
//                set(k++, clone_fwd);
//                set(k++, cutline_fwd);
//            }

//            if (i == lanes_num - 1) { // exit
//                set(k++, clone_fwd);
//            } else { // cutline to the next lane
//                set(k++, clone_fwd_last);
//                set(k++, cutline_last);
//            }
//        } else { // reverse
//            set(k++, clone_rev_first);
//            set(k++, cutline_rev);

//            for (size_t j = 1; j < items_num - 1; j++) {
//                set(k++, clone_rev);
//                set(k++, cutline_rev);
//            }

//            if (i == lanes_num - 1) // exit
//                set(k++, clone_rev);
//            else {
//                set(k++, clone_rev_last);
//                set(k++, cutline_last);
//            }
//        }
//    }

//    set(k++, outcut);

    push_back(incut);
    push_back(clone_fwd);
    push_back(cutline_fwd);

    for (size_t i = 0; i < lanes_num; i++) {
        if ((i & 1) == 0) { // forward
            if (i != 0) {
                push_back(clone_fwd_first);
                push_back(cutline_fwd);
            }

            for (size_t j = 1; j < items_num - 1; j++) {
                push_back(clone_fwd);
                push_back(cutline_fwd);
            }

            if (i == lanes_num - 1) { // exit
                push_back(clone_fwd);
            } else { // cutline to the next lane
                push_back(clone_fwd_last);
                push_back(cutline_last);
            }
        } else { // reverse
            push_back(clone_rev_first);
            push_back(cutline_rev);

            for (size_t j = 1; j < items_num - 1; j++) {
                push_back(clone_rev);
                push_back(cutline_rev);
            }

            if (i == lanes_num - 1) // exit
                push_back(clone_rev);
            else {
                push_back(clone_rev_last);
                push_back(cutline_last);
            }
        }
    }

    push_back(outcut);

    connectContours();

    return true;
}

// Main direction, aux dir
std::vector<size_t> ContourList::getSplitPoints(size_t ctr_num, AXIS axis, DIR items_dir, DIR lanes_dir) const {
    if (ctr_num < m_contours.size()) {
        const ContourPair& pair = m_contours[ctr_num];
        vector<size_t> split = pair.getVerticesPoints();

        return ENWS_to_dir(split, axis, items_dir, lanes_dir);
    }

    return std::vector<size_t>();
}

vector<size_t> ContourList::ENWS_to_dir(const vector<size_t>& vert, AXIS axis, DIR items_dir, DIR lanes_dir) {
    if (vert.size() == 4) {
        vector<size_t> res (4, 0);

        if (lanes_dir == DIR::DIR_MINUS) {
            if (axis == AXIS::AXIS_X && items_dir == DIR::DIR_PLUS)
                res = {vert[2], vert[0], vert[1], vert[3]};
            else if (axis == AXIS::AXIS_X && items_dir == DIR::DIR_MINUS)
                res = {vert[0], vert[2], vert[1], vert[3]};
            else if (axis == AXIS::AXIS_Y && items_dir == DIR::DIR_PLUS)
                res = {vert[3], vert[1], vert[0], vert[2]};
            else if (axis == AXIS::AXIS_Y && items_dir == DIR::DIR_MINUS)
                res = {vert[1], vert[3], vert[0], vert[2]};
        } else {
            if (axis == AXIS::AXIS_X && items_dir == DIR::DIR_PLUS)
                res = {vert[2], vert[0], vert[3], vert[1]};
            else if (axis == AXIS::AXIS_X && items_dir == DIR::DIR_MINUS)
                res = {vert[0], vert[2], vert[3], vert[1]};
            else if (axis == AXIS::AXIS_Y && items_dir == DIR::DIR_PLUS)
                res = {vert[3], vert[1], vert[2], vert[0]};
            else if (axis == AXIS::AXIS_Y && items_dir == DIR::DIR_MINUS)
                res = {vert[1], vert[3], vert[2], vert[0]};
        }

        return res;
    }

    return vector<size_t>();
}

bool ContourList::saveUndo() {
    m_contours_old = m_contours;
    return !m_contours_old.empty();
}

void ContourList::clearUndo() {
    m_contours_old.clear();
}

void ContourList::undo() {
    if (!m_contours_old.empty()) {
        m_contours.swap(m_contours_old);
        m_contours_old.clear();
    }
}

bool ContourList::hasUndo() const { return !m_contours_old.empty(); }

bool ContourList::find(const fpoint_t& pt, const ContourRange& range, size_t& ctr_num, size_t& row_num, size_t& col_num) const {
    double e = 1; // mm
    ctr_num = row_num = col_num = 0;

    if (range.valid)
        e = max(range.width(), range.height()) / 100;


    for (const ContourPair& pair: m_contours) {
        row_num = 0;

        if (pair.bot()) {
            col_num = 0;

            for (const SegmentEntity* const ent: pair.bot()->entities()) {
                switch (ent->type()) {
                case DXF_ENTITY_TYPE::LINE: {
                    const SegmentLine* const line = reinterpret_cast<const SegmentLine*>(ent);
                    double h = line->distance(pt);

                    if (h <= e)
                        return true;
                }
                    break;

                case DXF_ENTITY_TYPE::ARC: {
                    const SegmentArc* const arc = reinterpret_cast<const SegmentArc*>(ent);
                    double h = arc->distance(pt);

                    if (h <= e)
                        return true;
                }
                    break;

                default:
                    break;
                }

                row_num++;
            }
        }

        row_num = 0;

        if (pair.top()) {
            col_num = 1;

            for (const SegmentEntity* const ent: pair.top()->entities()) {
                switch (ent->type()) {
                case DXF_ENTITY_TYPE::LINE: {
                    const SegmentLine* const line = reinterpret_cast<const SegmentLine*>(ent);
                    double h = line->distance(pt);

                    if (h <= e)
                        return true;
                }
                    break;

                case DXF_ENTITY_TYPE::ARC: {
                    const SegmentArc* const arc = reinterpret_cast<const SegmentArc*>(ent);
                    double h = arc->distance(pt);

                    if (h <= e)
                        return true;
                }
                    break;

                default:
                    break;
                }

                row_num++;
            }
        }

        ctr_num++;
    }

    return false;
}

void ContourList::connectContours() {
    fpoint_t pt(0,0);

    for (auto it = m_contours.begin(); it != m_contours.end(); ++it) {
        it->moveTo(pt);
        pt = it->getOutPoint();
    }
}
