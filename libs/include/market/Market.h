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
    virtual std::optional<double> getPrice(const std::string& symbol, const Date& date) const = 0;
    virtual double getDiscountFactor(const Date& date) const = 0;
    virtual std::unique_ptr<BSVolSlice> getBSVolSlice(const Date& date) const = 0;
};
}  // namespace market