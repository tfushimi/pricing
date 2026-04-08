#pragma once

#include <map>
#include <memory>
#include <string>

#include "ConstantForwardCurve.h"
#include "market/ConstantDiscountCurve.h"
#include "market/Market.h"
#include "market/SVI.h"
#include "market/SVIVolSlice.h"

namespace market {
/**
 * A simple market implementation for testing and prototyping.
 * Combines a constant risk-free rate with a single SVI vol smile
 * applied uniformly across all maturities.
 */
class SimpleMarket final : public Market {
   public:
    SimpleMarket(const Date pricingDate, const std::string& symbol, const double spot,
                 const double rate, const double dividend, vol::SVIParams sviParams)
        : _pricingDate(pricingDate),
          _symbol(symbol),
          _spot(spot),
          _discountCurve(ConstantDiscountCurve(pricingDate, rate)),
          _forwardCurve(ConstantForwardCurve(pricingDate, spot, rate - dividend)),
          _sviParams(std::move(sviParams)) {}

    Date getPricingDate() const override { return _pricingDate; }

    std::optional<double> getPrice(const std::string& symbol, const Date& date) const override {
        if (symbol == _symbol && date <= _pricingDate) {
            return _spot;
        }
        return std::nullopt;
    }

    double getDiscountFactor(const double T) const override { return _discountCurve(T); }

    double getForward(const std::string&, const double T) const override {
        return _forwardCurve(T);
    }

    const BSVolSlice& getBSVolSlice(const std::string& symbol, const Date& date) const override {
        const auto key = std::make_pair(symbol, date);
        const auto it = _bsVolSlices.find(key);
        if (it != _bsVolSlices.end()) {
            return *it->second;
        }
        const auto T = yearFraction(_pricingDate, date);
        const auto [inserted_it, _] = _bsVolSlices.emplace(
            key, std::make_unique<SVIVolSlice>(_forwardCurve(T), T, _sviParams));
        return *inserted_it->second;
    }

   private:
    Date _pricingDate;
    std::string _symbol;
    mutable std::map<std::pair<std::string, Date>, std::unique_ptr<BSVolSlice>> _bsVolSlices;
    double _spot;
    ConstantDiscountCurve _discountCurve;
    ConstantForwardCurve _forwardCurve;
    vol::SVIParams _sviParams;
};

}  // namespace market