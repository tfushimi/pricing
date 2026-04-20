# pypricing — Python Bindings

Python bindings for the pricing library via pybind11. Exposes the payoff DSL, market data, and pricers.

## Build & Run

```bash
# Build the Python module
docker run --rm -v $(pwd):/work pricing cmake --build /work/cmake-build-docker --target pypricing

# Run Python tests
docker run --rm -v $(pwd):/work pricing cmake --build /work/cmake-build-docker --target pytest

# Run a script
docker run --rm -v $(pwd):/work -e PYTHONPATH=/work/cmake-build-docker/python pricing python3 /work/python/app/barrier_enhanced_note.py

# Interactive use
docker run --rm -it -v $(pwd):/work -e PYTHONPATH=/work/cmake-build-docker/python pricing python3
```

## Modules

- `pypricing.market` — `SimpleMarket`, `SVIParams`, `BSVolSlice`, `year_fraction`
- `pypricing.payoff` — `Fixing`, `CashPayment`, `Max`, `Min`, `Ite`, and arithmetic operators
- `pypricing.pricer` — `BSPricer`, `HestonPricer`, `HestonMCPricer`, `ApproxLocalVolMCPricer`, formula functions

## Apps

| Script | Description |
|---|---|
| `app/barrier_enhanced_note.py` | Barrier-protected capital note priced under BS across a range of spots |
| `app/locally_capped_globally_floored_cliquet.py` | LCGF cliquet (Figure 10.1 of Gatheral 2006) under Heston and approximate local vol |

## Example

```python
from datetime import date
from pypricing.market import SimpleMarket
from pypricing.payoff import CashPayment, Fixing, Max
from pypricing.pricer import BSPricer, HestonMCPricer, HestonParams

mkt = SimpleMarket(date(2025, 1, 15), "SPX", 100.0, 0.05, 0.0, 0.2)

fixing = Fixing("SPX", date(2026, 1, 15))
call = CashPayment(Max(fixing - 100.0, 0.0), date(2026, 1, 17))

# Analytic BS price
print(BSPricer(mkt).price(call))

# Heston MC price
params = HestonParams(v0=0.04, kappa=1.5, theta=0.04, xi=0.3, rho=-0.7)
print(HestonMCPricer(mkt, params, n_paths=100_000).price(call))
```
