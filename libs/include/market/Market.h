#pragma once

#include <optional>
#include <string>

#include "BSVolSlice.h"
#include "common/Date.h"

namespace market {
using calendar::Date;
using calendar::yearFraction;

class Market {
   public:
    Market(const Date pricingDate) : _pricingDate(pricingDate){};
    virtual ~Market() = default;
    Date getPricingDate() const { return _pricingDate; };
    // TODO FixingType (e.g., CLOSE)
    virtual std::optional<double> getPrice(const std::string& symbol, const Date& date) const = 0;
    virtual double getDiscountFactor(double T) const = 0;
    double getDiscountFactor(const Date date) const {
        return getDiscountFactor(yearFraction(getPricingDate(), date));
    }
    virtual double getForward(const std::string& symbol, double T) const = 0;
    double getForward(const std::string& symbol, const Date date) const {
        return getForward(symbol, yearFraction(getPricingDate(), date));
    }
    virtual const BSVolSlice& getBSVolSlice(const std::string& symbol, const Date& date) const = 0;

   private:
    const Date _pricingDate;
};
}  // namespace market