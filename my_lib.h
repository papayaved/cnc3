#ifndef MY_LIB_H
#define MY_LIB_H

#include <stddef.h>
#include <vector>

namespace my_lib {
    double circleSegmentSquareAlpha(double R, double alpha);
    double circleSegmentSquareNorm(double R, double d);
    std::vector<double>  splitCircleByEqualSquare(double R, unsigned n);
    void print(const std::vector<double>& v);

    std::vector<std::pair<double, double>> splitCircle(double R, unsigned n, double Vavg);
    void print(const std::vector< std::pair<double, double>>& v );
};

#endif // MY_LIB_H
