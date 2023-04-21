#include "contour_pass.h"
#include <QDebug>

using namespace std;

void ContourPass::contour_replace(std::deque<ContourPair> &rcv, const std::deque<ContourPair> &src) {
    rcv.clear();
    for (const ContourPair& pair: src)
        rcv.push_back(pair);
}

void ContourPass::offset_replace(std::vector<offset_t> &rcv, const std::vector<offset_t> &src) {
    rcv.clear();
    for (offset_t value: src) {
        if (value.offset < M_PRECISION) value.offset = 0;
        rcv.push_back(value);
    }
}

ContourPass::ContourPass(const ContourList& base) : m_base(base), m_contours(deque<ContourPair>()) {}
ContourPass::~ContourPass() {
    clear();
}

void ContourPass::clear() {
    for (ContourPair& pair: m_contours)
        pair.clear();

    m_contours.clear();
}

bool ContourPass::empty() const { return m_contours.empty(); }

deque<ContourPair>& ContourPass::generate() {
    clear();

    for (const ContourPair& pair: m_base.contours()) {
        if (!pair.empty())
            switch (pair.type()) {
            case CONTOUR_TYPE::MAIN_CONTOUR:
            {
                deque<ContourPair> pass;
                deque<ContourPair> tab;
                deque<ContourPair> overcut; // 2 contours (forward, reverse)
                deque<ContourPair> out; // only 1 if it exists it replaces the overcut

//#if !defined(STONE)
                vector<offset_t> offsets = pair.cut().getOffsets();
                vector<offset_t> tabOffsets = pair.cut().getTabOffsets();
                OFFSET_SIDE side = pair.cut().offset_side;

                bool aux_offset_ena = pair.cut().aux_offset_ena;
                OFFSET_SIDE aux_offset_side = pair.cut().aux_offset_side;
                double aux_offset = pair.cut().aux_offset;

                ContourPair aux_pair(pair, CONTOUR_TYPE::PASS_CONTOUR);

                if (aux_offset_ena)
                    aux_pair.offset(aux_offset_side, aux_offset);

                bool dir = 0;

                double tab_value = aux_pair.cut().tab;
                size_t tab_seg = aux_pair.cut().tab_seg;
                double overcut_value = aux_pair.cut().overcut;

                if (tab_seg != 0 || tab_value < M_TAB_MIN || tab_value > (aux_pair.lengthBot() - M_TAB_MIN))
                    tab_value = 0;

                if (overcut_value < M_OVERCUT_MIN || overcut_value > (aux_pair.lengthBot() - tab_value) / 2)
                    overcut_value = 0;

                {
                    ContourPair ctr(aux_pair, CONTOUR_TYPE::PASS_CONTOUR);

                    size_t bot_idx = aux_pair.countBot(), top_idx = aux_pair.countTop();

                    if (tab_seg != 0) {
                        bot_idx = tab_seg < ctr.countBot() ? ctr.countBot() - tab_seg : ctr.countBot();
                        top_idx = tab_seg < ctr.countTop() ? ctr.countTop() - tab_seg : ctr.countTop();
                    }
                    else
                        ctr.split_rev(tab_value, bot_idx, top_idx);

                    for (size_t i = 0; i < offsets.size(); i++, dir = !dir) {
                        ContourPair clone(ctr);
                        clone.offset(side, offsets[i].offset);
                        clone.choiceMode(offsets[i].mode_id);
                        clone.remove_from(bot_idx, top_idx);
                        if (dir)
                            clone.reverse();
                        pass.push_back(move(clone));
                    }

                    // todo: don't make tab for even passes
                    ctr.setType(CONTOUR_TYPE::TAB_CONTOUR);
                    dir = !dir;

                    for (size_t i = 0; i < tabOffsets.size(); i++, dir = !dir) {
                        ContourPair clone(ctr);
                        clone.offset(side, tabOffsets[i].offset);
                        clone.choiceMode(tabOffsets[i].mode_id);
                        clone.remove_before(bot_idx, top_idx);
                        if (dir)
                            clone.reverse();
                        tab.push_back(move(clone));
                    }
                }

                {
                    ContourPair ctr(aux_pair, CONTOUR_TYPE::OVERCUT_CONTOUR);
                    size_t bot_idx = 0, top_idx = 0;

                    ctr.getVerticesPoints(overcut_value, bot_idx, top_idx);

                    ctr.offset(side, pair.cut().getOvercutOffset().offset);
                    ctr.choiceMode(pair.cut().getOvercutOffset().mode_id);
                    ctr.remove_from(bot_idx, top_idx);

                    ContourPair clone(ctr);
                    clone.reverse();

                    overcut.push_back(move(ctr));
                    overcut.push_back(move(clone));
                }

                if (aux_pair.hasOut()) {
                    ContourPair ctr(aux_pair, CONTOUR_TYPE::OUT_CONTOUR);

                    // todo: give indeces

                    ctr.offset(side, pair.cut().getOvercutOffset().offset);
                    ctr.choiceMode(pair.cut().getOvercutOffset().mode_id);

                    // todo: it can be problem if segment were removed
                    ctr.remove_from(aux_pair.getOutNum() + 1, aux_pair.getOutNum() + 1);

                    out.push_back(move(ctr));
                }

                printDebug(pass, tab, overcut, out);
                deque<ContourPair> ctr;

                if (aux_pair.isLoop() && !aux_pair.hasOut())
                    ctr = joinLoop(pass, tab, overcut);
                else
                    ctr = join(pass, tab, out);

                move_back(ctr);
            }
                break;
            case CONTOUR_TYPE::CUTLINE_CONTOUR:
                printDebug(pair);
                copy_back(pair);
                break;
            default:
                break;
            }
    }
//#else
//                vector<offset_t> offsets = pair.cut().getOffsets();
//                OFFSET_SIDE side = pair.cut().offset_side;

//                bool aux_offset_ena = pair.cut().aux_offset_ena;
//                OFFSET_SIDE aux_offset_side = pair.cut().aux_offset_side;
//                double aux_offset = pair.cut().aux_offset;

//                ContourPair aux_pair(CONTOUR_TYPE::PASS_CONTOUR, pair);

//                if (aux_offset_ena)
//                    aux_pair.offset(aux_offset_side, aux_offset);

//                vector<offset_t> tabOffsets = offsets;

//                bool dir = 0;

//                double tab_value = aux_pair.cut().tab;
//                size_t tab_seg = aux_pair.cut().tab_seg;
//                double overcut_value = aux_pair.cut().overcut;

//                if (tab_seg != 0 || tab_value < M_TAB_MIN || tab_value > (aux_pair.lengthBot() - M_TAB_MIN))
//                    tab_value = 0;

//                if (overcut_value < M_OVERCUT_MIN || overcut_value > aux_pair.lengthBot() / 2)
//                    overcut_value = 0;

//                {
//                    ContourPair ctr(CONTOUR_TYPE::PASS_CONTOUR, aux_pair);

//                    size_t bot_idx = aux_pair.countBot(), top_idx = aux_pair.countTop();

//                    if (tab_seg != 0) {
//                        bot_idx = tab_seg < ctr.countBot() ? ctr.countBot() - tab_seg : ctr.countBot();
//                        top_idx = tab_seg < ctr.countTop() ? ctr.countTop() - tab_seg : ctr.countTop();
//                    }
//                    else
//                        ctr.split_rev(tab_value, bot_idx, top_idx);

//                    for (size_t i = 0; i < offsets.size(); i++, dir = !dir) {
//                        ContourPair clone(ctr);
//                        clone.offset(side, offsets[i].offset);
//                        clone.choiceMode(offsets[i].mode_id);
//                        clone.remove_from(bot_idx, top_idx);
//                        if (dir)
//                            clone.reverse();
//                        pass.push_back(move(clone));
//                    }

//                    // todo: don't make tab for even passes
//                    ctr.setType(CONTOUR_TYPE::TAB_CONTOUR);
//                    dir = !dir;

//                    for (size_t i = 0; i < tabOffsets.size(); i++, dir = !dir) {
//                        ContourPair clone(ctr);
//                        clone.offset(side, tabOffsets[i].offset);
//                        clone.choiceMode(tabOffsets[i].mode_id);
//                        clone.remove_before(bot_idx, top_idx);
//                        if (dir)
//                            clone.reverse();
//                        tab.push_back(move(clone));
//                    }
//                }

//                {
//                    ContourPair ctr(CONTOUR_TYPE::OVERCUT_CONTOUR, aux_pair);
//                    size_t bot_idx = 0, top_idx = 0;

//                    ctr.split(overcut_value, bot_idx, top_idx);

//                    ctr.offset(side, tabOffsets[0].offset);
//                    ctr.choiceMode(tabOffsets[0].mode_id);
//                    ctr.remove_from(bot_idx, top_idx);

//                    ContourPair clone(ctr);
//                    clone.reverse();

//                    overcut.push_back(move(ctr));
//                    overcut.push_back(move(clone));
//                }

////                {
////                    ContourPair ctr = pair.clone();
////                    ctr.type = CONTOUR_TYPE::CONTOUR_OUT;
////                    ctr.offset(pair.side, pair.cut.getOutOffset().offset);
////                }

//                printDebug(pass, tab, overcut, out);
//                deque<ContourPair> ctr;

//                if (aux_pair.isLoop() && !aux_pair.hasOut())
//                    ctr = joinLoop(pass, tab, overcut);
//                else
//                    ctr = join(pass, tab, overcut, out);

//                move_back(ctr);
//            }
//                break;
//            case CONTOUR_TYPE::CUTLINE_CONTOUR:
//                printDebug(pair);
//                copy_back(pair);
//                break;
//            default:
//                break;
//            }
//    }
//#endif

    return m_contours;
}

