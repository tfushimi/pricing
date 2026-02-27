#pragma once
#include <cmath>

class DiscountCurve {
   public:
    explicit DiscountCurve(const Date pricingDate) : _pricingDate(pricingDate) {}
    virtual ~DiscountCurve() = default;
    virtual double get(const Date& date) {
        const double T = yearFraction(_pricingDate, date);
        return get(T);
    }
    virtual double get(double T) = 0;

   private:
    Date _pricingDate;
};

class ConstantDiscountCurve final : public DiscountCurve {
   public:
    explicit ConstantDiscountCurve(Date pricingDate, const double rate)
        : DiscountCurve(pricingDate), _rate(rate) {}

    double get(const double T) override { return std::exp(-_rate * T); }

   private:
    double _rate;
};