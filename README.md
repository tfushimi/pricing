# pricing

A C++20 derivatives pricing library with two core components:

- **Payoff DSL** — a composable expression tree for defining exotic payoffs (barriers, cliquets, digitals, etc.) that is model-agnostic and serializable
- **Pricing models** — Black-Scholes, Heston, and Local Volatility via Monte Carlo simulation

Python bindings via pybind11 expose the payoff DSL and pricers for interactive use and scripting.

## Project Structure

```
libs/
  include/
    common/     # Date, types, utilities
    market/     # Market data
    mc/         # Monte Carlo engine: processes, time grid, simulation
    numerics/   # Root finding, RNG
    payoff/     # Payoff DSL: observable/payoff trees, transforms
    pricer/     # Pricers: MC pricer, PLF payoff pricer
  src/          # Implementations
  tests/        # GoogleTest unit tests

app/            # Executable entry points (one .cpp per product)
python/
  src/          # pybind11 binding sources (pypricing module)
  app/          # Python scripts (e.g. barrier_enhanced_note.py)
  tests/        # pytest tests
```

## Build

CMake + Ninja, C++20. Requires GCC 13+ on Ubuntu 24.04 (or Docker — a `Dockerfile` is provided).

**Ubuntu (native)**
```bash
apt-get install build-essential cmake ninja-build clang-format

cmake -S . -B cmake-build -G Ninja
cmake --build cmake-build

ctest --test-dir cmake-build
```

**Docker**
```bash
# Build the image (one-time)
docker build -t pricing .

# Configure (one-time, downloads dependencies)
docker run --rm -v $(pwd):/work pricing cmake -S /work -B /work/cmake-build-docker -G Ninja

# Build all targets
docker run --rm -v $(pwd):/work pricing cmake --build /work/cmake-build-docker

# Run all C++ tests
docker run --rm -v $(pwd):/work pricing ctest --test-dir /work/cmake-build-docker
```

## Python Bindings

The `pypricing` module exposes the payoff DSL to Python.

```bash
# Build the Python module
docker run --rm -v $(pwd):/work pricing cmake --build /work/cmake-build-docker --target pypricing

# Run Python tests
docker run --rm -v $(pwd):/work pricing cmake --build /work/cmake-build-docker --target pytest

# Run a Python script
docker run --rm -v $(pwd):/work -e PYTHONPATH=/work/cmake-build-docker/python pricing python3 /work/python/app/barrier_enhanced_note.py

# Interactive use
docker run --rm -it -v $(pwd):/work -e PYTHONPATH=/work/cmake-build-docker/python pricing python3
```

```python
>>> from pypricing import payoff
>>> spx = payoff.Fixing("SPX", "2026-12-31")
>>> call = payoff.Max(spx - 100.0, 0.0)
>>> payment = payoff.CashPayment(call, "2027-01-02")
>>> print(payment)
CashPayment(amount=Max(Add(Fixing(SPX, 2026-12-31), Multiply(-1.000000, 100.000000)), 0.000000), settlementDate=2027-01-02)
```
