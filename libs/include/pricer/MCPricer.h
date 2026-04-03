#pragma once

#include <ranges>
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
using RNGFactory = std::function<mc::RNG(int seed)>;

template <typename ProcessType>
class MCPricer final : public PayoffPricer {
   public:
    MCPricer(
        const market::Market& market, const ProcessType& process, const int nPaths,
        const double maxDt = 1.0 / 12.0, const int nThreads = 1,
        const RNGFactory& rngFactory = [](const int seed) { return mc::RNG(seed); },
        const int seed = 0)
        : _market(market),
          _processStateStepper(mc::ProcessStateStepper<ProcessType>(process)),
          _nPaths(nPaths),
          _maxDt(maxDt),
          _nThreads(nThreads),
          _rngFactory(rngFactory),
          _seed(seed) {}

    ~MCPricer() override = default;

    Scenario generateScenario(const payoff::PayoffNodePtr& _payoff) {
        const auto newPayoff = payoff::applyMarket(_payoff, _market);
        const auto [symbols, fixingDates] = payoff::getSymbolsAndFixingDates(newPayoff);

        if (symbols.empty()) {
            throw std::invalid_argument("No symbol found in payoff");
        }

        const auto timeGrid = mc::TimeGrid{fixingDates, _market.getPricingDate(), _maxDt};

        if (_nThreads <= 1) {
            return _processStateStepper.run(timeGrid, _nPaths, _rngFactory(_seed));
        }

        std::vector<Scenario> scenarios(_nThreads);
        std::vector<std::thread> threads;

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

        return mergeScenarios(scenarios);
    }

    double priceFromScenario(const payoff::PayoffNodePtr& _payoff, const Scenario& scenario) const {
        const auto newPayoff = payoff::applyMarket(_payoff, _market);
        const auto sample = payoff::applyFixings(newPayoff, _market, scenario);
        return sample.sum() / sample.size();
    }

    double price(const payoff::PayoffNodePtr& _payoff) override {
        const auto scenario = generateScenario(_payoff);
        return priceFromScenario(_payoff, scenario);
    }

   private:
    const market::Market& _market;
    const mc::ProcessStateStepper<ProcessType> _processStateStepper;
    const int _nPaths;
    const double _maxDt;
    const int _nThreads;
    const RNGFactory _rngFactory;
    const int _seed;

    static Sample concat(const Sample& a, const Sample& b) {
        Sample result(a.size() + b.size());
        std::copy(std::begin(a), std::end(a), std::begin(result));
        std::copy(std::begin(b), std::end(b), std::begin(result) + a.size());
        return result;
    }
    static Scenario mergeScenarios(const std::vector<Scenario>& scenarios) {
        if (scenarios.empty()) {
            return {};
        }

        Scenario merged;
        for (const auto& date : scenarios[0] | std::views::keys) {
            Sample combined = scenarios[0].at(date);
            for (std::size_t i = 1; i < scenarios.size(); i++) {
                combined = concat(combined, scenarios[i].at(date));
            }
            merged[date] = std::move(combined);
        }
        return merged;
    }
};
}  // namespace pricer