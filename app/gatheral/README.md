# Gatheral (2006) — The Volatility Surface

Executables that reproduce figures from Jim Gatheral's *The Volatility Surface* (2006).

All use Heston-Nandi parameters (v0=0.04, κ=10, θ=0.04, ξ=1, ρ=-1) with zero rates and dividends.

| Executable | Figure | Payoff |
|---|---|---|
| `digital_call` | 9.3 | Digital call: pays 1 if S_T ≥ K, across a range of strikes |
| `one_touch_call` | 9.4 | One-touch call: pays 1 if max(S_t) ≥ B during [0, T], across a range of barriers |
| `napoleon_cliquet` | 10.5 | Napoleon: annual coupon = max(0, MaxCoupon + worst monthly return) |
| `reverse_cliquet` | 10.3 | Reverse Cliquet: max(0, MaxCoupon + sum of semi-annual reverse returns) |
