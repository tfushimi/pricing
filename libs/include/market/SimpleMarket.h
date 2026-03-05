#pragma once

#include <memory>
#include <string>

#include "common/types.h"
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
                 const double rate, vol::SVIParams sviParams)
        : _pricingDate(pricingDate),
          _symbol(symbol),
          _spot(spot),
          _discountCurve(std::make_shared<ConstantDiscountCurve>(pricingDate, rate)),
          _sviParams(std::move(sviParams)) {}

    Date getPricingDate() const override { return _pricingDate; }

    std::optional<double> getPrice(const std::string& symbol, const Date& date) const override {
        if (symbol == _symbol && date <= _pricingDate) {
            return _spot;
        }

        return std::nullopt;
    }

    std::shared_ptr<Curve> getDiscountCurve(const Date&) const override {
        return _discountCurve;
    }

    std::shared_ptr<BSVolSlice> getBSVolSlice(const std::string&, const Date& date) const override {
        const auto T = yearFraction(_pricingDate, date);
        const auto disc = _discountCurve->get(T);
        const auto forward = _spot / disc;
        return std::make_shared<SVIVolSlice>(forward, T, _sviParams);
    }

   private:
    Date _pricingDate;
    std::string _symbol;
    double _spot;
    std::shared_ptr<Curve> _discountCurve;
    vol::SVIParams _sviParams;
};

}  // namespace market