#include "gcode.h"

#include <regex>
#include <sstream>
#include <QDebug>
#include <limits>
#include <cmath>

using namespace std;

//list<string> GCode::SplitByFrames(const string& gcode) {
//    list<string> frames;
////    const regex re(R"(.+[\n\r$]++)");
//    const regex re(R"([^\n\r\0]++)");
////    const regex empty_string_re(R"(\s+)");

//    auto frame_begin = std::sregex_iterator(gcode.begin(), gcode.end(), re);
//    auto frame_end = std::sregex_iterator();

//    for (std::sregex_iterator it = frame_begin; it != frame_end; ++it) {
//        std::smatch match = *it;
//        string s = match.str();
//        if (s.length() > 1)
//            frames.push_back(s.substr(0, s.length() - 1));
//    }

//    return frames;
//}

// don't use
void GCode::pointsOffset(std::vector<GPoint>& pts, const GPoint &pt) {
//    const GPoint& last = pts[pts.size() - 1];
    const GPoint& last = *(pts.cend() - 1);
    qreal dx = last.x() - pt.x();
    qreal dy = last.y() - pt.y();

    for (auto it = pts.begin(); it != pts.end(); ++it) {
        it->rx() -= dx;
        it->ry() -= dy;
    }
}

void GCode::append(const GEntity &entity) {
    if (!frames.empty()) {
        frames.back().push_back(entity);
    }
}

void GCode::append(const GEntity &entity0, const GEntity &entity1) {
    if (!frames.empty()) {
        frames.back().push_back(entity0);
        frames.back().push_back(entity1);
    }
}

void GCode::append(const GEntity &entity0, const GEntity &entity1, const GEntity &entity2, const GEntity &entity3) {
    if (!frames.empty()) {
        frames.back().push_back(entity0);
        frames.back().push_back(entity1);
        frames.back().push_back(entity2);
        frames.back().push_back(entity3);
    }
}

// add to current frame
void GCode::append(const GFrame& frame) {
    if (!frames.empty())
        frames.back().append(frame);
    else
        frames.push_back(frame);
}

void GCode::push_back(const GeneratorMode& genMode, bool fb_ena, const double low_thld[2], const double high_thld[2]) {
    bool low_hv = genMode.low_high_voltage;

    frames.push_back( GFrame::M105(genMode.roll_velocity) );

    frames.push_back( GFrame::M106(genMode.current_level) );
    frames.push_back( GFrame::M107(genMode.pulse_width, genMode.pulse_ratio) );

    frames.push_back( low_hv ? GFrame::M108() : GFrame::M109() );

    if (fb_ena)
        append( GFrame::M110( low_thld[low_hv], high_thld[low_hv] ) );
}

GCode &GCode::operator=(GCode &&other) noexcept {
    if (this != &other) {
        frames = std::move(other.frames);
    }
    return *this;
}

GCode &GCode::operator=(const GCode &other) noexcept {
    if (this != &other) {
        frames = other.frames;
    }
    return *this;
}

int GCode::parse(const string& gcode) {
    frames.clear();
    int i = 1;
    bool err = false;

    istringstream input;
    input.str(gcode);

    for (string line; getline(input, line); ) {
        GFrame frame = GFrame::parse(line, err, frame);
        if (err) break;
        i++;

        if (!frame.empty()) frames.push_back(frame);
    }

    return err ? i : 0;
}