void ContourPass::connect(const ContourPair& prev, ContourPair& next) {
    if (!prev.botEmpty() && !next.botEmpty() && prev.bot()->back() && next.bot()->front()) {
        fpoint_t A = prev.bot()->back()->point_1();
        fpoint_t B = next.bot()->front()->point_0();

        if (A != B)
            next.bot()->push_front( DxfLine(A, B) );
    }

    if (!prev.topEmpty() && !next.topEmpty() && prev.top()->back() && next.top()->front()) {
        fpoint_t A = prev.top()->back()->point_1();
        fpoint_t B = next.top()->front()->point_0();

        if (A != B)
            next.top()->push_front( DxfLine(A, B) );
    }
}

deque<ContourPair> ContourPass::join(deque<ContourPair> &pass, deque<ContourPair> &tab, deque<ContourPair> &overcut) {
    deque<ContourPair> contours;

    if (!pass.empty()) {
        for (ContourPair& pair: pass) {
            if (!pair.empty()) {
                if (!contours.empty())
                    connect(contours.back(), pair);

                contours.push_back(pair);
            }
        }
        pass.clear();
    }
    else
        clear(pass);

    if (!contours.empty()) {
        if (!tab.empty()) {
            for (ContourPair& pair: tab) {
                if (!pair.empty()) {
                    connect(contours.back(), pair);
                    contours.push_back(pair);
                }
            }
            tab.clear();
        }
        else
            clear(tab);

        if (!overcut.empty()) {
            for (ContourPair& pair: overcut) {
                if (!pair.empty()) {
                    connect(contours.back(), pair);
                    contours.push_back(pair);
                }
            }
            overcut.clear();
        }
        else
            clear(overcut);
    }
    else {
        clear(tab);
        clear(overcut);
    }

    return contours;
}

