# pricing

A C++20 library for pricing exotic and structured derivatives using Monte Carlo simulation.

## Project Structure

```
libs/
  include/
    common/     # Date, types, utilities
    market/     # Market data: vol surfaces (SVI), discount factors
    mc/         # Monte Carlo engine: processes, time grid, simulation
    numerics/   # Root finding, RNG
    payoff/     # Payoff DSL: observable/payoff trees, transforms
    pricer/     # Pricers: MC pricer, PLF payoff pricer
  src/          # Implementations
  tests/        # GoogleTest unit tests

app/            # Executable entry points (one .cpp per product)
```

## Build

CMake + Ninja, C++20. Requires GCC 13+ on Ubuntu 24.04 (or Docker — a `Dockerfile` is provided).

**Ubuntu (native)**
```bash
apt-get install build-essential cmake ninja-build clang-format

cmake -S . -B cmake-build -G Ninja
cmake --build cmake-build --target barrier_enhanced_note
./cmake-build/app/barrier_enhanced_note

ctest --test-dir cmake-build
```

**Docker**
```bash
# Build the image (one-time)
docker build -t pricing .

# Configure (one-time, downloads dependencies)
docker run --rm -v $(pwd):/work pricing cmake -S /work -B /work/cmake-build-docker -G Ninja

# Build and run
docker run --rm -v $(pwd):/work pricing cmake --build /work/cmake-build-docker --target barrier_enhanced_note
docker run --rm -v $(pwd):/work pricing /work/cmake-build-docker/app/barrier_enhanced_note

# Run all tests
docker run --rm -v $(pwd):/work pricing ctest --test-dir /work/cmake-build-docker
```
