#pragma once

#include <memory>
#include <optional>
#include <string>

#include "BSVolSlice.h"

namespace market {
class Market {
   public:
    Market() = default;
    virtual ~Market() = default;
    virtual std::string getPricingDate() const = 0;
    virtual std::optional<double> getPrice(const std::string& symbol,
                                           const std::string& date) const = 0;
    virtual double getDiscountFactor(const std::string& date) const = 0;
    virtual std::unique_ptr<BSVolSlice> getBSVolSlice(const std::string& date) const = 0;
};
}  // namespace market