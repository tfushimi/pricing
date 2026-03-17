#pragma once

#include <utility>

#include "PayoffPricer.h"
#include "market/Market.h"
#include "mc/ProcessStateStepper.h"
#include "mc/RNG.h"
#include "mc/TimeGrid.h"
#include "payoff/Payoff.h"
#include "payoff/Transforms.h"

namespace pricer {

template <typename ProcessType>
class MCPricer final : public PayoffPricer {
   public:
    MCPricer(const market::Market& market, const ProcessType& process, const int nPaths,
             const mc::RNG& rng)
        : _market(market),
          _processStateStepper(mc::ProcessStateStepper<ProcessType>(process)),
          _nPaths(nPaths),
          _rng(rng) {}

    ~MCPricer() override = default;

    double price(const payoff::PayoffNodePtr& _payoff, const double maxDt) {
        const auto newPayoff = payoff::applyMarket(_payoff, _market);
        const auto [symbols, fixingDates] = payoff::getSymbolsAndFixingDates(newPayoff);

        if (symbols.empty()) {
            throw std::invalid_argument("No symbol found in payoff");
        }

        const auto timeGrid = mc::TimeGrid{fixingDates, _market.getPricingDate(), maxDt};
        const auto scenario = _processStateStepper.run(timeGrid, _nPaths, _rng);
        const auto sample = payoff::applyFixings(newPayoff, _market, scenario);

        return sample.sum() / sample.size();
    }

    double price(const payoff::PayoffNodePtr& _payoff) override { return price(_payoff, 1 / 12.0); }

   private:
    const market::Market& _market;
    const mc::ProcessStateStepper<ProcessType> _processStateStepper;
    const int _nPaths;
    mc::RNG _rng;
};
}  // namespace pricer