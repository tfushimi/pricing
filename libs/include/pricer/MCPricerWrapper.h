#pragma once

#include "common/Types.h"
#include "mc/Process.h"
#include "numerics/RNG.h"
#include "payoff/Transforms.h"
#include "pricer/LocalVolFormula.h"
#include "pricer/MCPricer.h"

namespace pricer {

using Scenarios = std::vector<Scenario>;

inline const RNGFactory defaultRNGFactory = [](const int seed) {
    return std::make_unique<numerics::rng::NormalRNG>(seed);
};

class MCPricerBase {
   public:
    virtual ~MCPricerBase() = default;
    virtual Scenarios generateScenarios(const payoff::PayoffNodePtr& pf) const = 0;

    double priceFromScenarios(const payoff::PayoffNodePtr& pf, const Scenarios& scenarios) const {
        return pricer::priceFromScenarios(_market, pf, scenarios);
    }

    double price(const payoff::PayoffNodePtr& pf) const {
        return priceFromScenarios(pf, generateScenarios(pf));
    }

   protected:
    MCPricerBase(const market::Market& market, const int nPaths, const double maxDt,
                 const int nThreads, const int seed)
        : _market(market), _nPaths(nPaths), _maxDt(maxDt), _nThreads(nThreads), _seed(seed) {}

    std::string extractSymbol(const payoff::PayoffNodePtr& pf) const {
        const auto applied = payoff::applyMarket(pf, _market);
        const auto [symbols, _] = payoff::getSymbolsAndFixingDates(applied);
        if (symbols.empty())
            throw std::invalid_argument("No symbol found in payoff");
        if (symbols.size() > 1)
            throw std::invalid_argument("MCPricer supports single-symbol payoffs only");
        return *symbols.begin();
    }

    const market::Market& _market;
    int _nPaths;
    double _maxDt;
    int _nThreads;
    int _seed;
};

class GBMMCPricer final : public MCPricerBase {
   public:
    GBMMCPricer(const market::Market& market, const double vol, const int nPaths,
                const double maxDt = 1.0 / 12.0, const int nThreads = 1, const int seed = 0)
        : MCPricerBase(market, nPaths, maxDt, nThreads, seed), _vol(vol) {}

    Scenarios generateScenarios(const payoff::PayoffNodePtr& pf) const override {
        const std::string symbol = extractSymbol(pf);
        const auto forward = [&](const double t) { return _market.getForward(symbol, t); };
        const MCPricer pricer{
            _market, mc::GBMProcess{forward, _vol}, _nPaths, _maxDt, _nThreads, defaultRNGFactory,
            _seed};
        return pricer.generateScenarios(pf);
    }

   private:
    double _vol;
};

class HestonMCPricer final : public MCPricerBase {
   public:
    HestonMCPricer(const market::Market& market, const HestonParams& params, const int nPaths,
                   const double maxDt = 1.0 / 12.0, const int nThreads = 1, const int seed = 0)
        : MCPricerBase(market, nPaths, maxDt, nThreads, seed), _params(params) {}

    Scenarios generateScenarios(const payoff::PayoffNodePtr& pf) const override {
        const std::string symbol = extractSymbol(pf);
        const auto forward = [&](const double t) { return _market.getForward(symbol, t); };
        const MCPricer pricer{_market,   mc::HestonProcess{forward, _params},
                              _nPaths,   _maxDt,
                              _nThreads, defaultRNGFactory,
                              _seed};
        return pricer.generateScenarios(pf);
    }

   private:
    HestonParams _params;
};

class ApproxLocalVolMCPricer final : public MCPricerBase {
   public:
    ApproxLocalVolMCPricer(const market::Market& market, const HestonParams& params,
                           const int nPaths, const double maxDt = 1.0 / 12.0,
                           const int nThreads = 1, const int seed = 0)
        : MCPricerBase(market, nPaths, maxDt, nThreads, seed), _params(params) {}

    Scenarios generateScenarios(const payoff::PayoffNodePtr& pf) const override {
        const std::string symbol = extractSymbol(pf);
        const auto forward = [&](const double t) { return _market.getForward(symbol, t); };
        const auto localVol = [p = _params](const Sample& logZ, const double t) {
            return approximateLocalVol(p, logZ, t);
        };
        const MCPricer pricer{_market,   mc::LocalVolProcess{forward, localVol},
                              _nPaths,   _maxDt,
                              _nThreads, defaultRNGFactory,
                              _seed};
        return pricer.generateScenarios(pf);
    }

   private:
    HestonParams _params;
};

}  // namespace pricer
