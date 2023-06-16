#ifndef SEGMENT_INTERSECTION_H
#define SEGMENT_INTERSECTION_H

#include "segment_line.h"
#include "segment_arc.h"
#include "segment_circle.h"
#include "segment_point.h"

namespace SegmentIntersection {
    void intersect(const SegmentLine& A, const SegmentLine& B, fpoint_t& pt);
    bool intersect(const SegmentLine& A, const SegmentArc& B, fpoint_t (&pts)[2], double (&angles)[2]);
    bool intersect(const SegmentArc& A, const SegmentArc& B, double (&A_angles)[2], double (&B_angles)[2]);
    double atangent(double dx, double dy, double R);
    double atangent(const fpoint_t& C, const fpoint_t& A, double R);

};

#endif // SEGMENT_INTERSECTION_H
