"""
Prices a digital call option (pays 1 if S_T >= K, else 0) across a range of strikes
under two models:

  - Heston: analytical price via characteristic function inversion using Heston-Nandi
            parameters (v0=0.04, kappa=10, theta=0.04, xi=1, rho=-1).

  - Local Vol: Monte Carlo with the approximate local vol surface derived from the same
               Heston parameters, 1M paths, daily steps.

Strikes run from K/S = 1.00 to 1.39 in steps of 0.01.
Zero interest rates and dividends throughout.
Pricing date: 2000-01-01, expiry: 2001-01-01 (1 year).

Reproduces Figure 9.3 of Gatheral (2006), "The Volatility Surface".
"""

from datetime import date

from pypricing.market import SimpleMarket
from pypricing.payoff import CashPayment, Fixing
from pypricing.pricer import ApproxLocalVolMCPricer, HestonParams, HestonPricer

PRICING_DATE = date(2000, 1, 1)
EXPIRY_DATE = date(2001, 1, 1)
SYMBOL = "Underlier"
SPOT = 100.0
heston_params = HestonParams(v0=0.04, kappa=10.0, theta=0.04, xi=1.0, rho=-1.0)


def make_payoff(barrier: float):
    return CashPayment(Fixing(SYMBOL, EXPIRY_DATE) / SPOT >= barrier, EXPIRY_DATE)


if __name__ == "__main__":
    mkt = SimpleMarket(PRICING_DATE, SYMBOL, SPOT, 0.0, 0.0, 0.2)

    heston_pricer = HestonPricer(mkt, heston_params)
    local_vol_pricer = ApproxLocalVolMCPricer(mkt, heston_params, n_paths=1_000_000,
                                              max_dt=1.0 / 252.0, n_threads=8)

    local_vol_scenarios = local_vol_pricer.generate_scenarios(make_payoff(1.0))

    print(f"{'Barrier':>8} | {'Heston':>8} | {'LocalVol':>8}")
    print("-" * 35)

    for i in range(40):  # barrier = 1.00, 1.01, ..., 1.39
        barrier = 1.0 + i * 0.01
        pf = make_payoff(barrier)
        heston_price = heston_pricer.price(pf)
        local_vol_price = local_vol_pricer.price_from_scenarios(pf, local_vol_scenarios)
        print(f"{barrier:>8.2f} | {heston_price:>8.4f} | {local_vol_price:>8.4f}")
