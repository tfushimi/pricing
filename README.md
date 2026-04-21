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

CMake, C++20, Ubuntu 24.04. A `Dockerfile` is provided for convenience.

**Linux (native)**
```bash
# Install dependencies
apt-get install build-essential cmake ninja-build python3-dev python3-pytest

# Configure (one-time, downloads dependencies)
cmake -B cmake-build-docker

# Build all targets
cmake --build cmake-build-docker

# Run all C++ tests
ctest --test-dir cmake-build-docker
```

**Docker**
```bash
# Build the image (one-time)
docker build -t pricing .

# Configure (one-time, downloads dependencies)
docker run --rm -v $(pwd):/work pricing cmake -S /work -B /work/cmake-build-docker

# Build all targets
docker run --rm -v $(pwd):/work pricing cmake --build /work/cmake-build-docker

# Run all C++ tests
docker run --rm -v $(pwd):/work pricing ctest --test-dir /work/cmake-build-docker
```

## Python Bindings

The full library is also accessible from Python via the `pypricing` module. You can define payoffs,
set up market data, and run pricers without writing any C++. This makes it easy to experiment
interactively, run scenario analysis, or build higher-level workflows on top of the C++ engine.

See [python/README.md](python/README.md) for build instructions, available modules, and example scripts.

