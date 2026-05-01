"""
Computes the Heston implied volatility smile across a range of log-strikes,
reproducing Figure 4.3 of Gatheral (2006), "The Volatility Surface".

For a given maturity T, the implied volatility is computed by:
  1. Pricing a European call via the Heston characteristic function.
  2. Inverting the Black formula via bisection to recover implied vol.

Heston-Nandi parameters: v0=0.04, kappa=10, theta=0.04, xi=1, rho=-1.
Assumptions: zero interest rates and dividends (dF=1, F=S).
"""

import math

from pypricing.pricer import HestonParams, heston_implied_vol

F = 100.0
dF = 1.0
T = 0.5
heston_params = HestonParams(v0=0.04, kappa=10.0, theta=0.04, xi=1.0, rho=-1.0)

if __name__ == "__main__":
    n = 60

    print(f"{'logStrike':>10} | {'impliedVol':>10}")
    print("-" * 25)

    for i in range(n):
        k = -0.6 + i * (0.8 / (n - 1))
        K = F * math.exp(k)
        vol = heston_implied_vol(F=F, K=K, T=T, dF=dF, params=heston_params)
        print(f"{k:>10.4f} | {vol:>10.4f}")
