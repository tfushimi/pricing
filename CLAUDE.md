# pricing

C++20 derivatives pricing library focused on exotic and structured products.

## Project layout

- `libs/` — core library, split into modules under `include/`, `src/`, and `tests/`
- `python/` — pybind11 bindings exposing the library as the `pypricing` Python module

Each module under `libs/` has its headers in `include/<module>/`, implementations in `src/<module>/`, and tests in `tests/<module>/`.

## Build

Targets Linux (Ubuntu 24.04). Build via Docker or natively. See `README.md` for commands.
