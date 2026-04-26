#pragma once

#include <optional>
#include <string>
#include <map>
#include <memory>

#include "Market.h"
#include "BSVolSlice.h"
#include "common/Date.h"

namespace market {

class MarketImpl final : public Market {
public:
    explicit MarketImpl(const calendar::Date pricingDate) : Market(pricingDate){}
    ~MarketImpl() override = default;
    std::optional<double> getPrice(const std::string& symbol, const calendar::Date& date) const override;
    double getDiscountFactor(double T) const override;
    double getForward(const std::string& symbol, double T) const override;
    const BSVolSlice& getBSVolSlice(const std::string& symbol, const calendar::Date& date) const override;

private:
    mutable std::map<std::pair<std::string, calendar::Date>, std::unique_ptr<BSVolSlice>> _bsVolSlices;

};
}  // namespace market