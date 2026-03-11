#pragma once

#include <memory>
#include <string>

#include "ConstantForwardCurve.h"
#include "common/types.h"
#include "market/ConstantDiscountCurve.h"
#include "market/Market.h"
#include "market/SVI.h"
#include "market/SVIVolSlice.h"

namespace market {
// TODO add ForwardCurve
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
          _forwardCurve(std::make_shared<ConstantForwardCurve>(pricingDate, spot, rate)),
          _sviParams(std::move(sviParams)) {}

    Date getPricingDate() const override { return _pricingDate; }

    std::optional<double> getPrice(const std::string& symbol, const Date& date) const override {
        if (symbol == _symbol && date <= _pricingDate) {
            return _spot;
        }

        return std::nullopt;
    }

    std::shared_ptr<Curve> getDiscountCurve() const override { return _discountCurve; }

    std::shared_ptr<Curve> getForwardCurve(const std::string&) const override {
        return _forwardCurve;
    }

    std::shared_ptr<BSVolSlice> getBSVolSlice(const std::string&, const Date& date) const override {
        const auto T = yearFraction(_pricingDate, date);
        return std::make_shared<SVIVolSlice>(_forwardCurve->get(T), T, _sviParams);
    }

   private:
    Date _pricingDate;
    std::string _symbol;
    double _spot;
    std::shared_ptr<Curve> _discountCurve;
    std::shared_ptr<Curve> _forwardCurve;
    vol::SVIParams _sviParams;
};

}  // namespace market