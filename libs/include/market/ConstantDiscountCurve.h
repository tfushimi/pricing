#pragma once
#include <cmath>

#include "common/types.h"
#include "market/Curve.h"

namespace market {

class ConstantDiscountCurve final : public Curve {
   public:
    explicit ConstantDiscountCurve(const Date pricingDate, const double rate)
        : Curve(pricingDate), _rate(rate) {}

    double get(const double T) const override { return std::exp(-_rate * T); }

   private:
    double _rate;
};
}  // namespace market
