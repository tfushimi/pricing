#pragma once
#include <cmath>

#include "market/Curve.h"
namespace market {

class ConstantDiscountCurve final : public Curve {
   public:
    explicit ConstantDiscountCurve(const Date pricingDate, const double rate)
        : Curve(pricingDate), _rate(rate) {}

    double get(const double T) override { return std::exp(-_rate * T); }

   private:
    double _rate;
};
}  // namespace market
