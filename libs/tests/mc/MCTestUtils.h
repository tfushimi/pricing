#pragma once

#include <valarray>

#include "mc/RNG.h"

class ConstantRNG final : public mc::RNG {
   public:
    explicit ConstantRNG(const double val) : RNG(0), _val(val) {}
    ~ConstantRNG() override = default;
    void fill(mc::Sample& sample) override {
        for (double& value : sample) {
            value = _val;
        }
    }

   private:
    double _val;
};

inline double mean(const mc::Sample& s) {
    return s.sum() / static_cast<double>(s.size());
}

inline double variance(const mc::Sample& s) {
    const double avg = mean(s);
    return mean(s * s) - avg * avg;
}