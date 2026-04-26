"""Populate market.db with realistic market data (random walk prices, tenor-dependent SVI)."""

import math
import sqlite3
from datetime import date, timedelta

import numpy as np

import market_db

SYMBOL = "SPX"
CCY = "USD"
START = date(2024, 1, 1)
END = date(2026, 4, 24)
SPOT0 = 4500.0
MU = 0.08 / 252.0
VOL = 0.15 / math.sqrt(252)
RATE = 0.045
DIV_YIELD = 0.015

TENORS_DAYS = [7, 30, 60, 91, 182, 365, 730]
STRIKES = [3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000]


def _svi_params(T: float) -> tuple[float, float, float, float, float]:
    return 0.04 * T, 0.4 * math.sqrt(T), -0.7, 0.0, 0.1 + 0.05 * T


def main() -> None:
    conn = sqlite3.connect("market.db")
    cur = conn.cursor()
    market_db.create_schema(cur)

    rng = np.random.default_rng(42)
    all_days = [START + timedelta(days=i) for i in range((END - START).days + 1)]
    biz_days = [d for d in all_days if d.weekday() < 5]
    log_rets = rng.normal(MU - 0.5 * VOL**2, VOL, len(biz_days))
    prices = SPOT0 * np.exp(np.cumsum(log_rets))

    for d, p in zip(biz_days, prices):
        market_db.insert_price(cur, SYMBOL, d, float(p))

    asof = biz_days[-1]
    spot = float(prices[-1])

    for t_days in TENORS_DAYS:
        maturity = asof + timedelta(days=t_days)
        T = t_days / 365.0
        df = math.exp(-RATE * T)
        fwd = spot * math.exp((RATE - DIV_YIELD) * T)
        a, b, rho, m, sigma = _svi_params(T)

        market_db.insert_curve(cur, SYMBOL, CCY, asof, maturity, df, fwd)
        market_db.insert_vol_slice(cur, SYMBOL, asof, maturity, a, b, rho, m, sigma, STRIKES, fwd)

    conn.commit()
    conn.close()
    print(f"market.db written: {len(biz_days)} prices, {len(TENORS_DAYS)} tenors, "
          f"asof={asof.isoformat()}, spot={spot:.2f}")


if __name__ == "__main__":
    main()
