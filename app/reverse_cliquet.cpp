/**
 * @file reverse_cliquet.cpp
 *
 * Prices a reverse cliquet on a basket of telecom stocks.
 *
 * A single final premium is paid at maturity (May 1, 2005):
 *
 *   P = max( 0, MaxCoupon + sum_{i=1}^{10} r_i )
 *
 * where each semi-annual return is:
 *
 *   r_i = (basket_i - basket_{i-1}) / basket_i = 1 - basket_{i-1} / basket_i
 *
 * Only negative returns reduce the sum; positive returns push it toward
 * MaxCoupon and reduce the payout. The maximum redemption is therefore
 * principal + MaxCoupon = 200%.
 *
 * This executable prices the structure across a range of MaxCoupon values
 * under the Heston model with Heston-Nandi parameters (v0=0.04, kappa=10,
 * theta=0.04, xi=1, rho=-1), reproducing Figure 10.3 of Gatheral (2006),
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

constexpr std::string SYMBOL = "BASKET";

ObservableNodePtr getReverseReturn(const Date earlier, const Date later) {
    // r_i = (basket_i - basket_{i-1}) / basket_i = 1 - basket_{i-1} / basket_i
    return 1.0 - fixing(SYMBOL, earlier) / fixing(SYMBOL, later);
}

ObservableNodePtr getAnnualCoupon(const std::vector<Date>& fixingDates, const double maxCoupon) {
    assert(fixingDates.size() == 11);

    std::vector<ObservableNodePtr> coupons;
    coupons.reserve(fixingDates.size());
    for (std::size_t i = 0; i < fixingDates.size() - 1; ++i) {
        const auto ret = getReverseReturn(fixingDates[i], fixingDates[i + 1]);
        coupons.push_back(min(0.0, ret));
    }

    return max(0.0, maxCoupon + sum(coupons));
}

std::vector<Date> getFixingDates() {
    using namespace std::chrono;

    std::vector<Date> dates;
    dates.reserve(11);

    for (int y = 2000; y <= 2005; ++y) {
        dates.emplace_back(year{y} / month{5} / day{1});
        if (y < 2005) {
            dates.emplace_back(year{y} / month{11} / day{1});
        }
    }

    return dates;
}

int main() {
    // Zero rates and dividends; Heston model does not rely on implied vol surface
    const Date pricingDate = makeDate(2000, 5, 1);
    constexpr SVIParams sviParams{.a = 0.0, .b = 0.0, .rho = 0.0, .m = 0.0, .sigma = 0.0};
    constexpr double spot = 100.0;
    SimpleMarket market{pricingDate, SYMBOL, spot, 0.0, 0.0, sviParams};

    // Heston model with Heston-Nandi parameters
    constexpr HestonParams hestonParams{
        .v0 = 0.04, .kappa = 10.0, .theta = 0.04, .xi = 1.0, .rho = -1.0};
    const HestonProcess heston{[&](const double) { return spot; }, hestonParams};
    const RNG rng(42);
    MCPricer hestonPricer{market, heston, 1'000'000, rng};

    const auto fixingDates = getFixingDates();

    constexpr int n = 11;  // maxCoupon = 0.0, 0.1, 0.2, ..., 1.0

    std::cout << "--------------------------------------\n";
    std::cout << "  MaxCoupon  |  Heston  |  LocalVol  \n";
    std::cout << "--------------------------------------\n";

    std::optional<Scenario> scenario;

    for (int i = 0; i < n; ++i) {
        const double maxCoupon = i * 0.1;

        const auto payoff =
            cashPayment(getAnnualCoupon(fixingDates, maxCoupon), makeDate(2005, 5, 1));

        if (i == 0) {
            scenario = hestonPricer.generateScenario(payoff, 1 / 252.0);
        }

        const double hestonPrice = hestonPricer.priceFromScenario(payoff, *scenario);

        // TODO implement LocalVolPricer
        std::cout << maxCoupon << " | " << hestonPrice << std::endl;
    }

    return 0;
}