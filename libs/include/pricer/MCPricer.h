#pragma once

#include <thread>
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

    Scenario generateScenario(const payoff::PayoffNodePtr& _payoff, const double maxDt) {
        const auto newPayoff = payoff::applyMarket(_payoff, _market);
        const auto [symbols, fixingDates] = payoff::getSymbolsAndFixingDates(newPayoff);

        if (symbols.empty()) {
            throw std::invalid_argument("No symbol found in payoff");
        }

        const auto timeGrid = mc::TimeGrid{fixingDates, _market.getPricingDate(), maxDt};
        return _processStateStepper.run(timeGrid, _nPaths, _rng);
    }

    double priceFromScenario(const payoff::PayoffNodePtr& _payoff, const Scenario& scenario) const {
        const auto newPayoff = payoff::applyMarket(_payoff, _market);
        const auto sample = payoff::applyFixings(newPayoff, _market, scenario);

        return sample.sum() / sample.size();
    }

    double price(const payoff::PayoffNodePtr& _payoff, const double maxDt) {
        const auto scenario = generateScenario(_payoff, maxDt);
        return priceFromScenario(_payoff, scenario);
    }

    double price(const payoff::PayoffNodePtr& _payoff) override { return price(_payoff, 1 / 12.0); }

   private:
    const market::Market& _market;
    const mc::ProcessStateStepper<ProcessType> _processStateStepper;
    const int _nPaths;
    mc::RNG _rng;
};

template <typename ProcessType>
class ParallelMCPricer final : public PayoffPricer {
   public:
    ParallelMCPricer(const market::Market& market, const ProcessType& process, const int nPaths,
                     const int nThreads)
        : _market(market),
          _processStateStepper(mc::ProcessStateStepper<ProcessType>(process)),
          _nPaths(nPaths),
          _nThreads(nThreads) {}

    ~ParallelMCPricer() override = default;

    double price(const payoff::PayoffNodePtr& _payoff) override {
        const auto newPayoff = payoff::applyMarket(_payoff, _market);
        const auto [symbols, fixingDates] = payoff::getSymbolsAndFixingDates(newPayoff);
        const auto timeGrid = mc::TimeGrid{fixingDates, _market.getPricingDate(), 1 / 12.0};

        std::vector sums(_nThreads, 0.0);
        std::vector<std::thread> threads;
        const int nPathsPerThread = _nPaths / _nThreads;

        for (int i = 0; i < _nThreads; i++) {
            threads.emplace_back([&, i]() {
                mc::RNG rng(i);
                const auto scenario = _processStateStepper.run(timeGrid, nPathsPerThread, rng);
                const auto sample = payoff::applyFixings(newPayoff, _market, scenario);
                sums[i] = sample.sum();
            });
        }

        for (auto& t : threads) {
            t.join();
        }

        return std::accumulate(sums.begin(), sums.end(), 0.0) / _nPaths;
    }

   private:
    const market::Market& _market;
    const mc::ProcessStateStepper<ProcessType> _processStateStepper;
    const int _nPaths;
    const int _nThreads;
};

}  // namespace pricer