#pragma once

#include "PLFPayoffPricer.h"
#include "common/Types.h"
#include "market/Market.h"
#include "payoff/Observable.h"
#include "payoff/Payoff.h"
#include "pricer/HestonFormula.h"

namespace pricer {

/**
 * Price piecewise linear payoff by applying Heston formula to each segment.
 */
class HestonPricer final : public PLFPayoffPricer {
   public:
    explicit HestonPricer(const market::Market& market, const HestonParams& params)
        : PLFPayoffPricer(market), _params(params) {}

   private:
    const HestonParams& _params;

    double callFormula(const double F, const double K, const double T, const double dF,
                       const market::BSVolSlice&) const override {
        return hestonCallFormula(F, K, T, dF, _params);
    }

    double digitalCallFormula(double F, double K, double T, double dF,
                              const market::BSVolSlice&) const override {
        return hestonDigitalCallFormula(F, K, T, dF, _params);
    }
};

inline double hestonPricer(const payoff::PayoffNodePtr& payment, const market::Market& market,
                           const HestonParams& params) {
    return HestonPricer(market, params).price(payment);
}
}  // namespace pricer