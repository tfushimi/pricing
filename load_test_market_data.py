"""Populate test_market.db with simple deterministic data for unit tests."""

import math
import sqlite3
from datetime import date

import market_db

ASOF = date(2026, 4, 24)
SYMBOL = "SPX"
CCY = "USD"
SPOT = 100.0
RATE = 0.05
DIV_YIELD = 0.02

MATURITIES = [
    date(2026, 5, 24),   # ~1M
    date(2026, 7, 24),   # ~3M
    date(2026, 10, 24),  # ~6M
    date(2027, 4, 24),   # ~1Y
]

STRIKES = [80.0, 90.0, 100.0, 110.0, 120.0]

# Fixed SVI parameters — easy to hand-verify
SVI = dict(a=0.04, b=0.10, rho=-0.30, m=0.0, sigma=0.10)


def main() -> None:
    conn = sqlite3.connect("test_market.db")
    cur = conn.cursor()
    market_db.create_schema(cur)

    market_db.insert_price(cur, SYMBOL, ASOF, SPOT)

    for maturity in MATURITIES:
        T = (maturity - ASOF).days / 365.25
        df = math.exp(-RATE * T)
        fwd = SPOT * math.exp((RATE - DIV_YIELD) * T)

        market_db.insert_curve(cur, SYMBOL, CCY, ASOF, maturity, df, fwd)
        market_db.insert_vol_slice(cur, SYMBOL, ASOF, maturity,
                                   strikes=STRIKES, forward=fwd, **SVI)

    conn.commit()
    conn.close()
    print(f"test_market.db written: asof={ASOF.isoformat()}, spot={SPOT}, "
          f"maturities={[m.isoformat() for m in MATURITIES]}, strikes={STRIKES}")


if __name__ == "__main__":
    main()
