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

    Scenario generateScenario(const payoff::PayoffNodePtr& _payoff, const double maxDt) {
        const auto newPayoff = payoff::applyMarket(_payoff, _market);
        const auto [symbols, fixingDates] = payoff::getSymbolsAndFixingDates(newPayoff);
        const auto timeGrid = mc::TimeGrid{fixingDates, _market.getPricingDate(), maxDt};

        if (symbols.empty()) {
            throw std::invalid_argument("No symbol found in payoff");
        }

        std::vector<Scenario> scenarios(_nThreads);
        std::vector<std::thread> threads;

        const int nPathsPerThread = _nPaths / _nThreads;

        for (int i = 0; i < _nThreads; i++) {
            threads.emplace_back([&, i]() {
                mc::RNG rng(i);
                scenarios[i] = _processStateStepper.run(timeGrid, nPathsPerThread, rng);
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

    double price(const payoff::PayoffNodePtr& _payoff, const double maxDt) {
        const auto scenario = generateScenario(_payoff, maxDt);
        return priceFromScenario(_payoff, scenario);
    }

    double price(const payoff::PayoffNodePtr& _payoff) override { return price(_payoff, 1 / 12.0); }

   private:
    const market::Market& _market;
    const mc::ProcessStateStepper<ProcessType> _processStateStepper;
    const int _nPaths;
    const int _nThreads;
    static Sample concat(const Sample& a, const Sample& b) {
        Sample result(a.size() + b.size());
        std::copy(std::begin(a), std::end(a), std::begin(result));
        std::copy(std::begin(b), std::end(b), std::begin(result) + a.size());
        return result;
    }
    static Scenario mergeScenarios(const std::vector<Scenario>& subScenarios) {
        if (subScenarios.empty())
            return {};

        Scenario merged = subScenarios[0];  // copy first

        for (std::size_t i = 1; i < subScenarios.size(); i++) {
            for (auto& [date, sample] : merged) {
                sample = concat(sample, subScenarios[i].at(date));
            }
        }

        return merged;
    }
};

}  // namespace pricer