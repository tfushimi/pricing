#pragma once

#include <valarray>

#include "PayoffPricer.h"
#include "common/types.h"
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
    MCPricer(const ProcessType& process, const int nPaths)
        : _processStateStepper(mc::ProcessStateStepper<ProcessType>(process)), _nPaths(nPaths) {}

    ~MCPricer() override = default;

    double price(const payoff::Payoff& payoff, const market::Market& market) override {
        const auto* cashPayment = dynamic_cast<const payoff::CashPayment*>(&payoff);

        if (!cashPayment) {
            throw std::invalid_argument("not a CashPayment");
        }

        const auto discountCurve = market.getDiscountCurve(market.getPricingDate());

        if (!discountCurve) {
            throw std::invalid_argument("Discount curve not found");
        }

        const auto newPayoff = payoff::applyMarket(cashPayment->getAmount(), market);
        // TODO define getFixingDates
        const auto fixingDatesSet = payoff::getFixings(newPayoff);
        std::vector<Date> fixingDates;
        fixingDates.reserve(fixingDatesSet.size());
        for (const auto& fixing : fixingDatesSet) {
            fixingDates.push_back(fixing.getDate());
        }
        const auto timeGrid = mc::TimeGrid{fixingDates, market.getPricingDate(), 1.0 / 12.0};

        // TODO store seed
        mc::RNG rng;

        const auto scenario = _processStateStepper.run(timeGrid, _nPaths, rng);
        const auto sample = payoff::applyFixings(newPayoff, scenario);

        const double dF = discountCurve->get(cashPayment->getSettlementDate());

        return dF * sample.sum() / sample.size();
    }

   private:
    const mc::ProcessStateStepper<ProcessType> _processStateStepper;
    const int _nPaths;
};
}  // namespace pricer