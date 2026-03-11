#pragma once

#include <common/types.h>

#include <memory>
#include <optional>
#include <string>

#include "BSVolSlice.h"
#include "Curve.h"

namespace market {
class Market {
   public:
    Market() = default;
    virtual ~Market() = default;
    virtual Date getPricingDate() const = 0;
    // TODO FixingType (e.g., CLOSE)
    virtual std::optional<double> getPrice(const std::string& symbol, const Date& date) const = 0;
    virtual std::shared_ptr<Curve> getDiscountCurve() const = 0;
    virtual std::shared_ptr<Curve> getForwardCurve(const std::string& symbol) const = 0;
    virtual std::shared_ptr<BSVolSlice> getBSVolSlice(const std::string& symbol,
                                                      const Date& date) const = 0;
};
}  // namespace market