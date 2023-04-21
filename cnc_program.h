#ifndef CNC_H
#define CNC_H

#include <QVector>
#include <QList>

//"G0", "G1", "G4", "G92", "M2", "M30"

enum class CncLineType {STRAIGHT, ARC};

struct cnc_point_t {
    double x, y;
};

struct CncLine {
    CncLineType type;
    cnc_point_t xy[2], uv[2];
};

enum class CncCmdType { G, M, TAG };

struct CncCmd
{
    uint8_t code;
    CncCmdType type;
    uint32_t N;
    double X, Y, U, V, F, P, Q;
};

class CncProgram {
private:
    QList<CncCmd> list;

public:
//    void ParseGCode(const QString& txt);
//    QString GetGCode();
//    QVector<CncLine> GetFigure();
//    void Insert(size_t pos);
};

#endif // CNC_H
