/**
 * @file napoleon.cpp
 *
 * Prices the Napoleon reverse cliquet structure described in Gatheral (2006),
 * "The Volatility Surface", Chapter 10.
 *
 * The structure pays an annual coupon on December 1 of each year 2003-2005:
 *
 *   coupon_i = max( 0, MaxCoupon + r̃_i )
 *
 * where MaxCoupon = 10% and r̃_i is the most negative monthly return over
 * year i:
 *
 *   r̃_i = min_{t in year_i} ( S_t / S_{t-1} - 1 )
 *
 * The investor is implicitly short a put on the worst monthly return: in calm
 * years the coupon approaches MaxCoupon; in crash years it is floored at zero.
 *
 * This executable reproduces Figure 10.5 of Gatheral, pricing the expected
 * annual coupon across a range of MaxCoupon values under the Heston model
 * with Heston-Nandi parameters (v0=0.04, kappa=10, theta=0.04, xi=1, rho=-1).
 * The full structure spans three coupon years; output is the average annual
 * coupon (total PV / 3) since interest rates are zero.
 *
 * Note: the actual Mediobanca structure averages r̃_i across three indices
 * (SPX, STOXX50, NIKKEI225). We price the single-index version for simplicity,
 * which overestimates the expected coupon relative to the book's Figure 10.5.
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

ObservableNodePtr getReturn(const Date earlier, const Date later) {
    return min(0.0, fixing(SYMBOL, later) / fixing(SYMBOL, earlier) - 1.0);
}

ObservableNodePtr getAnnualCoupon(const std::vector<Date>& fixingDates, const double maxCoupon) {
    assert(fixingDates.size() == 13);

    std::vector<ObservableNodePtr> returns;
    returns.reserve(12);
    for (std::size_t i = 0; i < fixingDates.size() - 1; ++i) {
        returns.push_back(getReturn(fixingDates[i], fixingDates[i + 1]));
    }

    return max(0.0, constant(maxCoupon) + min(returns));
}

std::vector<Date> getFixingDates(const int couponYear) {
    using namespace std::chrono;

    std::vector<Date> dates;
    dates.reserve(13);

    // Month 0: December of the previous year (strike-set date)
    dates.emplace_back(year{couponYear - 1} / December / day{1});

    // Months 1-12: January through December of couponYear
    for (int m = 1; m <= 12; ++m) {
        dates.emplace_back(year{couponYear} / month{static_cast<unsigned>(m)} / day{1});
    }

    return dates;
}

int main() {
    // Zero rates and dividends; Heston model does not rely on implied vol surface
    const Date pricingDate = makeDate(2002, 12, 1);
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

    constexpr int n = 16;  // maxCoupon = 0.00, 0.01, ..., 0.15

    std::cout << "--------------------------------------\n";
    std::cout << "  MaxCoupon  |  Heston  |  LocalVol  \n";
    std::cout << "--------------------------------------\n";

    const auto hestonScenarios = hestonPricer.generateScenarios(fixingDates);
    const auto localVolScenarios = localVolPricer.generateScenarios(fixingDates);

    for (int i = 0; i < n; ++i) {
        const double maxCoupon = i * 0.01;

        const auto payoff1 =
            cashPayment(getAnnualCoupon(fixingDates1, maxCoupon), makeDate(2003, 12, 1));
        const auto payoff2 =
            cashPayment(getAnnualCoupon(fixingDates2, maxCoupon), makeDate(2004, 12, 1));
        const auto payoff3 =
            cashPayment(getAnnualCoupon(fixingDates3, maxCoupon), makeDate(2005, 12, 1));
        const auto payoff = payoff1 + payoff2 + payoff3;

        // Divide by 3: average annual coupon across the 3 coupon years (2003, 2004, 2005)
        const double hestonPrice = hestonPricer.priceFromScenarios(payoff, hestonScenarios) / 3.0;
        const double localVolPrice = localVolPricer.priceFromScenarios(payoff, localVolScenarios) / 3.0;

        std::cout << maxCoupon << " | " << hestonPrice << " | "  << localVolPrice << std::endl;
    }

    return 0;
}