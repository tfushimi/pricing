#pragma once

#include "common/types.h"

class Curve {
   public:
    explicit Curve(const Date pricingDate) : _pricingDate(pricingDate) {}
    virtual ~Curve() = default;
    double get(const Date& date) const {
        return get(yearFraction(_pricingDate, date));
    }
    virtual double get(double T) const = 0;

   private:
    Date _pricingDate;
};