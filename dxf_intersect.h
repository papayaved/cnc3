#ifndef DXF_INTERSECT_H
#define DXF_INTERSECT_H

#include "dxf_line.h"
#include "dxf_arc.h"
#include "dxf_circle.h"
#include "dxf_point.h"

namespace DxfIntersect {
    void intersect(const DxfLine& A, const DxfLine& B, fpoint_t& pt);
    bool intersect(const DxfLine& A, const DxfArc& B, fpoint_t (&pts)[2], double (&angles)[2]);
    bool intersect(const DxfArc& A, const DxfArc& B, double (&A_angles)[2], double (&B_angles)[2]);
    double atangent(double dx, double dy, double R);
    double atangent(const fpoint_t& C, const fpoint_t& A, double R);

};

#endif // DXF_INTERSECT_H
