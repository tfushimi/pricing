"""
Prices the Napoleon reverse cliquet structure described in Gatheral (2006),
"The Volatility Surface", Chapter 10.

The structure pays an annual coupon on December 1 of each year 2003-2005:

    coupon_i = max( 0, MaxCoupon + r̃_i )

where r̃_i is the most negative monthly return over year i:

    r̃_i = min_{t in year_i} ( S_t / S_{t-1} - 1 )

The investor is implicitly short a put on the worst monthly return: in calm years the
coupon approaches MaxCoupon; in crash years it is floored at zero.

Output is the average annual coupon (total PV / 3) since interest rates are zero.

Note: the actual Mediobanca structure averages r̃_i across three indices
(SPX, STOXX50, NIKKEI225). We price the single-index version for simplicity,
which overestimates the expected coupon relative to the book's Figure 10.5.

Reproduces Figure 10.5 of Gatheral (2006), "The Volatility Surface".

Assumptions: zero interest rates and dividends, daily MC steps (dt=1/252).
"""

from datetime import date

from pypricing.market import SimpleMarket
from pypricing.payoff import CashPayment, Fixing, Max, Min
from pypricing.pricer import ApproxLocalVolMCPricer, HestonMCPricer, HestonParams

PRICING_DATE = date(2002, 12, 1)
SYMBOL = "SX5E"
SPOT = 100.0
heston_params = HestonParams(v0=0.04, kappa=10.0, theta=0.04, xi=1.0, rho=-1.0)


def annual_coupon(year: int, max_coupon: float):
    fixing_dates = [date(year - 1, 12, 1)] + [date(year, m, 1) for m in range(1, 13)]
    returns = [Min(Fixing(SYMBOL, fixing_dates[i + 1]) / Fixing(SYMBOL, fixing_dates[i]) - 1.0, 0.0)
               for i in range(len(fixing_dates) - 1)]
    return Max(max_coupon + Min(returns), 0.0)


def make_payoff(max_coupon: float):
    return (CashPayment(annual_coupon(2003, max_coupon), date(2003, 12, 1)) +
            CashPayment(annual_coupon(2004, max_coupon), date(2004, 12, 1)) +
            CashPayment(annual_coupon(2005, max_coupon), date(2005, 12, 1)))


if __name__ == "__main__":
    mkt = SimpleMarket(PRICING_DATE, SYMBOL, SPOT, 0.0, 0.0, 0.2)

    heston_pricer = HestonMCPricer(mkt, heston_params, n_paths=1_000_000,
                                   max_dt=1.0 / 252.0, n_threads=8)
    local_vol_pricer = ApproxLocalVolMCPricer(mkt, heston_params, n_paths=1_000_000,
                                              max_dt=1.0 / 252.0, n_threads=8)

    first_payoff = make_payoff(0.0)
    heston_scenarios = heston_pricer.generate_scenarios(first_payoff)
    local_vol_scenarios = local_vol_pricer.generate_scenarios(first_payoff)

    print(f"{'MaxCoupon':>10} | {'Heston':>8} | {'LocalVol':>8}")
    print("-" * 35)

    for i in range(16):  # maxCoupon = 0.00, 0.01, ..., 0.15
        max_coupon = i * 0.01
        pf = make_payoff(max_coupon)
        heston_price = heston_pricer.price_from_scenarios(pf, heston_scenarios) / 3.0
        local_vol_price = local_vol_pricer.price_from_scenarios(pf, local_vol_scenarios) / 3.0
        print(f"{max_coupon:>10.2f} | {heston_price:>8.4f} | {local_vol_price:>8.4f}")
