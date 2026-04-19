import math
import pytest
from pypricing import market, payoff, pricer

PRICING_DATE = "2026-01-15"
EXPIRY_DATE = "2027-01-15"
SETTLEMENT_DATE = "2027-01-17"
SYMBOL = "SPX"
SPOT = 100.0
STRIKE = 100.0
RATE = 0.05
DIVIDEND = 0.0
VOL = 0.2
T = market.year_fraction(PRICING_DATE, EXPIRY_DATE)
T_SETTLE = market.year_fraction(PRICING_DATE, SETTLEMENT_DATE)
DF = math.exp(-RATE * T_SETTLE)
F = SPOT * math.exp((RATE - DIVIDEND) * T)


@pytest.fixture
def flat_market():
    return market.SimpleMarket(PRICING_DATE, SYMBOL, SPOT, RATE, DIVIDEND, VOL)


@pytest.fixture
def vanilla_call():
    fixing = payoff.Fixing(SYMBOL, EXPIRY_DATE)
    return payoff.CashPayment(payoff.Max(fixing - STRIKE, 0.0), SETTLEMENT_DATE)


@pytest.fixture
def heston_params():
    return pricer.HestonParams(v0=0.04, kappa=1.5, theta=0.04, xi=0.3, rho=-0.7)


def test_heston_params_attributes(heston_params):
    assert heston_params.v0 == pytest.approx(0.04)
    assert heston_params.kappa == pytest.approx(1.5)
    assert heston_params.theta == pytest.approx(0.04)
    assert heston_params.xi == pytest.approx(0.3)
    assert heston_params.rho == pytest.approx(-0.7)


def test_bs_call():
    price = pricer.bs_call(F=F, K=STRIKE, T=T, dF=DF, vol=VOL)
    assert price == pytest.approx(10.443, abs=1e-3)


def test_bs_digital_call():
    price = pricer.bs_digital_call(F=F, K=STRIKE, T=T, dF=DF, vol=VOL, dVolDStrike=0.0)
    assert price == pytest.approx(0.5323, abs=1e-3)


def test_heston_call(heston_params):
    price = pricer.heston_call(F=F, K=STRIKE, T=T, dF=DF, params=heston_params)
    assert price == pytest.approx(10.361, abs=1e-2)


def test_heston_digital_call():
    # xi -> 0, rho = 0: Heston degenerates to BS with flat vol = sqrt(v0)
    flat = pricer.HestonParams(v0=VOL**2, kappa=5.0, theta=VOL**2, xi=1e-4, rho=0.0)
    price = pricer.heston_digital_call(F=F, K=STRIKE, T=T, dF=DF, params=flat)
    assert price == pytest.approx(0.5323, abs=1e-3)


def test_bs_pricer_matches_formula(flat_market, vanilla_call):
    expected = pricer.bs_call(F=F, K=STRIKE, T=T, dF=DF, vol=VOL)
    price = pricer.BSPricer(flat_market).price(vanilla_call)
    assert price == pytest.approx(expected, abs=1e-6)


def test_heston_pricer_matches_formula(flat_market, vanilla_call, heston_params):
    expected = pricer.heston_call(F=F, K=STRIKE, T=T, dF=DF, params=heston_params)
    price = pricer.HestonPricer(flat_market, heston_params).price(vanilla_call)
    assert price == pytest.approx(expected, abs=1e-6)
