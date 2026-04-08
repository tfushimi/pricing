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
using calendar::Date;

// TODO move this to RNG
using RNGFactory = std::function<std::unique_ptr<mc::RNG>(int seed)>;

template <typename ProcessType>
class MCPricer final : public PayoffPricer {
   public:
    MCPricer(
        const market::Market& market, const ProcessType& process, const int nPaths,
        const double maxDt = 1.0 / 12.0, const int nThreads = 1,
        const RNGFactory& rngFactory = [](const int seed) { return std::make_unique<mc::RNG>(seed); },
        const int seed = 0)
        : _market(market),
          _processStateStepper(mc::ProcessStateStepper<ProcessType>(process)),
          _nPaths(nPaths),
          _maxDt(maxDt),
          _nThreads(nThreads),
          _rngFactory(rngFactory),
          _seed(seed) {}

    ~MCPricer() override = default;

    std::vector<mc::Scenario> generateScenarios(const std::vector<Date>& fixingDates) const {

        const auto timeGrid = mc::TimeGrid{fixingDates, _market.getPricingDate(), _maxDt};

        if (_nThreads <= 1) {
            // single-threaded
            return {_processStateStepper.run(timeGrid, _nPaths, _rngFactory(_seed))};
        }

        std::vector<mc::Scenario> scenarios(_nThreads);
        std::vector<std::thread> threads;
        threads.reserve(_nThreads);

        const int nPathsPerThread = _nPaths / _nThreads;

        for (int i = 0; i < _nThreads; i++) {
            threads.emplace_back([&, i]() {
                scenarios[i] =
                    _processStateStepper.run(timeGrid, nPathsPerThread, _rngFactory(_seed + i));
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        return scenarios;
    }

    std::vector<mc::Scenario> generateScenarios(const payoff::PayoffNodePtr& _payoff) const {
        const auto newPayoff = payoff::applyMarket(_payoff, _market);
        const auto [symbols, fixingDates] = payoff::getSymbolsAndFixingDates(newPayoff);

        if (symbols.empty()) {
            throw std::invalid_argument("No symbol found in payoff");
        }

        return generateScenarios(fixingDates);
    }

    double priceFromScenarios(const payoff::PayoffNodePtr& _payoff, const std::vector<mc::Scenario>& scenarios) const {
        const auto newPayoff = payoff::applyMarket(_payoff, _market);
        double total = 0.0;
        int totalPaths = 0;
        for (auto& scenario : scenarios) {
            const auto sample = payoff::applyFixings(newPayoff, _market, scenario);
            total += sample.sum();
            totalPaths += sample.size();
        }
        return total / totalPaths;
    }

    double price(const payoff::PayoffNodePtr& _payoff) override {
        const auto scenarios = generateScenarios(_payoff);
        return priceFromScenarios(_payoff, scenarios);
    }

   private:
    const market::Market& _market;
    const mc::ProcessStateStepper<ProcessType> _processStateStepper;
    const int _nPaths;
    const double _maxDt;
    const int _nThreads;
    const RNGFactory _rngFactory;
    const int _seed;
};
}  // namespace pricer