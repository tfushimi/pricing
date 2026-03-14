#pragma once

#include "BSFormula.h"
#include "PLFPayoffPricer.h"
#include "market/Market.h"
#include "payoff/Observable.h"
#include "payoff/Payoff.h"

namespace pricer {

/**
 * Price piecewise linear payoff by applying BlackScholes formula to each segment.
 */
class BSPricer final : public PLFPayoffPricer {
   public:
    explicit BSPricer(const market::Market& market) : PLFPayoffPricer(market) {}

   private:
    double callFormula(const double F, const double K, const double T, const double dF,
                       const market::BSVolSlice& bsVolSlice) const override {
        return bsCallFormula(F, K, T, dF, bsVolSlice.vol(K));
    }

    double digitalCallFormula(const double F, const double K, const double T, const double dF,
                              const market::BSVolSlice& bsVolSlice) const override {
        return bsDigitalFormula(F, K, T, dF, bsVolSlice.vol(K), bsVolSlice.dVolDStrike(K));
    }
};

inline double bsPricer(const payoff::PayoffNodePtr& payment, const market::Market& market) {
    return BSPricer(market).price(payment);
}
}  // namespace pricer