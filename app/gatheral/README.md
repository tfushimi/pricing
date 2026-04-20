# Gatheral (2006) — The Volatility Surface

Executables that reproduce figures from Jim Gatheral's *The Volatility Surface* (2006).

All use Heston-Nandi parameters (v0=0.04, κ=10, θ=0.04, ξ=1, ρ=-1) with zero rates and dividends.

| Executable | Figure | Payoff |
|---|---|---|
| `implied_vol_curve` | — | Implied vol curve from the SVI parametrization |

The exotic product examples (digital call, one-touch, cliquets) have been migrated to Python.
See [`python/app/`](../../python/app/).
