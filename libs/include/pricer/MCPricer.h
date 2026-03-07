#pragma once

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
    MCPricer(const std::string& symbol, const ProcessType& process, const int nPaths,
             const mc::RNG& rng)
        : _symbol(symbol),
          _processStateStepper(mc::ProcessStateStepper<ProcessType>(process)),
          _nPaths(nPaths),
          _rng(rng) {}

    ~MCPricer() override = default;

    double price(const payoff::PayoffNodePtr& payment, const market::Market& market) override {
        const auto* cashPayment = dynamic_cast<const payoff::CashPayment*>(payment.get());

        if (!cashPayment) {
            throw std::invalid_argument("not a CashPayment");
        }

        const auto discountCurve = market.getDiscountCurve();

        if (!discountCurve) {
            throw std::invalid_argument("Discount curve not found");
        }

        const auto payoff = payoff::applyMarket(cashPayment->getAmountPtr(), market);
        const auto [symbols, fixingDates] = payoff::getSymbolsAndFixingDates(payoff);

        if (symbols.size() == 0) {
            throw std::invalid_argument("No symbol found");
        }
        if (symbols.size() > 1) {
            throw std::invalid_argument("Multi-assets not supported by MCPricer");
        }
        if (!symbols.contains(_symbol)) {
            throw std::invalid_argument("Symbol not found: " + _symbol);
        }
        const auto timeGrid = mc::TimeGrid{fixingDates, market.getPricingDate(), 1.0 / 12.0};
        const auto scenario = _processStateStepper.run(timeGrid, _nPaths, _rng);
        const auto sample = payoff::applyFixings(payoff, scenario);
        const auto dF = discountCurve->get(cashPayment->getSettlementDate());

        return dF * sample.sum() / sample.size();
    }

   private:
    const std::string _symbol;
    const mc::ProcessStateStepper<ProcessType> _processStateStepper;
    const int _nPaths;
    mc::RNG _rng;
};
}  // namespace pricer