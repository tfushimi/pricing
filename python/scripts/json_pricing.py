"""
Loads a payoff from a JSON file and prices it using Black-Scholes.
"""

import json
from datetime import date
from pathlib import Path

from pypricing.market import SimpleMarket
from pypricing.payoff import from_json
from pypricing.pricer import BSPricer

if __name__ == "__main__":
    payoff_path = Path(__file__).parent / "payoff.json"
    with open(payoff_path) as f:
        payoff = from_json(json.load(f))

    symbol = payoff.get_symbols()[0]
    mkt = SimpleMarket(date(2002, 12, 1), symbol, 100.0, 0.0, 0.0, 0.2)

    print(BSPricer(mkt).price(payoff))
