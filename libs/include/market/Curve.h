#pragma once

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
}