"""
Prices a reverse cliquet on a basket of telecom stocks.

A single final premium is paid at maturity (May 1, 2005):

    P = max( 0, MaxCoupon + sum_{i=1}^{10} r_i )

where each semi-annual return is:

    r_i = (basket_i - basket_{i-1}) / basket_i = 1 - basket_{i-1} / basket_i

Only negative returns reduce the sum; positive returns push it toward MaxCoupon and
reduce the payout. The maximum redemption is therefore principal + MaxCoupon = 200%.

Reproduces Figure 10.3 of Gatheral (2006), "The Volatility Surface".

Assumptions: zero interest rates and dividends, daily MC steps (dt=1/252).
"""

from datetime import date
from functools import reduce

from pypricing.market import SimpleMarket
from pypricing.payoff import CashPayment, Fixing, Max, Min
from pypricing.pricer import ApproxLocalVolMCPricer, HestonMCPricer, HestonParams

PRICING_DATE = date(2000, 5, 1)
SETTLEMENT_DATE = date(2005, 5, 1)
SYMBOL = "BASKET"
SPOT = 100.0
heston_params = HestonParams(v0=0.04, kappa=10.0, theta=0.04, xi=1.0, rho=-1.0)


def make_payoff(max_coupon: float):
    dates = [date(y, m, 1) for y in range(2000, 2006) for m in ([5, 11] if y < 2005 else [5])]
    returns = [Min(1.0 - Fixing(SYMBOL, dates[i]) / Fixing(SYMBOL, dates[i + 1]), 0.0)
               for i in range(len(dates) - 1)]
    total = reduce(lambda a, b: a + b, returns)
    return CashPayment(Max(max_coupon + total, 0.0), SETTLEMENT_DATE)


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

    for i in range(11):  # maxCoupon = 0.0, 0.1, ..., 1.0
        max_coupon = i * 0.1
        pf = make_payoff(max_coupon)
        heston_price = heston_pricer.price_from_scenarios(pf, heston_scenarios)
        local_vol_price = local_vol_pricer.price_from_scenarios(pf, local_vol_scenarios)
        print(f"{max_coupon:>10.2f} | {heston_price:>8.4f} | {local_vol_price:>8.4f}")
