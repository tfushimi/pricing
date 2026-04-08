#pragma once
#include <random>

#include "mc/Process.h"

namespace mc {

class RNG {
   public:
    virtual ~RNG() = default;
    explicit RNG(const std::size_t seed = std::random_device{}())
        : _engine(seed), _dist(0.0, 1.0) {}

    virtual void fill(Sample& sample) {
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