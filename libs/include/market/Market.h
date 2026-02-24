#pragma once

#include <memory>
#include <string>

#include "BSVolSlice.h"

namespace market {
class Market {
   public:
    Market() = default;
    virtual ~Market() = default;
    virtual std::string getPricingDate() const = 0;
    virtual double getSpot(const std::string& symbol) const = 0;
    virtual double getDiscountFactor(const std::string& date) const = 0;
    virtual std::unique_ptr<BSVolSlice> getBSVolSlice(const std::string& date) const = 0;
};
}  // namespace market