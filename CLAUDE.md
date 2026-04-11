# quant-finance

C++20 derivatives pricing library focused on exotic and structured products.

## Project layout

- `libs/` — core library, split into modules under `include/`, `src/`, and `tests/`
- `app/` — executable entry points (one `.cpp` per product/strategy)

Each module under `libs/` has its headers in `include/<module>/`, implementations in `src/<module>/`, and tests in `tests/<module>/`.

## Build

CMake + Ninja, C++20. The Docker image (Ubuntu 24.04) is the canonical build environment.

```bash
cmake --build cmake-build-docker --target <target>   # build a target
ctest --test-dir cmake-build-docker                  # run all tests
cmake --build cmake-build-docker --target format     # run clang-format
```

## Testing

GoogleTest (fetched via CMake FetchContent). Each module has its own test executable registered with `gtest_discover_tests`. Add new tests in `libs/tests/<module>/`.

## Code style

- Warnings: `-Wall -Wextra -Wsign-compare -Wshadow -Wnon-virtual-dtor -Wpedantic`
- Formatting: `clang-format` via the `format` CMake target
