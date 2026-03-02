#pragma once
#include <cmath>

#include "market/DiscountCurve.h"
namespace market {

class ConstantDiscountCurve final : public DiscountCurve {
   public:
    explicit ConstantDiscountCurve(const Date pricingDate, const double rate)
        : DiscountCurve(pricingDate), _rate(rate) {}

    double get(const double T) override { return std::exp(-_rate * T); }

   private:
    double _rate;
};
}  // namespace market
