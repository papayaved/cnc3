#include "dxf_intersect.h"
#include <cmath>

using namespace DxfIntersect;
using namespace std;

void DxfIntersect::intersect(const DxfLine& A, const DxfLine& B, fpoint_t& pt) {
    if (A.point_1() == B.point_0())
        pt = A.point_1();
    else if ((A.vertical() && B.vertical()) || (A.horizontal() && B.horizontal()) || A.same(B))
        pt = DxfLine::midpoint(A.point_1(), B.point_0());
    else if (A.vertical()) {
        pt.x = A.point_1().x;
        pt.y = B.y(pt.x);
    }
    else if (B.vertical()) {
        pt.x = B.point_0().x;
        pt.y = A.y(pt.x);
    }
    else {
        double Ak, Ab, Bk, Bb;

        A.func(Ak, Ab);
        B.func(Bk, Bb);

        if (Ak == Bk)
            pt = DxfLine::midpoint(A.point_1(), B.point_0());
        else {
            pt.x = (Bb - Ab) / (Ak - Bk);

            // main line nearest to 45 degrees
            Ak = fabs(Ak) - 0.5;
            Bk = fabs(Bk) - 0.5;

            if (fabs(Ak) < fabs(Bk))
                pt.y = A.y(pt.x);
            else
                pt.y = B.y(pt.x);
        }
    }
}

bool DxfIntersect::intersect(const DxfLine& A, const DxfArc& B, fpoint_t (&pt)[2], double (&angle)[2]) {
    double x0 = B.center().x;
    double y0 = B.center().y;
    double R = B.radius();

    if (A.vertical()) {
        double b = -2 * y0;
        double x = A.point_0().x;
        double dx = x - x0;
        double c = y0*y0 + dx*dx - R*R;

        double D = b*b - 4*c;

        if (D >= 0) {
            double sqrt_D = sqrt(D);
            pt[0].x = pt[1].x = x;
            pt[0].y = (-b + sqrt_D) / 2;
            pt[1].y = (-b - sqrt_D) / 2;

            angle[0] = atangent(B.center(), pt[0], R);
            angle[1] = atangent(B.center(), pt[1], R);

            return true;
        }
    }
    else {
        double k = A.k();
        double b = A.b();

        double P = k * k + 1;
        double Q = 2 * k * b - 2 * x0 - 2 * k * y0;
        double C = b * b + y0 * y0 - 2 * b * y0 - R * R + x0 * x0;

        double D = Q * Q - 4 * P * C;

        if (D >= 0) {
            double sqrt_D = sqrt(D);
            double _2_P = 2 * P;
            pt[0].x = (-Q + sqrt_D) / _2_P;
            pt[1].x = (-Q - sqrt_D) / _2_P;

            pt[0].y = A.y(pt[0].x);
            pt[1].y = A.y(pt[1].x);

            angle[0] = atangent(B.center(), pt[0], R);
            angle[1] = atangent(B.center(), pt[1], R);

    //        if (angle[0] > angle[1]) swap(angle[0], angle[1]);

            return true;
        }
    }

    return false;
}

bool DxfIntersect::intersect(const DxfArc& A, const DxfArc& B, double (&angleA)[2], double (&angleB)[2]) {
    if (A.intersected(B)) {
        double R1 = A.radius();
        double R2 = B.radius();

        double _dx = B.center().x - A.center().x;
        double _dy = B.center().y - A.center().y;
        double d__2 = _dx * _dx + _dy * _dy;

        double d = sqrt(d__2);

        double R1__2 = R1*R1;

        double a = (R1__2 - R2*R2 + d__2) / (2 * d);

        double h = sqrt(R1__2 - a*a);

        fpoint_t pt[2];
        pt[0].x = A.center().x + (a * _dx + h * _dy) / d;
        pt[1].x = A.center().x + (a * _dx - h * _dy) / d;

        pt[0].y = A.center().y + (a * _dy + h * _dx) / d;
        pt[1].y = A.center().y + (a * _dy - h * _dx) / d;

        angleA[0] = atangent(A.center(), pt[0], R1);
        angleA[1] = atangent(A.center(), pt[1], R1);

        angleB[0] = atangent(B.center(), pt[0], R2);
        angleB[1] = atangent(B.center(), pt[1], R2);

        bool swap_req;
        if (B.radius() > A.radius()) {
            double dB[2];
            dB[0] = DxfArc::deltaAngle(B.CCW(), angleB[0], B.endAngle());
            dB[1] = DxfArc::deltaAngle(B.CCW(), angleB[1], B.endAngle());
            swap_req = fabs(dB[1]) < fabs(dB[0]);
        } else {
            double dA[2];
            dA[0] = DxfArc::deltaAngle(A.CCW(), angleA[0], A.endAngle());
            dA[1] = DxfArc::deltaAngle(A.CCW(), angleA[1], A.endAngle());
            swap_req = fabs(dA[1]) < fabs(dA[0]);
        }

        if (swap_req) {
            swap(angleA[0], angleA[1]);
            swap(angleB[0], angleB[1]);
            swap(pt[0], pt[1]);
        }

        return true;
    }

    return false;
}

double DxfIntersect::atangent(double dx, double dy, double R) {
    if (R <= 0) // error
        return 0;

    double angle = asin(fabs(dy) / R); // [-90, 90] -> [0,90]

    if (dx >= 0)
        angle = dy >= 0 ? angle : 2 * M_PI - angle;
    else
        angle = dy >= 0 ? M_PI - angle : M_PI + angle;

    if (angle >= 2*M_PI || angle <= 0)
        angle = 0;

    return angle;
}

double DxfIntersect::atangent(const fpoint_t& C, const fpoint_t& A, double R) {
    return atangent(A.x - C.x, A.y - C.y, R);
}
