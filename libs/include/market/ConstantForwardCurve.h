#pragma once
#include <cmath>

#include "common/types.h"
#include "market/Curve.h"

namespace market {

class ConstantForwardCurve final : public Curve {
   public:
    explicit ConstantForwardCurve(const Date pricingDate, const double spot, const double rate)
        : Curve(pricingDate), _spot(spot), _rate(rate) {}

    double get(const double T) const override { return _spot * std::exp(_rate * T); }

   private:
    const double _spot;
    const double _rate;
};
}  // namespace market
