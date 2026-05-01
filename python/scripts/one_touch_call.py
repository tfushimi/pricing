"""
Prices a one-touch call option: pays $1 if the underlying ever reaches the barrier B
during [0, T], zero otherwise.

The running maximum is approximated by daily fixings:

    payoff = 1{ max_{t in {0, dt, 2dt, ..., T}} S_t / S_0 >= B }

The local volatility model prices this higher than Heston. The fundamental reason is the
conditional distribution of future spot: with rho=-1, Heston assigns lower instantaneous
vol to paths where spot has risen (negative spot-vol correlation), suppressing the
probability of sustained upward moves that breach the barrier. Local vol does not capture
this effect in the forward sense.

Reproduces Figure 9.4 of Gatheral (2006), "The Volatility Surface".

Assumptions: zero interest rates and dividends, T=1 year, daily MC steps (dt=1/252),
barrier expressed as a multiple of spot (B=1.0 means at-the-money).
"""

from datetime import date, timedelta

from pypricing.market import SimpleMarket
from pypricing.payoff import CashPayment, Fixing, Max
from pypricing.pricer import ApproxLocalVolMCPricer, HestonMCPricer, HestonParams

PRICING_DATE = date(2000, 1, 1)
SETTLEMENT_DATE = date(2001, 1, 1)
SYMBOL = "Underlier"
SPOT = 100.0
heston_params = HestonParams(v0=0.04, kappa=10.0, theta=0.04, xi=1.0, rho=-1.0)


def make_payoff(barrier: float):
    d, fixing_dates = PRICING_DATE, []
    while d <= SETTLEMENT_DATE:
        if d.weekday() < 5:
            fixing_dates.append(d)
        d += timedelta(days=1)
    spot_ratios = [Fixing(SYMBOL, d) / Fixing(SYMBOL, fixing_dates[0]) for d in fixing_dates]
    return CashPayment(Max(spot_ratios) >= barrier, SETTLEMENT_DATE)


if __name__ == "__main__":
    mkt = SimpleMarket(PRICING_DATE, SYMBOL, SPOT, 0.0, 0.0, 0.2)

    heston_pricer = HestonMCPricer(mkt, heston_params, n_paths=1_000_000,
                                   max_dt=1.0 / 252.0, n_threads=8)
    local_vol_pricer = ApproxLocalVolMCPricer(mkt, heston_params, n_paths=1_000_000,
                                              max_dt=1.0 / 252.0, n_threads=8)

    heston_scenarios = heston_pricer.generate_scenarios(make_payoff(1.0))
    local_vol_scenarios = local_vol_pricer.generate_scenarios(make_payoff(1.0))

    print(f"{'Barrier':>8} | {'Heston':>8} | {'LocalVol':>8}")
    print("-" * 35)

    for i in range(40):  # barrier = 1.00, 1.01, ..., 1.39
        barrier = 1.0 + i * 0.01
        pf = make_payoff(barrier)
        heston_price = heston_pricer.price_from_scenarios(pf, heston_scenarios)
        local_vol_price = local_vol_pricer.price_from_scenarios(pf, local_vol_scenarios)
        print(f"{barrier:>8.2f} | {heston_price:>8.4f} | {local_vol_price:>8.4f}")
