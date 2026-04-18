# pricing

C++20 derivatives pricing library focused on exotic and structured products.

## Project layout

- `libs/` — core library, split into modules under `include/`, `src/`, and `tests/`
- `app/` — executable entry points (one `.cpp` per product/strategy)

Each module under `libs/` has its headers in `include/<module>/`, implementations in `src/<module>/`, and tests in `tests/<module>/`.
