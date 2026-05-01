"""
Prices a Locally Capped, Globally Floored Cliquet structure.

The annual coupon paid on December 2 of each year is:

    max( sum_{t=1}^{12} min(max(r_t, -1%), +1%), MinCoupon )

where r_t = S_t / S_{t-1} - 1 is the monthly return. The coupon is
therefore capped at 12% and floored at MinCoupon per year.

Prices across a range of MinCoupon values under two models:
  - Heston MC with Heston-Nandi parameters (v0=0.04, kappa=10, theta=0.04, xi=1, rho=-1)
  - Approximate local vol MC derived from the same Heston parameters

Reproduces Figure 10.1 of Gatheral (2006), "The Volatility Surface".

Assumptions: zero interest rates and dividends, daily MC steps (dt=1/252), 8 threads.
"""

from datetime import date
from functools import reduce
from pypricing.market import SimpleMarket
from pypricing.payoff import CashPayment, Fixing, Max, Min
from pypricing.pricer import ApproxLocalVolMCPricer, HestonMCPricer, HestonParams

PRICING_DATE = date(2002, 12, 2)
SYMBOL = "SX5E"
SPOT = 100.0
heston_params = HestonParams(v0=0.04, kappa=10.0, theta=0.04, xi=1.0, rho=-1.0)


def locally_capped_return(date1: date, date2: date):
    ret = Fixing(SYMBOL, date2) / Fixing(SYMBOL, date1) - 1.0
    return Min(Max(ret, -0.01), 0.01)


def annual_coupon(year: int, min_coupon: float):
    fixing_dates = [date(year - 1, 12, 2)]
    for m in range(1, 12):
        fixing_dates.append(date(year, m, 2))
    fixing_dates.append(date(year, 11, 25))
    coupons = [locally_capped_return(fixing_dates[i], fixing_dates[i + 1])
               for i in range(len(fixing_dates) - 1)]
    return Max(reduce(lambda a, b: a + b, coupons), min_coupon)


def make_payoff(min_coupon: float):
    return (CashPayment(annual_coupon(2003, min_coupon), date(2003, 12, 2)) +
            CashPayment(annual_coupon(2004, min_coupon), date(2004, 12, 2)) +
            CashPayment(annual_coupon(2005, min_coupon), date(2005, 12, 2)))


if __name__ == "__main__":
    mkt = SimpleMarket(PRICING_DATE, SYMBOL, SPOT, 0.0, 0.0, 0.2)

    heston_pricer = HestonMCPricer(mkt, heston_params, n_paths=1_000_000, max_dt=1.0 / 252.0, n_threads=8)
    local_vol_pricer = ApproxLocalVolMCPricer(mkt, heston_params, n_paths=1_000_000, max_dt=1.0 / 252.0, n_threads=8)

    first_payoff = make_payoff(0.0)
    heston_scenarios = heston_pricer.generate_scenarios(first_payoff)
    local_vol_scenarios = local_vol_pricer.generate_scenarios(first_payoff)

    print(f"{'MinCoupon':>10} | {'Heston':>8} | {'LocalVol':>8}")
    print("-" * 35)

    for i in range(13):  # minCoupon = 0.00, 0.01, ..., 0.12
        min_coupon = i * 0.01
        pf = make_payoff(min_coupon)
        heston_price = heston_pricer.price_from_scenarios(pf, heston_scenarios) / 3.0
        local_vol_price = local_vol_pricer.price_from_scenarios(pf, local_vol_scenarios) / 3.0
        print(f"{min_coupon:>10.2f} | {heston_price:>8.4f} | {local_vol_price:>8.4f}")
