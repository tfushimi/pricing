#pragma once

#include <soci/soci.h>

#include <map>
#include <memory>
#include <optional>
#include <string>

#include "BSVolSlice.h"
#include "Market.h"
#include "common/Date.h"

namespace market {

class MarketDB final : public Market {
   public:
    explicit MarketDB(calendar::Date pricingDate, const std::string& url);
    ~MarketDB() override = default;
    std::optional<double> getPrice(const std::string& symbol,
                                   const calendar::Date& date) const override;
    double getDiscountFactor(double T) const override;
    double getForward(const std::string& symbol, double T) const override;
    const BSVolSlice& getBSVolSlice(const std::string& symbol,
                                    const calendar::Date& date) const override;

   private:
    mutable soci::session sql;
    mutable std::map<std::string, double> _prices;
    mutable std::map<std::pair<std::string, calendar::Date>, std::unique_ptr<BSVolSlice>>
        _bsVolSlices;
};
}  // namespace market