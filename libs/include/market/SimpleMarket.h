#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include "Curve.h"
#include "common/Types.h"
#include "market/BSVolSlice.h"
#include "market/Market.h"

namespace market {
/**
 * A simple market implementation for testing and prototyping.
 * Combines a constant risk-free rate with a vol surface applied
 * uniformly across all maturities — either SVI or flat (constant) vol.
 */
class SimpleMarket final : public Market {
   public:
    using VolSliceFactory = std::function<std::unique_ptr<BSVolSlice>(double forward, double T)>;

    SimpleMarket(const calendar::Date pricingDate, std::string symbol, const double spot, const double rate,
                 const double dividend, const SVIParams& sviParams)
        : SimpleMarket(pricingDate, std::move(symbol), spot, rate, dividend,
                       [sviParams](const double forward, const double T) {
                           return std::make_unique<SVIVolSlice>(forward, T, sviParams);
                       }) {}

    SimpleMarket(const calendar::Date pricingDate, std::string symbol, const double spot, const double rate,
                 const double dividend, const double vol)
        : SimpleMarket(pricingDate, std::move(symbol), spot, rate, dividend,
                       [vol](const double forward, const double T) {
                           return std::make_unique<FlatVolSlice>(forward, T, vol);
                       }) {}

    std::optional<double> getPrice(const std::string& symbol, const calendar::Date& date) const override {
        if (symbol == _symbol && date <= getPricingDate()) {
            return _spot;
        }
        return std::nullopt;
    }

    double getDiscountFactor(const double T) const override { return _discountCurve(T); }

    double getForward(const std::string&, const double T) const override {
        return _forwardCurve(T);
    }

    const BSVolSlice& getBSVolSlice(const std::string& symbol, const calendar::Date& date) const override {
        const auto key = std::make_pair(symbol, date);
        const auto it = _bsVolSlices.find(key);
        if (it != _bsVolSlices.end()) {
            return *it->second;
        }
        const auto T = calendar::yearFraction(getPricingDate(), date);
        const auto [inserted_it, _] =
            _bsVolSlices.emplace(key, _volSliceFactory(_forwardCurve(T), T));
        return *inserted_it->second;
    }

   private:
    SimpleMarket(const calendar::Date pricingDate, std::string symbol, const double spot, const double rate,
                 const double dividend, VolSliceFactory factory)
        : Market(pricingDate),
          _symbol(std::move(symbol)),
          _spot(spot),
          _discountCurve(ConstantDiscountCurve(pricingDate, rate)),
          _forwardCurve(ConstantForwardCurve(pricingDate, spot, rate - dividend)),
          _volSliceFactory(std::move(factory)) {}

    std::string _symbol;
    mutable std::map<std::pair<std::string, calendar::Date>, std::unique_ptr<BSVolSlice>> _bsVolSlices;
    double _spot;
    ConstantDiscountCurve _discountCurve;
    ConstantForwardCurve _forwardCurve;
    VolSliceFactory _volSliceFactory;
};

}  // namespace market
