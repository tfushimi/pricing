/**
 * @file lcgf_cliquet.cpp
 *
 * Prices a Locally Capped, Globally Floored Cliquet structure.
 *
 * The annual coupon paid on December 2 of each year is:
 *
 *   max( sum_{t=1}^{12} min(max(r_t, -1%), +1%), MinCoupon )
 *
 * where r_t = S_t / S_{t-1} - 1 is the monthly return. The coupon is
 * therefore capped at 12% and floored at MinCoupon per year.
 *
 * This executable prices the structure across a range of MinCoupon values
 * under the Heston model with Heston-Nandi parameters (v0=0.04, kappa=10,
 * theta=0.04, xi=1, rho=-1), reproducing Figure 10.1 of Gatheral (2006),
 * "The Volatility Surface".
 *
 * Assumptions: zero interest rates and dividends, daily MC steps (dt=1/252).
 */
#include <assert.h>

#include <iostream>
#include <optional>

#include "market/SVI.h"
#include "market/SimpleMarket.h"
#include "mc/Process.h"
#include "payoff/Observable.h"
#include "payoff/Payoff.h"
#include "payoff/Transforms.h"
#include "pricer/MCPricer.h"

using namespace market;
using namespace payoff;
using namespace pricer;
using namespace vol;
using namespace mc;

constexpr std::string SYMBOL = "SX5E";

ObservableNodePtr getLocallyCappedReturn(const Date fixingDate1, const Date fixingDate2) {
    const auto ret = fixing(SYMBOL, fixingDate2) / fixing(SYMBOL, fixingDate1) - 1.0;
    return min(max(ret, -0.01), 0.01);
}

ObservableNodePtr getCoupon(const std::vector<Date>& fixingDates, const double minCoupon) {
    assert(fixingDates.size() == 13);

    auto coupon = constant(0.0);
    for (std::size_t i = 0; i < fixingDates.size() - 1; ++i) {
        coupon += getLocallyCappedReturn(fixingDates[i], fixingDates[i + 1]);
    }

    return max(coupon, minCoupon);
}

std::vector<Date> getFixingDates(const int y) {
    using namespace std::chrono;

    std::vector<Date> fixingDates;
    fixingDates.reserve(13);

    // Dec 2 of prior year — anchor fixing
    fixingDates.emplace_back(year{y - 1} / month{12} / day{2});

    // Jan 2 through Nov 2 of coupon year
    for (unsigned int m = 1; m <= 11; ++m) {
        fixingDates.emplace_back(year{y} / month{m} / day{2});
    }

    // Nov 25 — final fixing, one week before Dec 2 payment
    fixingDates.emplace_back(year{y} / month{11} / day{25});

    return fixingDates;
}

int main() {
    // Zero rates and dividends; Heston model does not rely on implied vol surface
    const Date pricingDate = makeDate(2002, 12, 2);
    constexpr SVIParams sviParams{.a = 0.0, .b = 0.0, .rho = 0.0, .m = 0.0, .sigma = 0.0};
    constexpr double spot = 100.0;
    SimpleMarket market{pricingDate, SYMBOL, spot, 0.0, 0.0, sviParams};

    // Heston model with Heston-Nandi parameters
    constexpr HestonParams hestonParams{
        .v0 = 0.04, .kappa = 10.0, .theta = 0.04, .xi = 1.0, .rho = -1.0};
    const HestonProcess heston{[&](const double) { return spot; }, hestonParams};
    const RNG rng(42);
    MCPricer hestonPricer{market, heston, 1'000'000, rng};

    const auto fixingDates1 = getFixingDates(2003);
    const auto fixingDates2 = getFixingDates(2004);
    const auto fixingDates3 = getFixingDates(2005);

    constexpr int n = 13;  // minCoupon = 0.00, 0.01, ..., 0.12

    std::cout << "--------------------------------------\n";
    std::cout << "  MinCoupon  |  Heston  |  LocalVol  \n";
    std::cout << "--------------------------------------\n";

    std::optional<Scenario> scenario;

    for (int i = 0; i < n; ++i) {
        const double minCoupon = i * 0.01;

        const auto payoff1 = cashPayment(getCoupon(fixingDates1, minCoupon), makeDate(2003, 12, 2));
        const auto payoff2 = cashPayment(getCoupon(fixingDates2, minCoupon), makeDate(2004, 12, 2));
        const auto payoff3 = cashPayment(getCoupon(fixingDates3, minCoupon), makeDate(2005, 12, 2));
        const auto payoff = payoff1 + payoff2 + payoff3;

        if (i == 0) {
            scenario = hestonPricer.generateScenario(payoff, 1 / 252.0);
        }

        // Divide by 3: average annual coupon across the 3 coupon years (2003, 2004, 2005)
        const double hestonPrice = hestonPricer.priceFromScenario(payoff, *scenario) / 3.0;

        // TODO implement LocalVolPricer
        std::cout << minCoupon << " | " << hestonPrice << std::endl;
    }

    return 0;
}