#ifndef STONE
bool GCode::generate(const cut_t& cut, const deque<ContourPair>& contours, const CncParam& par) {
#else
bool GCode::generate(const cut_t& cut, const deque<ContourPair>& contours, const CncParam& /*par*/) {
#endif
    frames.clear();

    if (!contours.empty()) {
        const DxfEntity *bot = nullptr, *top = nullptr;
        bool cutline = false;
        bool init = true;
        bool tab = false;
        bool isSpeed = false; // todo: if changed        
        uint8_t G_reg = 0;
        struct {double value; bool init; } F {0, false};
        struct {uint8_t value; bool init; } M105 {0, false}; // todo: template <type,valid>

        size_t i = 0;
        for (const ContourPair& pair: contours) {
            if (!pair.empty()) {
                if (init) {
                    init = false;
                    frames.push_back(GFrame::delimiter());

                    if (cut.uv_ena) {
                        frames.push_back(GFrame::M100(cut.L, cut.H));
                        frames.push_back(GFrame::M101(cut.T));

                        if (cut.D_ena) {
                            frames.push_back(GFrame::M102(cut.D + cut.wire_D / 2, cut.D_tilted));
                            frames.push_back(GFrame::M103(cut.D_axis, cut.D_wire_side));
                        }
                    }

                    frames.push_back(GFrame::M40()); // pump enable
//                    frames.push_back(GFrame::M0()); // stop

//                    frames.push_back(GFrame::G4(uint32_t(pair.cut().pump_delay) * 1000));
//                    if (pair.cut().pump_pause)
//                        frames.push_back(GFrame::M0());

#if defined(STONE)
                    const DxfEntity* ent = contours.front().bot()->front();

                    M105.value = ent->hasRollVel() ? ent->rollVel() : 7;
                    M105.init = true;

                    frames.push_back(GFrame::M105(M105.value));
#else
                    push_back(pair.generatorMode(), par.fb_ena, par.low_thld, par.high_thld);
                    frames.push_back(GFrame::M82()); // roll enable
#endif
                    frames.push_back(GFrame::M84()); // HV enable

//                    frames.push_back(GFrame::enable());
//                    frames.push_back(GFrame::M0()); // stop

                    // init first
                    if (!pair.empty()) {
                        pair.reader(bot, top);

                        if (bot || top) {
                            frames.push_back(GFrame::G92());
                            if (bot) append(GEntity('X', bot->point_0().x), GEntity('Y', bot->point_0().y));
                            if (top) append(GEntity('U', top->point_0().x), GEntity('V', top->point_0().y));
                        }
                    }

                    if (pair.isCutline() && !cutline) {
                        cutline = true;
                        frames.push_back(GFrame::cutlineBegin());
                    }
                    else if (cutline && !pair.isCutline()) {
                        cutline = false;
                        frames.push_back(GFrame::cutlineEnd());
                    }
                }
                else {
                    if (pair.isCutline() && !cutline) {
                        cutline = true;
                        frames.push_back(GFrame::cutlineBegin());
                    }
                    else if (cutline && !pair.isCutline()) {
                        cutline = false;
                        frames.push_back(GFrame::cutlineEnd());
                    }
                    else if (cut.tab_pause && !tab && pair.type() == CONTOUR_TYPE::TAB_CONTOUR) {
                        tab = true;
                        frames.push_back(GFrame::disable());
                        frames.push_back(GFrame::M0());
//                        frames.push_back(GFrame::enable());
//                        frames.push_back(GFrame::M0());
                    }

#ifndef STONE
                    push_back(pair.generatorMode(), par.fb_ena, par.low_thld, par.high_thld);
#endif
                }

                auto r = pair.reader(bot, top);

                while (bot || top) {
                    if (bot) {
                        if (bot->type() == ENTITY_TYPE::LINE) {
                            if (bot->hasRollVel()) {
                                if (!M105.init || M105.value != bot->rollVel()) {
                                    M105.value = bot->rollVel();
                                    frames.push_back(GFrame::M105(M105.value));
                                }
                            }

                            frames.push_back(GFrame::G1());
                            G_reg = 1;
                            append(GEntity('X', bot->point_1().x), GEntity('Y', bot->point_1().y));

                            if (bot->hasSpeed()) {
                                isSpeed = true;
                                if (!F.init || !(bot->speed() > (F.value - 0.001) && bot->speed() < (F.value + 0.001))) {
                                    F.init = true;
                                    F.value = bot->speed();
                                    append(GEntity('F', F.value));
                                }
                            }
                            else if (!isSpeed) {
                                isSpeed = true;
                                append(GEntity('F', cut.speed));
                            }
                        }
                        else if (bot->type() == ENTITY_TYPE::ARC) {
                            if (bot->CCW()) {
                                frames.push_back(GFrame::G3());
                                G_reg = 3;
                            }
                            else {
                                frames.push_back(GFrame::G2());
                                G_reg = 2;
                            }

                            fpoint_t A = bot->point_0();
                            fpoint_t B = bot->point_1();
                            fpoint_t C = bot->center();

                            append(GEntity('X', B.x), GEntity('Y', B.y), GEntity('I', C.x - A.x), GEntity('J', C.y - A.y));

                            if (!isSpeed) {
                                isSpeed = true;
                                append(GEntity('F', cut.speed));
                            }
                        }
                    }

                    if (top) {
                        if (top->type() == ENTITY_TYPE::LINE) {
                            if (G_reg != 1) {
                                append(GFrame::G1());
                                G_reg = 1;
                            }

                            append(GEntity('U', top->point_1().x), GEntity('V', top->point_1().y));
                        }
                        else if (top->type() == ENTITY_TYPE::ARC) {
                            if (top->CCW()) {
                                if (G_reg != 3) {
                                    append(GFrame::G3());
                                    G_reg = 3;
                                }
                            }
                            else {
                                if (G_reg != 2) {
                                    append(GFrame::G2());
                                    G_reg = 2;
                                }
                            }

                            fpoint_t A = top->point_0();
                            fpoint_t B = top->point_1();
                            fpoint_t C = top->center();

                            append(GEntity('U', B.x), GEntity('V', B.y), GEntity('I', C.x - A.x), GEntity('J', C.y - A.y));
                        }
                    }

                    if (cut.seg_pause &&
                                (pair.type() == CONTOUR_TYPE::PASS_CONTOUR  ||
                                 pair.type() == CONTOUR_TYPE::TAB_CONTOUR   ||
                                (pair.type() == CONTOUR_TYPE::CUTLINE_CONTOUR && i == 0))
                        ) {

                        frames.push_back(GFrame::G4(uint32_t(cut.seg_pause) * 1000));
                    } else if (bot && bot->hasPause()) {
                        frames.push_back(GFrame::G4(uint32_t(bot->pause()) * 1000));
                    }

                    r.next(bot, top);

                    i++;
                }
            }
        }

        if (cutline) frames.push_back(GFrame::cutlineEnd());

        frames.push_back(GFrame::disable());
        frames.push_back(GFrame::M2());
        frames.push_back(GFrame::delimiter());

        return true;
    }
    m_error = "Contour empty";
    return false;
}

void cone_cw(const fpoint_t& C, double Dbot, double Dtop, int& i, fpoint_t& bot, fpoint_t& top) {
    switch (i) {
    case 0: bot = fpoint_t{C.x + Dbot/2, C.y}; top = fpoint_t{C.x + Dtop/2, C.y}; i = 3; break;
    case 1: bot = fpoint_t{C.x, C.y + Dbot/2}; top = fpoint_t{C.x, C.y + Dtop/2}; i--; break;
    case 2: bot = fpoint_t{C.x - Dbot/2, C.y}; top = fpoint_t{C.x - Dtop/2, C.y}; i--; break;
    case 3: bot = fpoint_t{C.x, C.y - Dbot/2}; top = fpoint_t{C.x, C.y - Dtop/2}; i--; break;
    default: bot = fpoint_t(); top = fpoint_t(); i = 0; break;
    }
}

void GCode::addG2G2(fpoint_t& A, const fpoint_t& B, fpoint_t& A2, const fpoint_t& B2, const fpoint_t& C) {
    frames.push_back(GFrame::G2(B.x, B.y, C.x - A.x, C.y - A.y, B2.x, B2.y, C.x - A2.x, C.y - A2.y));
    A = B;
    A2 = B2;
}

// Tapped cutting calibration
bool GCode::generate(double D_top, double D_bot, double L, double H, double T, bool D_ena, double D, bool D_tilted, AXIS D_axis, DIR D_wire_side, double cutline, AXIS cutline_axis) {
    frames.clear();
    frames.push_back(GFrame::delimiter());
    frames.push_back(GFrame::M100(L, H));
    frames.push_back(GFrame::M101(T));

    if (D_ena) {
        frames.push_back(GFrame::M102(D, D_tilted));
        frames.push_back(GFrame::M103(D_axis, D_wire_side));
    }

    frames.push_back(GFrame::G92(0,0,0,0));

    bool sign = cutline < 0;
    cutline = fabs(cutline);

    double cutline_uv = cutline + (D_bot - D_top) / 2;

    if (sign) {
        cutline = -cutline;
        cutline_uv = -cutline_uv;
    }

    fpoint_t A(0,0), B, A2(0,0), B2, C;

    switch (cutline_axis) {
    case AXIS::AXIS_X:
        B = fpoint_t(cutline, 0);
        B2 = fpoint_t(cutline_uv, 0);
        break;
    case AXIS::AXIS_Y:
        B = fpoint_t(0, cutline);
        B2 = fpoint_t(0, cutline_uv);
        break;
    default:
        return false;
    }

    frames.push_back(GFrame::G1(B.x, B.y, B2.x, B2.y));

    A = B;
    A2 = B2;

    if (cutline_axis == AXIS::AXIS_X)
        C = !sign ? fpoint_t(A.x + D_bot/2, A.y) : fpoint_t(A.x - D_bot/2, A.y);
    else
        C = !sign ? fpoint_t(A.x, A.y + D_bot/2) : fpoint_t(A.x, A.y - D_bot/2);

    int i = cutline_axis == AXIS::AXIS_X ? (sign ? 3 : 1) : (sign ? 0 : 2);

    for (int k = 0; k < 4; k++) {
        cone_cw(C, D_bot, D_top, i, B, B2);
        addG2G2(A, B, A2, B2, C);
    }

    frames.push_back(GFrame::G1(0, 0, 0, 0));

    frames.push_back(GFrame::disable());
    frames.push_back(GFrame::M2());
    frames.push_back(GFrame::delimiter());

    return true;
}

void GCode::addLine(Dxf& cnt, const GCommand& cmd, bool inc_ena, const fpoint_t& offset, double& X, double& Y, LAYER_T layer) {
    fpoint_t A(X, Y), B(X, Y);

    if (layer == LAYER_T::BOT && cmd.isXY()) {
        if (cmd.isX())
            B.x = X = inc_ena ? X + cmd.X() : offset.x + cmd.X();

        if (cmd.isY())
            B.y = Y = inc_ena ? Y + cmd.Y() : offset.y + cmd.Y();
    }
    else if (layer == LAYER_T::TOP && cmd.isUV()) {
        if (cmd.isU())
            B.x = X = inc_ena ? X + cmd.U() : offset.x + cmd.U();

        if (cmd.isV())
            B.y = Y = inc_ena ? Y + cmd.V() : offset.y + cmd.V();
    }

    if (A == B)
        cnt.push_back(new DxfPoint(X, Y));
    else
        cnt.push_back(new DxfLine(A, B));
}

void GCode::addArc(Dxf& cnt, const GCommand& cmd, bool inc_ena, const fpoint_t& offset, double& X, double& Y, bool ccw, LAYER_T layer) {
    fpoint_t A(X, Y), B(X, Y), C(X, Y);

    if (layer == LAYER_T::BOT && cmd.isR()) {
        if (cmd.isX())
            B.x = X = inc_ena ? X + cmd.X() : offset.x + cmd.X();

        if (cmd.isY())
            B.y = Y = inc_ena ? Y + cmd.Y() : offset.y + cmd.Y();

        double R = cmd.R();

        if (fabs(R) < M_PRECISION)
            cnt.push_back(new DxfPoint(X, Y));
        else
            cnt.push_back(new DxfArc(A, B, R, ccw));
    }
    else if (layer == LAYER_T::TOP && cmd.isR2()) {
        if (cmd.isU())
            B.x = X = inc_ena ? X + cmd.U() : offset.x + cmd.U();

        if (cmd.isV())
            B.y = Y = inc_ena ? Y + cmd.V() : offset.y + cmd.V();

        double R = cmd.R2();

        if (fabs(R) < M_PRECISION)
            cnt.push_back(new DxfPoint(X, Y));
        else
            cnt.push_back(new DxfArc(A, B, R, ccw));
    }
    else if (layer == LAYER_T::BOT) {
        if (cmd.isI())
            C.x += cmd.I();

        if (cmd.isJ())
            C.y += cmd.J();

        if (cmd.isX())
            B.x = X = inc_ena ? X + cmd.X() : offset.x + cmd.X();

        if (cmd.isY())
            B.y = Y = inc_ena ? Y + cmd.Y() : offset.y + cmd.Y();

        if (A == C && B == C)
            cnt.push_back(new DxfPoint(X, Y));
        else if (A == C || B == C)
            cnt.push_back(new DxfLine(A, B));
        else
            cnt.push_back(new DxfArc(A, B, C, ccw));
    }
    else if (layer == LAYER_T::TOP) {
        if (cmd.isI2())
            C.x += cmd.I2();

        if (cmd.isJ2())
            C.y += cmd.J2();

        if (cmd.isU())
            B.x = X = inc_ena ? X + cmd.U() : offset.x + cmd.U();

        if (cmd.isV())
            B.y = Y = inc_ena ? Y + cmd.V() : offset.y + cmd.V();

        if (A == C && B == C)
            cnt.push_back(new DxfPoint(X, Y));
        else if (A == C || B == C)
            cnt.push_back(new DxfLine(A, B));
        else
            cnt.push_back(new DxfArc(A, B, C, ccw));
    }
    else
        cnt.push_back(new DxfPoint(X, Y));
}

void f(bool inc_ena, bool valid, double cmd_X, double& X) {
    if (valid) {
        if (inc_ena)
            X += cmd_X;
        else
            X = cmd_X;
    }
}

void GUV(vector<GFrame>::iterator it, int G, bool isR2, double U, double V, double R2, double I2, double J2) {
    switch (G) {
    case 0: case 1:
        it->append(GFrame::G1UV(U, V));
        break;
    case 2:
        if (isR2)
            it->append(GFrame::G2UV(U, V, R2));
        else
            it->append(GFrame::G2UV(U, V, I2, J2));
        break;
    case 3:
        if (isR2)
            it->append(GFrame::G3UV(U, V, R2));
        else
            it->append(GFrame::G3UV(U, V, I2, J2));
        break;
    }
}

// to one string one full abs G-command
void GCode::gNormalize() {
    bool inc_ena = false, uv_ena = false, first_xyuv = true, erase = false;
    int G = -1;
    UV_ENA axis_ena = UV_ENA::UNKNOWN;
    double X = 0, Y = 0, U = 0, V = 0, I = 0, J = 0, R = 0, I2 = 0, J2 = 0, R2 = 0;

    for (auto it = frames.begin(); it < frames.end();) {
        list<GCommand> cmdList = it->get(G, axis_ena);

        for (const GCommand& cmd: cmdList) {
            if (first_xyuv && cmd.isXYUV()) {
                first_xyuv = false;

                if (cmd.isUV())
                    uv_ena = true;
            }

            if (cmd.isG()) {
                switch (cmd.G()) {
                case 90:
                    inc_ena = false;
                    erase = true;
                    break;

                case 91:
                    inc_ena = true;
                    erase = true;
                    break;

                case 92:
                    erase = true;

                    if (cmd.isX()) X = cmd.X();
                    if (cmd.isY()) Y = cmd.Y();
                    if (cmd.isU()) U = cmd.U();
                    if (cmd.isV()) V = cmd.V();

                    it = uv_ena ? frames.insert(it, GFrame::G92(X, Y, U, V)) : frames.insert(it, GFrame::G92(X, Y));
                    ++it;

                    break;

                case 0: case 1: case 2: case 3:
                    erase = true;

                    f(inc_ena, cmd.isX(), cmd.X(), X);
                    f(inc_ena, cmd.isY(), cmd.Y(), Y);
                    f(inc_ena, cmd.isU(), cmd.U(), U);
                    f(inc_ena, cmd.isV(), cmd.V(), V);

                    I = cmd.isI() ? cmd.I() : 0;
                    J = cmd.isJ() ? cmd.J() : 0;
                    R = cmd.isR() ? cmd.R() : 0;

                    I2 = cmd.isI2() ? cmd.I2() : 0;
                    J2 = cmd.isJ2() ? cmd.J2() : 0;
                    R2 = cmd.isR2() ? cmd.R2() : 0;

                    switch (cmd.G()) {
                    case 0:
                        it = uv_ena ? frames.insert(it, GFrame::G0(X, Y, U, V)) : frames.insert(it, GFrame::G0(X, Y));
                        ++it;

                        break;

                    case 1:
                        it = frames.insert(it, GFrame::G1(X, Y));
                        if (cmd.isF()) it->append(GFrame::F(cmd.F()));

                        if (uv_ena) {
                            if (cmd.isG2()) G = cmd.G2();
                            GUV(it, G, cmd.isR2(), U, V, R2, I2, J2);
                        }

                        ++it;

                        break;

                    case 2:
                        it = cmd.isR() ? frames.insert(it, GFrame::G2(X, Y, R)) : frames.insert(it, GFrame::G2(X, Y, I, J));
                        if (cmd.isF()) it->append(GFrame::F(cmd.F()));

                        if (uv_ena) {
                            if (cmd.isG2()) G = cmd.G2();
                            GUV(it, G, cmd.isR2(), U, V, R2, I2, J2);
                        }

                        ++it;

                        break;

                    case 3:
                        it = cmd.isR() ? frames.insert(it, GFrame::G3(X, Y, R)) : frames.insert(it, GFrame::G3(X, Y, I, J));
                        if (cmd.isF()) it->append(GFrame::F(cmd.F()));

                        if (uv_ena) {
                            if (cmd.isG2()) G = cmd.G2();
                            GUV(it, G, cmd.isR2(), U, V, R2, I2, J2);
                        }

                        ++it;

                        break;
                    }
                    break;
                }
            }
        }

        if (erase) {
            erase = false;

            if (it != frames.end()) {
//                qDebug("%s\n", it->toString().c_str());
                it = frames.erase(it);
//                qDebug("%s\n", it->toString().c_str());
            }
        }
        else if (it != frames.end() && it->empty())
            it = frames.erase(it);
        else
            ++it;
    }
}

// for absolute
void GCode::RtoIJ() {
    double X = 0, Y = 0, U = 0, V = 0;
    int G = -1; // not valid
    UV_ENA axis_ena = UV_ENA::UNKNOWN;

    for (GFrame& frame: frames) {
        list<GCommand> cmdList = frame.get(G, axis_ena);

        for (const GCommand& cmd: cmdList) {
            if (cmd.type() == GCommandType::G) {
                if (cmd.isG()) {
                    switch (cmd.G()) {
                    case 0: case 92:
                        if (cmd.isX()) X = cmd.X();
                        if (cmd.isY()) Y = cmd.Y();
                        if (cmd.isU()) U = cmd.U();
                        if (cmd.isV()) V = cmd.V();

                        break;

                    case 1:
                        if (cmd.isX()) X = cmd.X();
                        if (cmd.isY()) Y = cmd.Y();

                        break;

                    case 2: case 3:
                    {
                        fpoint_t A(X,Y);

                        if (cmd.isX()) X = cmd.X();
                        if (cmd.isY()) Y = cmd.Y();

                        if (cmd.isR()) {
                            fpoint_t B(X,Y);
                            DxfArc arc(A, B, cmd.R(), cmd.G() == 3);

                            frame.replaceRtoIJ(true, arc.center());
                        }
                    }
                        break;

                    default:
                        break;
                    }
                }

                if (cmd.isG2()) {
                    switch (cmd.G2()) {
                    case 1:
                        if (cmd.isX()) U = cmd.U();
                        if (cmd.isY()) V = cmd.V();

                        break;

                    case 2: case 3:
                    {
                        fpoint_t A2(U,V);

                        if (cmd.isU()) X = cmd.U();
                        if (cmd.isV()) Y = cmd.V();

                        if (cmd.isR2()) {
                            fpoint_t B2(X,Y);
                            DxfArc arc(A2, B2, cmd.R2(), cmd.G2() == 3);

                            frame.replaceRtoIJ(false, arc.center());
                        }
                    }
                        break;

                    default:
                        break;
                    }
                }
            }
        }
    }
}

void GCode::normalize() {
    removeComments();
    gNormalize();
    RtoIJ();
}

deque<ContourPair> GCode::getContours(vector<pair<size_t, size_t>>* const map) const {
    double X = 0, Y = 0, U = 0, V = 0;
    int G = -1; // not valid
    UV_ENA axis_ena = UV_ENA::UNKNOWN;
    fpoint_t offset_xy(0, 0), offset_uv(0, 0);
    bool inc_ena = false;

    deque<ContourPair> contours({ContourPair()});

    if (map) map->clear();

    for (const GFrame& frame: frames) {
        list<GCommand> cmdList = frame.get(G, axis_ena);

        for (const GCommand& cmd: cmdList) {
            if (cmd.type() == GCommandType::G) {
                if (cmd.isG()) {
                    switch (cmd.G()) {
                    case 90:
                        inc_ena = false;
                        break;

                    case 91:
                        inc_ena = true;
                        break;

                    case 92:
                        if (contours.back().empty()) {
                            offset_xy = fpoint_t(0, 0);
                            offset_uv = fpoint_t(0, 0);
                            X = cmd.isX() ? cmd.X() : 0;
                            Y = cmd.isY() ? cmd.Y() : 0;
                            U = cmd.isU() ? cmd.U() : 0;
                            V = cmd.isV() ? cmd.V() : 0;
                        }
                        else {
                            if (cmd.isX()) offset_xy.x = X - cmd.X();
                            if (cmd.isY()) offset_xy.y = Y - cmd.Y();
                            if (cmd.isU()) offset_uv.x = U - cmd.U();
                            if (cmd.isV()) offset_xy.y = V - cmd.V();
                        }

                        break;

                    case 0:
                        if (cmd.isXY())
                            addLine(*contours.back().bot(), cmd, inc_ena, offset_xy, X, Y);

                        if (cmd.isUV())
                            addLine(*contours.back().top(), cmd, inc_ena, offset_uv, U, V, LAYER_T::TOP);

                        break;

                    case 1:
                        if (cmd.isXY())
                            addLine(*contours.back().bot(), cmd, inc_ena, offset_xy, X, Y);

                        break;

                    case 2: case 3:
                        if (cmd.isXY())
                            addArc(*contours.back().bot(), cmd, inc_ena, offset_xy, X, Y, cmd.G() == 3);

                        break;

                    default:
                        break;
                    }
                }

                if (cmd.isG2()) {
                    switch (cmd.G2()) {
                    case 1:
                        if (cmd.isUV())
                            addLine(*contours.back().top(), cmd, inc_ena, offset_uv, U, V, LAYER_T::TOP);

                        break;

                    case 2: case 3:
                        if (cmd.isUV())
                            addArc(*contours.back().top(), cmd, inc_ena, offset_uv, U, V, cmd.G() == 3, LAYER_T::TOP);

                        break;

                    default:
                        break;
                    }
                }
            }
        }

        // record for each frame
        if (map) {
            size_t n = contours.size();
            size_t m = contours.back().count();
            if (n) n -= 1;
            if (m) m -= 1;
            map->push_back(std::make_pair(n, m));
        }
    }

    return contours;
}

string GCode::toText() const {
    string s;
    for (auto it = frames.begin(); it != frames.end(); ) {
        string frame = it->toString();
        ++it;
        if (!frame.empty()) {
            if (it != frames.end())
                s += frame + "\n";
            else
                s += frame;
        }
    }
    return s;
}

std::list<string> GCode::toFrameList() {
    list<string> frameList;

    for (const GFrame& frame: frames) {
        GFrame new_frame(frame);
        new_frame.removeComment();

        string line = new_frame.toString();
        if (!line.empty())
            frameList.push_back(line);
    }

    return frameList;
}

void GCode::removeComments() {
    for (auto it = frames.begin(); it < frames.end();) {
        it->removeComment();

        if (it->empty())
            it = frames.erase(it);
        else
            ++it;
    }
}

//GCode GCode::noComment() const {
//    GCode gcode;

//    for (const GFrame& frame: frames) {
//        GFrame new_frame = frame.noComment();
//        if (!new_frame.empty())
//            gcode.frames.push_back(new_frame);
//    }

//    return gcode;
//}

list<string> GCode::splitTextByStrings(const string& text) {
    list<string> frames;
    string s;
    for (auto it = text.begin(); it != text.end(); ++it) {
        if (*it == '\n' || *it == '\r' || *it == '\0') {
            if (!s.empty()) {
                frames.push_back(s);
                s.clear();
            }
        }
        else {
            s += *it;
        }
    }

    if (!s.empty())
        frames.push_back(std::move(s));

    return frames;
}

bool GCode::compareFrames(const std::list<string>& ref, const std::list<string>& cmp, string& error_text) {
    int i = 0;
    auto it1 = ref.begin(), it2 = cmp.begin();

    for (; it1 != ref.end() && it2 != cmp.end(); ++it1, ++it2, i++) {
        const string& s1 = *it1;
        const string& s2 = *it2;

        if (s1.compare(s2) != 0) {
            error_text = "Read error at line " + std::to_string(i) + "\n";
            error_text += "Reference frame is " + s1 + "\n";
            error_text += "Received frame is " + s2;
            return false;
        }
    }

    if (it1 == ref.end() && it2 == cmp.end())
        return true;

    error_text = "Frames number error. Reference frames number are " + std::to_string(ref.size()) + ", Received frames number are " + std::to_string(cmp.size());
    return false;
}

string GCode::toText(list<string> &gframes, bool str_num_ena) {
    string txt;
    unsigned i = 1;

    if (str_num_ena) {
        for (const string& s : gframes)
            txt += "N" + to_string(i++) + " " + s + "\n";
    }
    else {
        for (auto it = gframes.begin(); it != gframes.end(); ++it)
            txt += *it + "\n";
    }
    return txt;
}

pair<pair<double, double>, pair<double, double>> GCode::MinMaxPlotAxis(const vector<GPoint>& pts) {
    double x_min = numeric_limits<double>::max(), x_max = numeric_limits<double>::min();
    double y_min = numeric_limits<double>::max(), y_max = numeric_limits<double>::min();

    for (const GPoint& pt: pts) {
        if (pt.x() < x_min) x_min = pt.x();
        else if (pt.x() > x_max) x_max = pt.x();

        if (pt.y() < y_min) y_min = pt.y();
        else if (pt.y() > y_max) y_max = pt.y();
    }

    double dx = x_max - x_min;
    double dy = y_max - y_min;
    double range = (dx > dy) ? dx : dy;

    return pair<pair<double, double>, pair<double, double>>(pair<double, double>(x_min, x_min + range), pair<double, double>(y_min, y_min + range));
}

void GCode::clear() { frames.clear(); }
bool GCode::empty() const { return frames.empty(); }

// Read UV settings from G-code
GCodeSettings GCode::getSettings() const {
    GCodeSettings res = GCodeSettings();
    int G = -1;
    UV_ENA axis_ena = UV_ENA::UNKNOWN;

    for (const GFrame& frame: frames) {
        list<GCommand> cmds = frame.get(G, axis_ena);

        for (const GCommand& cmd: cmds) {
            if (!res.valid.LH && cmd.isM() && cmd.M() == 100 && cmd.isP() && cmd.isQ()) {
                res.L = cmd.P();
                res.H = cmd.Q();
                res.valid.LH = true;
            }
            else if (!res.valid.T && cmd.isM() && cmd.M() == 101 && cmd.isP()) {
                res.T = cmd.P();
                res.valid.T = true;
            }
            else if (!res.D_ena && cmd.isM() && cmd.M() == 102 && cmd.isP()) {
                res.D = cmd.P();
                res.D_ena = true;

                if (cmd.isQ()) {
                    res.D_tilted_ena = (int)cmd.Q() != 0;
                }

                res.D_axis = AXIS::AXIS_Y;
                res.D_dir = DIR::DIR_MINUS;
            } else if (res.D_ena && cmd.isM() && cmd.M() == 103 && cmd.isP()) {
                res.D_axis = ((int)cmd.P() == 0) ? AXIS::AXIS_X : AXIS::AXIS_Y;

                if (cmd.isQ()) {
                    res.D_dir = ((int)cmd.Q() == 0) ? DIR::DIR_PLUS : DIR::DIR_MINUS;
                }
            }

            if (!res.uv_ena && res.valid.LH && res.valid.T && cmd.isUV()) { // first UV command
                res.uv_ena = true;
                break;
            }
        }

        if (res.uv_ena)
            break;
    }

    res.D_ena &= res.isUV();
    res.bot_coe = res.isUV() ? res.H / res.L : 0;

    return res;
}
