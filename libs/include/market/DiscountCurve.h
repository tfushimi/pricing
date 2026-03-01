#pragma once

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