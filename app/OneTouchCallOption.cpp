/**
 * @file OneTouchCallOption.cpp
 *
 * Prices a one-touch call option: pays $1 if the underlying ever reaches
 * the barrier B during [0, T], zero otherwise.
 *
 * The running maximum is approximated by daily fixings:
 *
 *   payoff = 1{ max_{t in {0, dt, 2dt, ..., T}} S_t / S_0 >= B }
 *
 * The local volatility model prices this higher than Heston. The fundamental
 * reason is the conditional distribution of future spot: with rho=-1, Heston
 * assigns lower instantaneous vol to paths where spot has risen (negative
 * spot-vol correlation), suppressing the probability of sustained upward moves
 * that breach the barrier. Local vol does not capture this effect in the
 * forward sense — conditioning on a future spot level leaves little residual
 * vol uncertainty, so upward paths retain higher vol than under Heston.
 *
 * This executable reproduces Figure 9.4 of Gatheral (2006), "The Volatility
 * Surface", pricing the one-touch call as a function of barrier level under
 * the Heston model with Heston-Nandi parameters (v0=0.04, kappa=10,
 * theta=0.04, xi=1, rho=-1).
 *
 * Assumptions: zero interest rates and dividends, T=1 year, daily MC steps
 * (dt=1/252), barrier expressed as a multiple of spot (B=1.0 means at-the-money).
 */

#include <iostream>

#include "FlatMarket.h"
#include "HestonNandi.h"
#include "common/Date.h"
#include "common/TableUtils.h"
#include "mc/Process.h"
#include "payoff/Observable.h"
#include "payoff/Payoff.h"
#include "payoff/Transforms.h"
#include "pricer/MCPricer.h"

using namespace calendar;
using namespace market;
using namespace payoff;
using namespace pricer;
using namespace vol;
using namespace mc;

constexpr std::string SYMBOL = "Underlier";

ObservableNodePtr getOneTouchCall(const std::vector<Date>& fixingDates, const double barrier) {
    std::vector<ObservableNodePtr> fixings;
    fixings.reserve(fixingDates.size());
    for (const auto fixingDate : fixingDates) {
        fixings.push_back(fixing(SYMBOL, fixingDate) / fixing(SYMBOL, fixingDates[0]));
    }
    return max(fixings) >= barrier;
}

std::vector<Date> getDailyFixingDates(const Date& start, const Date& end) {
    std::vector<Date> dates;
    auto d = std::chrono::sys_days{start};
    const auto last = std::chrono::sys_days{end};
    while (d <= last) {
        const auto ymd = std::chrono::year_month_day{d};
        const auto wd = std::chrono::weekday{d};
        if (wd != std::chrono::Saturday && wd != std::chrono::Sunday) {
            dates.push_back(ymd);
        }
        d += std::chrono::days{1};
    }
    return dates;
}

int main() {
    // Zero rates and dividends; Heston model does not rely on implied vol surface
    const Date pricingDate = makeDate(2000, 1, 1);
    auto market = makeFlatMarket(pricingDate, SYMBOL, SPOT);

    MCPricer hestonPricer{market, heston, 1'000'000, 1.0 / 252.0, 8};
    MCPricer localVolPricer{market, localVol, 1'000'000, 1.0 / 252.0, 8};

    const auto fixingDates = getDailyFixingDates(pricingDate, makeDate(2001, 1, 1));

    constexpr int n = 40;  // barrier = 1.0, 1.01, 1.02, ..., 1.4

    const auto hestonScenarios = hestonPricer.generateScenarios(fixingDates);
    const auto localVolScenarios = localVolPricer.generateScenarios(fixingDates);

    std::vector<double> barriers, hestonPrices, localVolPrices;

    for (int i = 0; i < n; ++i) {
        const double barrier = 1 + i * 0.01;

        const auto payoff =
            cashPayment(getOneTouchCall(fixingDates, barrier), makeDate(2001, 1, 1));

        const double hestonPrice = hestonPricer.priceFromScenarios(payoff, hestonScenarios);
        const double localVolPrice = localVolPricer.priceFromScenarios(payoff, localVolScenarios);

        barriers.push_back(barrier);
        hestonPrices.push_back(hestonPrice);
        localVolPrices.push_back(localVolPrice);
    }

    printTable({"Barrier", "Heston", "LocalVol"}, {barriers, hestonPrices, localVolPrices});
    writeCsv("figure_9_4.csv", {"Barrier", "Heston", "LocalVol"},
             {barriers, hestonPrices, localVolPrices});

    return 0;
}