#pragma once

#include <valarray>

#include "common/types.h"
#include "mc/RNG.h"

class ConstantRNG final : public mc::RNG {
   public:
    explicit ConstantRNG(const double val) : RNG(0), _val(val) {}
    ~ConstantRNG() override = default;
    void fill(Sample& sample) override {
        for (double& value : sample) {
            value = _val;
        }
    }

   private:
    double _val;
};

inline double mean(const Sample& s) {
    return s.sum() / static_cast<double>(s.size());
}