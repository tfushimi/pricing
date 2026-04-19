import pytest
from pypricing import payoff

@pytest.fixture
def spx():
    return payoff.Fixing("SPX", "2026-12-31")

def test_fixing_repr(spx):
    assert repr(spx) == "Fixing(SPX, 2026-12-31)"

def test_add(spx):
    assert repr(spx + 1.0) == "Add(Fixing(SPX, 2026-12-31), 1.000000)"

def test_sub(spx):
    assert repr(spx - 100.0) == "Add(Fixing(SPX, 2026-12-31), Multiply(-1.000000, 100.000000))"

def test_mul(spx):
    assert repr(spx * 2.0) == "Multiply(Fixing(SPX, 2026-12-31), 2.000000)"

def test_neg(spx):
    assert repr(-spx) == "Multiply(Fixing(SPX, 2026-12-31), -1.000000)"

def test_max_binary(spx):
    call = payoff.Max(spx - 100.0, 0.0)
    assert repr(call).startswith("Max(")

def test_max_list(spx):
    call = payoff.Max([spx, spx - 50.0, payoff.Fixing("SPX", "2026-12-31")])
    assert repr(call).startswith("Max(")

def test_min_binary(spx):
    result = payoff.Min(spx, 120.0)
    assert repr(result).startswith("Min(")

def test_ite(spx):
    condition = spx >= 80.0
    result = payoff.Ite(condition, spx, 80.0)
    assert repr(result).startswith("IfThenElse(")

def test_fixing_datetime(spx):
    import datetime
    s = payoff.Fixing("SPX", datetime.date(2026, 12, 31))
    assert repr(s) == "Fixing(SPX, 2026-12-31)"

def test_fixing_invalid_date():
    with pytest.raises(Exception):
        payoff.Fixing("SPX", 12345)
