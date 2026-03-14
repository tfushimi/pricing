#pragma once

#include "common/types.h"

class Curve {
   public:
    explicit Curve(const Date pricingDate) : _pricingDate(pricingDate) {}
    virtual ~Curve() = default;
    double operator()(const Date& date) const {
        return this->operator()(yearFraction(_pricingDate, date));
    }
    virtual double operator()(double T) const = 0;

   private:
    Date _pricingDate;
};