import math
import pytest
from pypricing import pricer

# Same inputs as C++ HestonFormula_test
T = 0.5
DF = math.exp(-0.03 * T)
F = 100.0 * math.exp((0.03 - 0.02) * T)
K = 100.0
VOL = math.sqrt(0.05)

@pytest.fixture
def heston_params():
    return pricer.HestonParams(v0=0.05, kappa=5.0, theta=0.05, xi=0.5, rho=-0.8)

def test_heston_params_attributes(heston_params):
    assert heston_params.v0 == pytest.approx(0.05)
    assert heston_params.kappa == pytest.approx(5.0)
    assert heston_params.theta == pytest.approx(0.05)
    assert heston_params.xi == pytest.approx(0.5)
    assert heston_params.rho == pytest.approx(-0.8)

def test_bs_call():
    price = pricer.bs_call(F=F, K=K, T=T, dF=DF, vol=VOL)
    assert price == pytest.approx(6.4730, abs=1e-3)

def test_bs_digital_call():
    price = pricer.bs_digital_call(F=F, K=K, T=T, dF=DF, vol=VOL, dVolDStrike=0.0)
    assert price == pytest.approx(0.4739, abs=1e-3)

def test_heston_call(heston_params):
    # Reference value from "The Heston Model and Its Extensions in MATLAB and C#" p.15
    price = pricer.heston_call(F=F, K=K, T=T, dF=DF, params=heston_params)
    assert price == pytest.approx(6.2528, abs=1e-3)

def test_heston_digital_call():
    # xi -> 0, rho = 0: Heston degenerates to BS with flat vol = sqrt(v0)
    flat = pricer.HestonParams(v0=0.05, kappa=5.0, theta=0.05, xi=1e-4, rho=0.0)
    price = pricer.heston_digital_call(F=F, K=K, T=T, dF=DF, params=flat)
    assert price == pytest.approx(0.4739, abs=1e-3)
