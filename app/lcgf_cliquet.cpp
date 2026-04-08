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
#include <cassert>

#include <iostream>

#include "common/Date.h"
#include "HestonNandi.h"
#include "market/SVI.h"
#include "market/SimpleMarket.h"
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

constexpr std::string SYMBOL = "SX5E";

ObservableNodePtr getLocallyCappedReturn(const Date fixingDate1, const Date fixingDate2) {
    const auto ret = fixing(SYMBOL, fixingDate2) / fixing(SYMBOL, fixingDate1) - 1.0;
    return min(max(ret, -0.01), 0.01);
}

ObservableNodePtr getAnnualCoupon(const std::vector<Date>& fixingDates, const double minCoupon) {
    assert(fixingDates.size() == 13);

    std::vector<ObservableNodePtr> coupons;
    coupons.reserve(fixingDates.size());
    for (std::size_t i = 0; i < fixingDates.size() - 1; ++i) {
        coupons.push_back(getLocallyCappedReturn(fixingDates[i], fixingDates[i + 1]));
    }

    return max(sum(coupons), minCoupon);
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
    SimpleMarket market{pricingDate, SYMBOL, SPOT, 0.0, 0.0, sviParams};

    MCPricer hestonPricer{market, heston, 1'000'000, 1.0 / 252.0, 8};
    MCPricer localVolPricer{market, localVol, 1'000'000, 1.0 / 252.0, 8};

    const auto fixingDates1 = getFixingDates(2003);
    const auto fixingDates2 = getFixingDates(2004);
    const auto fixingDates3 = getFixingDates(2005);

    std::vector<Date> fixingDates;
    fixingDates.reserve(fixingDates1.size() + fixingDates2.size() + fixingDates3.size());
    fixingDates.insert(fixingDates.end(), fixingDates1.begin(), fixingDates1.end());
    fixingDates.insert(fixingDates.end(), fixingDates2.begin(), fixingDates2.end());
    fixingDates.insert(fixingDates.end(), fixingDates3.begin(), fixingDates3.end());

    constexpr int n = 13;  // minCoupon = 0.00, 0.01, ..., 0.12

    const auto hestonScenarios = hestonPricer.generateScenarios(fixingDates);
    const auto localVolScenarios = localVolPricer.generateScenarios(fixingDates);

    std::vector<double> minCoupons, hestonPrices, localVolPrices;

    for (int i = 0; i < n; ++i) {
        const double minCoupon = i * 0.01;

        const auto payoff1 =
            cashPayment(getAnnualCoupon(fixingDates1, minCoupon), makeDate(2003, 12, 2));
        const auto payoff2 =
            cashPayment(getAnnualCoupon(fixingDates2, minCoupon), makeDate(2004, 12, 2));
        const auto payoff3 =
            cashPayment(getAnnualCoupon(fixingDates3, minCoupon), makeDate(2005, 12, 2));
        const auto payoff = payoff1 + payoff2 + payoff3;

        // Divide by 3: average annual coupon across the 3 coupon years (2003, 2004, 2005)
        const double hestonPrice = hestonPricer.priceFromScenarios(payoff, hestonScenarios) / 3.0;
        const double localVolPrice = localVolPricer.priceFromScenarios(payoff, localVolScenarios) / 3.0;

        minCoupons.push_back(minCoupon);
        hestonPrices.push_back(hestonPrice);
        localVolPrices.push_back(localVolPrice);
    }

    printTable("MinCoupon", {"Heston", "LocalVol"}, minCoupons, {hestonPrices, localVolPrices});

    return 0;
}