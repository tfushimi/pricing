#pragma once

#include <cmath>
#include <vector>

#include "common/Date.h"
#include "numerics/interpolation/LinearInterpolator.h"
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

// Curve backed by (date, value) knots, interpolated linearly in time.
// Extrapolates using the slope of the nearest segment beyond the knot range.
class LinearInterpolatedCurve final : public Curve {
   public:
    LinearInterpolatedCurve(const calendar::Date pricingDate,
                            const std::vector<calendar::Date>& dates,
                            const std::vector<double>& values)
        : Curve(pricingDate), _interp(toYearFractions(pricingDate, dates), values) {}

    double operator()(const double T) const override { return _interp(T); }

   private:
    numerics::interpolation::LinearInterpolator _interp;

    static std::vector<double> toYearFractions(const calendar::Date pricingDate,
                                               const std::vector<calendar::Date>& dates) {
        std::vector<double> result;
        result.reserve(dates.size());
        for (const auto& date : dates) {
          result.push_back(calendar::yearFraction(pricingDate, date));
        }
        return result;
    }
};

// Curve backed by (date, value) knots, interpolated log-linearly in time.
// Equivalent to linearly interpolating the continuously compounded rate.
// Guarantees strictly positive values. Suitable for discount factors and forward prices.
class LogLinearInterpolatedCurve final : public Curve {
   public:
    LogLinearInterpolatedCurve(const calendar::Date pricingDate,
                               const std::vector<calendar::Date>& dates,
                               const std::vector<double>& values)
        : Curve(pricingDate), _interp(toYearFractions(pricingDate, dates), toLogs(values)) {}

    double operator()(const double T) const override { return std::exp(_interp(T)); }

   private:
    numerics::interpolation::LinearInterpolator _interp;

    static std::vector<double> toYearFractions(const calendar::Date pricingDate,
                                               const std::vector<calendar::Date>& dates) {
        std::vector<double> result;
        result.reserve(dates.size());
        for (const auto& date : dates) {
            result.push_back(calendar::yearFraction(pricingDate, date));
        }
        return result;
    }

    static std::vector<double> toLogs(const std::vector<double>& values) {
        std::vector<double> logs;
        logs.reserve(values.size());
        for (const double v : values) {
            logs.push_back(std::log(v));
        }
        return logs;
    }
};

}  // namespace market