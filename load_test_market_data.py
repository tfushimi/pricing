"""Populate test_market.db with simple deterministic data for unit tests."""

import math
import sqlite3
from datetime import date, timedelta

import market_db

ASOF = date(2026, 4, 24)
SYMBOL = "SPX"
CCY = "USD"
SPOT = 100.0
RATE = 0.05
DIV_YIELD = 0.02

# Fixed SVI parameters — easy to hand-verify
SVI = dict(a=0.04, b=0.10, rho=-0.30, m=0.0, sigma=0.10)

TENORS_DAYS = [30, 91, 182, 365]
STRIKES = [80.0, 90.0, 100.0, 110.0, 120.0]


def main() -> None:
    conn = sqlite3.connect("test_market.db")
    cur = conn.cursor()
    market_db.create_schema(cur)

    market_db.insert_price(cur, SYMBOL, ASOF, SPOT)

    for t_days in TENORS_DAYS:
        maturity = ASOF + timedelta(days=t_days)
        T = t_days / 365.0
        df = math.exp(-RATE * T)
        fwd = SPOT * math.exp((RATE - DIV_YIELD) * T)

        market_db.insert_curve(cur, SYMBOL, CCY, ASOF, maturity, df, fwd)
        market_db.insert_vol_slice(cur, SYMBOL, ASOF, maturity,
                                   strikes=STRIKES, forward=fwd, **SVI)

    conn.commit()
    conn.close()
    print(f"market.db written: asof={ASOF.isoformat()}, spot={SPOT}, "
          f"tenors={TENORS_DAYS}, strikes={STRIKES}")


if __name__ == "__main__":
    main()
