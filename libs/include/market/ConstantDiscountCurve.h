#pragma once
#include <cmath>

#include "common/Date.h"
#include "market/Curve.h"

namespace market {

class ConstantDiscountCurve final : public Curve {
   public:
    explicit ConstantDiscountCurve(const calendar::Date pricingDate, const double rate)
        : Curve(pricingDate), _rate(rate) {}

    double operator()(const double T) const override { return std::exp(-_rate * T); }

   private:
    double _rate;
};
}  // namespace market
