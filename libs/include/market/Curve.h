#pragma once

#include <cmath>

#include "common/Date.h"
#include "payoff/Observable.h"

namespace market {

// Abstract curve callable by time to expiry T (years) or by date.
// Used for discount factors and forward prices.
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

// Represents a data point on Curve
class CurvePoint final {
   public:
    explicit CurvePoint(const calendar::Date date, const double value)
        : _date(date), _value(value) {}
    ~CurvePoint() = default;
    calendar::Date getDate() const { return _date; }
    double getValue() const { return _value; }

   private:
    calendar::Date _date;
    double _value;
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