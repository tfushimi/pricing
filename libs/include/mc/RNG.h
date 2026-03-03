#pragma once
#include <random>

#include "common/types.h"

namespace mc {

class RNG {
   public:
    explicit RNG(const std::size_t seed = std::random_device{}())
        : _engine(seed), _dist(0.0, 1.0) {}

    void fill(Sample& sample) {
        for (double& value : sample) {
            value = _dist(_engine);
        }
    }

   private:
    std::mt19937 _engine;
    std::normal_distribution<> _dist;
};

// TODO define Antithetic variates, QuasiRNG (Sobol)
}  // namespace mc