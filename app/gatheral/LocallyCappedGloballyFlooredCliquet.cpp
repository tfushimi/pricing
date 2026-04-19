/**
 * @file LocallyCappedGloballyFlooredCliquet.cpp
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

#include "HestonNandi.h"
#include "common/Date.h"
#include "common/TableUtils.h"
#include "market/SimpleMarket.h"
#include "payoff/Observable.h"
#include "payoff/Payoff.h"
#include "payoff/Transforms.h"
#include "pricer/MCPricerWrapper.h"

using namespace calendar;
using namespace market;
using namespace payoff;
using namespace pricer;
using namespace vol;

const Date pricingDate = makeDate(2002, 12, 2);
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

PayoffNodePtr makePayoff(const double minCoupon) {

    const auto fixingDates1 = getFixingDates(2003);
    const auto fixingDates2 = getFixingDates(2004);
    const auto fixingDates3 = getFixingDates(2005);
    return cashPayment(getAnnualCoupon(fixingDates1, minCoupon), makeDate(2003, 12, 2)) +
           cashPayment(getAnnualCoupon(fixingDates2, minCoupon), makeDate(2004, 12, 2)) +
           cashPayment(getAnnualCoupon(fixingDates3, minCoupon), makeDate(2005, 12, 2));
}

int main() {
    // Zero rates and dividends; Heston model does not rely on implied vol surface
    SimpleMarket market{pricingDate, SYMBOL, SPOT, 0.0, 0.0, 0.2};

    HestonMCPricer hestonPricer{market, hestonParams, 1'000'000, 1.0 / 252.0, 8};
    ApproxLocalVolMCPricer localVolPricer{market, hestonParams, 1'000'000, 1.0 / 252.0, 8};

    constexpr int n = 13;  // minCoupon = 0.00, 0.01, ..., 0.12

    const auto firstPayoff = makePayoff(0.0);
    const auto hestonScenarios = hestonPricer.generateScenarios(firstPayoff);
    const auto localVolScenarios = localVolPricer.generateScenarios(firstPayoff);

    std::vector<double> minCoupons, hestonPrices, localVolPrices;

    for (int i = 0; i < n; ++i) {
        const double minCoupon = i * 0.01;

        const auto payoff = makePayoff(minCoupon);

        // Divide by 3: average annual coupon across the 3 coupon years (2003, 2004, 2005)
        const double hestonPrice = hestonPricer.priceFromScenarios(payoff, hestonScenarios) / 3.0;
        const double localVolPrice =
            localVolPricer.priceFromScenarios(payoff, localVolScenarios) / 3.0;

        minCoupons.push_back(minCoupon);
        hestonPrices.push_back(hestonPrice);
        localVolPrices.push_back(localVolPrice);
    }

    printTable({"MinCoupon", "Heston", "LocalVol"}, {minCoupons, hestonPrices, localVolPrices});
    writeCsv("figure_10_1.csv", {"MinCoupon", "Heston", "LocalVol"},
             {minCoupons, hestonPrices, localVolPrices});

    return 0;
}