void ContourPass::join(deque<ContourPair>& contours, ContourPair& out) {
    if (!out.empty()) {
        connect(contours.back(), out);
        contours.push_back(out);
    }
    else
        out.clear();
}

deque<ContourPair> ContourPass::join(deque<ContourPair>& pass, deque<ContourPair>& tab, deque<ContourPair>& overcut, ContourPair& out) {
    deque<ContourPair> contours = join(pass, tab, overcut);
    join(contours, out);
    return contours;
}

deque<ContourPair> ContourPass::joinLoop(deque<ContourPair> &pass, deque<ContourPair> &tab, deque<ContourPair> &overcut) {
    deque<ContourPair> contours = join(pass, tab, overcut);

    ContourPair out(CONTOUR_TYPE::OUT_CONTOUR);
    out.setGeneratorModes(contours.back());
    const ContourPair& last = contours.back();
    const ContourPair& first = contours.front();

    if (!first.botEmpty() && !last.botEmpty()) {
        const fpoint_valid_t A = last.bot()->last_point();
        const fpoint_valid_t B = first.bot()->first_point();

        if (A.valid && B.valid && A != B)
            out.bot()->push_back( DxfLine(A, B) );
    }

    if (!first.topEmpty() && !last.topEmpty()) {
        const fpoint_valid_t A = last.top()->last_point();
        const fpoint_valid_t B = first.top()->first_point();

        if (A.valid && B.valid && A != B)
            out.top()->push_back( DxfLine(A, B) );
    }

    join(contours, out);
    return contours;
}

void ContourPass::copy_back(const ContourPair& pair) {
    if (m_contours.empty())
        m_contours.push_back(pair);
    else if (pair.type() == CONTOUR_TYPE::CUTLINE_CONTOUR && !pair.empty()) {
        ContourPair new_pair(pair);
        new_pair.changeFirstPoint(m_contours.back().lastBot(), m_contours.back().lastTop());
        m_contours.push_back(new_pair);
    }
}

void ContourPass::move_back(deque<ContourPair>& ctr) {
    if (!ctr.empty()) {
        if (m_contours.empty())
            m_contours = ctr;
        else if (m_contours.back().type() == CONTOUR_TYPE::CUTLINE_CONTOUR && !ctr.front().empty()) {
            m_contours.back().changeLastPoint(ctr.front().firstBot(), ctr.front().firstTop());
            m_contours.insert(m_contours.end(), ctr.begin(), ctr.end());
        }
    }

    ctr.clear();
}

void ContourPass::printDebug(const deque<ContourPair>& contours) {
    if (contours.empty())
        qDebug() << "Empty";
    else
        for (const ContourPair& pair: contours)
            qDebug() << pair.toString().c_str();
}

void ContourPass::printDebug(const ContourPair& pair) {
    qDebug() << pair.toString().c_str();
}

void ContourPass::printDebug(const deque<ContourPair>& pass, const deque<ContourPair>& tab, const deque<ContourPair>& overcut, const deque<ContourPair>& out) {
    printDebug(pass);
    printDebug(tab);
    printDebug(overcut);
    printDebug(out);
}

void ContourPass::clear(deque<ContourPair>& contours) {
    for (ContourPair& pair: contours) {
        pair.clear();
    }
    contours.clear();
}
