#pragma once

#include <common/types.h>

#include <memory>
#include <optional>
#include <string>

#include "BSVolSlice.h"

namespace market {
class Market {
   public:
    Market() = default;
    virtual ~Market() = default;
    virtual Date getPricingDate() const = 0;
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
    virtual std::shared_ptr<BSVolSlice> getBSVolSlice(const std::string& symbol,
                                                      const Date& date) const = 0;
};
}  // namespace market