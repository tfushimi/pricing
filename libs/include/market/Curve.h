#pragma once

class Curve {
   public:
    explicit Curve(const Date pricingDate) : _pricingDate(pricingDate) {}
    virtual ~Curve() = default;
    virtual double get(const Date& date) {
        const double T = yearFraction(_pricingDate, date);
        return get(T);
    }
    virtual double get(double T) = 0;

   private:
    Date _pricingDate;
};