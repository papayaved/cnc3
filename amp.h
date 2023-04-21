#ifndef AMP_H
#define AMP_H

class amp {

public:
    static constexpr double GainAmpPos(double Rpos, double Rpfb, double Rneg, double Rnfb) {
        return Rpfb / (Rpos + Rpfb) * (1.0 + Rnfb / Rneg);
    }

    static constexpr double GainResDiv(double R1 = 100e3, double R2 = 3.01e3, double R3 = 100e3, double R4 = 100e3) {
        return GainAmpPos(R3, R4, R3, R4) / (1.0 + R1 / R2 + R1 / (R3 + R4));
    }

//    static constexpr double GainResDivDiff(double R1 = 100e3, double R2 = 3.01e3, double R3 = 100e3, double R4 = 100e3) {
//        return 0;
//    }
};

#endif // AMP_H
