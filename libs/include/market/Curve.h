#pragma once

#include <cmath>

#include "common/Date.h"

namespace market {

class Curve {
   public:
    explicit Curve(const calendar::Date pricingDate) : _pricingDate(pricingDate) {}
    virtual ~Curve() = default;
    double operator()(const calendar::Date& date) const {
        return this->operator()(calendar::yearFraction(_pricingDate, date));
    }
    virtual double operator()(double T) const = 0;

   private:
    calendar::Date _pricingDate;
};

class ConstantDiscountCurve final : public Curve {
   public:
    explicit ConstantDiscountCurve(const calendar::Date pricingDate, const double rate)
        : Curve(pricingDate), _rate(rate) {}

    double operator()(const double T) const override { return std::exp(-_rate * T); }

   private:
    double _rate;
};

class ConstantForwardCurve final : public Curve {
   public:
    explicit ConstantForwardCurve(const calendar::Date pricingDate, const double spot,
                                  const double rate)
        : Curve(pricingDate), _spot(spot), _rate(rate) {}

    double operator()(const double T) const override { return _spot * std::exp(_rate * T); }

   private:
    const double _spot;
    const double _rate;
};
}  // namespace market