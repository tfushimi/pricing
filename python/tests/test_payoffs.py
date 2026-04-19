import pytest
from pypricing import payoff


@pytest.fixture
def spx():
    return payoff.Fixing("SPX", "2026-12-31")


@pytest.fixture
def settle():
    return "2027-01-02"


def test_cash_payment_repr(spx, settle):
    p = payoff.CashPayment(spx, settle)
    assert "CashPayment" in repr(p)


def test_cash_payment_with_observable(spx, settle):
    p = payoff.CashPayment(payoff.Max(spx - 100.0, 0.0), settle)
    assert "CashPayment" in repr(p)


def test_cash_payment_with_float(settle):
    p = payoff.CashPayment(100.0, settle)
    assert "CashPayment" in repr(p)


def test_cash_payment_datetime(spx):
    import datetime
    p = payoff.CashPayment(spx, datetime.date(2027, 1, 2))
    assert "CashPayment" in repr(p)


def test_combined_payment(spx, settle):
    p1 = payoff.CashPayment(spx, settle)
    p2 = payoff.CashPayment(100.0, settle)
    combined = p1 + p2
    assert "CombinedPayment" in repr(combined)


def test_branch_payment(spx, settle):
    condition = spx >= 80.0
    then_ = payoff.CashPayment(spx, settle)
    else_ = payoff.CashPayment(80.0, settle)
    p = payoff.BranchPayment(condition, then_, else_)
    assert "BranchPayment" in repr(p)
