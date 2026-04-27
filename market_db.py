"""Shared utilities for creating and populating market.db."""

import math
import sqlite3
from datetime import date


def create_schema(cur: sqlite3.Cursor) -> None:
    cur.executescript("""
        DROP TABLE IF EXISTS price;
        DROP TABLE IF EXISTS forward_price;
        DROP TABLE IF EXISTS discount_factor;
        DROP TABLE IF EXISTS implied_vol;
        DROP TABLE IF EXISTS svi_slice;

        CREATE TABLE price (
            symbol TEXT NOT NULL,
            date   TEXT NOT NULL,
            price  REAL NOT NULL,
            PRIMARY KEY (symbol, date)
        );
        CREATE TABLE forward_price (
            symbol        TEXT NOT NULL,
            date          TEXT NOT NULL,
            maturity_date TEXT NOT NULL,
            forward       REAL NOT NULL,
            PRIMARY KEY (symbol, date, maturity_date)
        );
        CREATE TABLE discount_factor (
            ccy             TEXT NOT NULL,
            date            TEXT NOT NULL,
            maturity_date   TEXT NOT NULL,
            discount_factor REAL NOT NULL,
            PRIMARY KEY (ccy, date, maturity_date)
        );
        CREATE TABLE implied_vol (
            symbol        TEXT NOT NULL,
            date          TEXT NOT NULL,
            maturity_date TEXT NOT NULL,
            strike        REAL NOT NULL,
            implied_vol   REAL NOT NULL,
            PRIMARY KEY (symbol, date, maturity_date, strike)
        );
        CREATE TABLE svi_slice (
            symbol        TEXT NOT NULL,
            date          TEXT NOT NULL,
            maturity_date TEXT NOT NULL,
            a     REAL NOT NULL,
            b     REAL NOT NULL,
            rho   REAL NOT NULL,
            m     REAL NOT NULL,
            sigma REAL NOT NULL,
            PRIMARY KEY (symbol, date, maturity_date)
        );
    """)


def svi_implied_vol(k: float, T: float,
                    a: float, b: float, rho: float, m: float, sigma: float) -> float:
    """Black-Scholes implied vol from SVI total variance at log-moneyness k and maturity T."""
    dk = k - m
    w = a + b * (rho * dk + math.sqrt(dk**2 + sigma**2))
    return math.sqrt(max(w, 1e-8) / T)


def insert_price(cur: sqlite3.Cursor, symbol: str, date: date, price: float) -> None:
    cur.execute("INSERT INTO price VALUES (?, ?, ?)", (symbol, date.isoformat(), price))


def insert_curve(cur: sqlite3.Cursor, symbol: str, ccy: str, asof: date, maturity: date,
                 discount_factor: float, forward: float) -> None:
    asof_str = asof.isoformat()
    maturity_str = maturity.isoformat()
    cur.execute("INSERT INTO discount_factor VALUES (?, ?, ?, ?)",
                (ccy, asof_str, maturity_str, discount_factor))
    cur.execute("INSERT INTO forward_price VALUES (?, ?, ?, ?)",
                (symbol, asof_str, maturity_str, forward))


def insert_vol_slice(cur: sqlite3.Cursor, symbol: str, asof: date, maturity: date,
                     a: float, b: float, rho: float, m: float, sigma: float,
                     strikes: list[float], forward: float) -> None:
    asof_str = asof.isoformat()
    maturity_str = maturity.isoformat()
    T = (maturity - asof).days / 365.25
    cur.execute("INSERT INTO svi_slice VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
                (symbol, asof_str, maturity_str, a, b, rho, m, sigma))
    for K in strikes:
        iv = svi_implied_vol(math.log(K / forward), T, a, b, rho, m, sigma)
        cur.execute("INSERT INTO implied_vol VALUES (?, ?, ?, ?, ?)",
                    (symbol, asof_str, maturity_str, K, iv))
