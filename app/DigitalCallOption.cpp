/**
 * @file DigitalCallOption.cpp
 *
 * Prices a digital call option (pays 1 if S_T >= K, else 0) across a range
 * of strikes under two models:
 *
 *   - Heston: analytical price via characteristic function inversion using Heston-Nandi parameters
 *             (v0=0.04, kappa=10, theta=0.04, xi=1, rho=-1).
 *
 *   - Local Vol: Monte Carlo with the Dupire local vol surface derived
 *                from the Heston-Nandi SVI fit, 1M paths, daily steps.
 *
 * Strikes run from K/S = 1.00 to 1.39 in steps of 0.01.
 * Zero interest rates and dividends throughout.
 * Pricing date: 2000-01-01, expiry: 2001-01-01 (1 year).
 */

#include "HestonNandi.h"
#include "common/Date.h"
#include "common/TableUtils.h"
#include "market/SVI.h"
#include "market/SimpleMarket.h"
#include "mc/Process.h"
#include "payoff/Observable.h"
#include "payoff/Payoff.h"
#include "payoff/Transforms.h"
#include "pricer/HestonPricer.h"
#include "pricer/MCPricer.h"

using namespace calendar;
using namespace market;
using namespace payoff;
using namespace pricer;
using namespace vol;
using namespace mc;

constexpr std::string SYMBOL = "Underlier";

int main() {
    // Zero rates and dividends; Heston model does not rely on implied vol surface
    const Date pricingDate = makeDate(2000, 1, 1);
    constexpr SVIParams sviParams{.a = 0.0, .b = 0.0, .rho = 0.0, .m = 0.0, .sigma = 1e-10};
    SimpleMarket market{pricingDate, SYMBOL, SPOT, 0.0, 0.0, sviParams};

    HestonPricer hestonPricer{market, hestonParams};
    MCPricer localVolPricer{market, localVol, 1'000'000, 1.0 / 252.0, 8};

    const auto fixingDates = {makeDate(2001, 1, 1)};

    constexpr int n = 40;  // barrier = 1.0, 1.01, 1.02, ..., 1.4

    const auto localVolScenarios = localVolPricer.generateScenarios(fixingDates);

    std::vector<double> barriers, hestonPrices, localVolPrices;

    for (int i = 0; i < n; ++i) {
        const double barrier = 1 + i * 0.01;

        const auto payoff = cashPayment(fixing(SYMBOL, *fixingDates.begin()) / SPOT >= barrier,
                                        makeDate(2001, 1, 1));

        const double hestonPrice = hestonPricer.price(payoff);
        const double localVolPrice = localVolPricer.priceFromScenarios(payoff, localVolScenarios);

        barriers.push_back(barrier);
        hestonPrices.push_back(hestonPrice);
        localVolPrices.push_back(localVolPrice);
    }

    printTable({"Barrier", "Heston", "LocalVol"}, {barriers, hestonPrices, localVolPrices});
    writeCsv("figure_9_3.csv", {"barrier", "Heston", "LocalVol"},
             {barriers, hestonPrices, localVolPrices});
    return 0;
}