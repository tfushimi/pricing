#pragma once

#include "market/BSVolSlice.h"

namespace market {

class FlatVolSlice final : public BSVolSlice {
   public:
    FlatVolSlice(const double forward, const double time, const double vol)
        : _forward(forward), _time(time), _vol(vol) {}

    double forward() const override { return _forward; }
    double time() const override { return _time; }
    double vol(const double strike) const override { return _vol; }
    double dVolDStrike(const double strike) const override { return 0.0; }

   private:
    double _forward;
    double _time;
    double _vol;
};
}  // namespace market