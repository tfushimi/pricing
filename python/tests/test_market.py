import datetime
import math
import pytest
from pypricing import market

PRICING_DATE = "2026-01-15"
SYMBOL = "SPX"
SPOT = 100.0
RATE = 0.05
DIVIDEND = 0.0
VOL = 0.2

@pytest.fixture
def flat_market():
    return market.SimpleMarket(PRICING_DATE, SYMBOL, SPOT, RATE, DIVIDEND, VOL)


@pytest.fixture
def svi_market():
    svi = market.SVIParams(a=0.04, b=0.10, rho=-0.30, m=0.00, sigma=0.10)
    return market.SimpleMarket(PRICING_DATE, SYMBOL, SPOT, RATE, DIVIDEND, svi)

def test_svi_params_attributes():
    svi = market.SVIParams(a=0.04, b=0.10, rho=-0.30, m=0.00, sigma=0.10)
    assert svi.a == pytest.approx(0.04)
    assert svi.b == pytest.approx(0.10)
    assert svi.rho == pytest.approx(-0.30)
    assert svi.m == pytest.approx(0.00)
    assert svi.sigma == pytest.approx(0.10)

def test_pricing_date(flat_market):
    assert flat_market.get_pricing_date() == datetime.date(2026, 1, 15)


def test_get_price_on_pricing_date(flat_market):
    assert flat_market.get_price(SYMBOL, PRICING_DATE) == SPOT


def test_get_price_future_date_returns_none(flat_market):
    assert flat_market.get_price(SYMBOL, "2027-01-15") is None


def test_get_price_unknown_symbol_returns_none(flat_market):
    assert flat_market.get_price("UNKNOWN", PRICING_DATE) is None


def test_get_discount_factor_zero(flat_market):
    assert flat_market.get_discount_factor(0.0) == pytest.approx(1.0)


def test_get_discount_factor_one_year(flat_market):
    expected = math.exp(-RATE * 1.0)
    assert flat_market.get_discount_factor(1.0) == pytest.approx(expected)


def test_get_discount_factor_by_date(flat_market):
    assert flat_market.get_discount_factor("2026-01-15") == pytest.approx(1.0)


def test_get_forward(flat_market):
    expected = SPOT * math.exp(RATE * 1.0)
    assert flat_market.get_forward(SYMBOL, 1.0) == pytest.approx(expected)


def test_get_bs_vol_slice_flat(flat_market):
    vol_slice = flat_market.get_bs_vol_slice(SYMBOL, "2027-01-15")
    assert vol_slice.vol(SPOT) == pytest.approx(VOL)


def test_get_bs_vol_slice_svi(svi_market):
    vol_slice = svi_market.get_bs_vol_slice(SYMBOL, "2027-01-15")
    assert vol_slice.vol(SPOT) > 0.0


def test_simple_market_with_datetime():
    m = market.SimpleMarket(datetime.date(2026, 1, 15), SYMBOL, SPOT, RATE, DIVIDEND, VOL)
    assert m.get_pricing_date() == datetime.date(2026, 1, 15)
