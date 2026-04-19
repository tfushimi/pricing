from pypricing import market, payoff, pricer

PRICING_DATE = "2025-01-15"
FIXING_DATE = "2026-01-15"
SETTLEMENT_DATE = "2026-01-17"
SYMBOL = "SPX"
RATE = 0.05

NOTIONAL = 100.0
BARRIER = 80.0
CAP = 120.0
PARTICIPATION = 1.1

# SVI surface calibrated to ~20% ATM vol with realistic equity skew:
#   a=0.04  — overall variance level (ATM total var ≈ 0.05 → ~22% vol)
#   b=0.10  — controls the wings / vol-of-vol
#   rho=-0.30 — negative skew (downside puts more expensive than calls)
#   m=0.00  — ATM forward at current spot
#   sigma=0.10 — smoothness of the smile minimum
svi = market.SVIParams(a=0.04, b=0.10, rho=-0.30, m=0.00, sigma=0.10)

S = payoff.Fixing(SYMBOL, FIXING_DATE)
barrier_condition = S >= BARRIER

# Payoff regions:
#   S <  80:           S                          (breached: full downside)
#   80 <= S < 100:     100                        (protected: capital guarantee)
#   100 <= S < 120:    100 + 1.1 * (S - 100)      (participating: 110% upside)
#   S >= 120:          120                        (capped)
participation_payoff = NOTIONAL + PARTICIPATION * (S - NOTIONAL)
protected_payoff = payoff.Min(payoff.Max(participation_payoff, NOTIONAL), CAP)
note_payoff = payoff.Ite(barrier_condition, protected_payoff, S)
payment = payoff.CashPayment(note_payoff, SETTLEMENT_DATE)

print(payment)
print()

spots = [50, 60, 70, 79, 80, 85, 90, 95, 100, 105, 110, 115, 120, 130, 150]

print(f"{'Spot':>7}  | {'Price':>7} | Region")
print("-" * 35)

for spot in spots:
    m = market.SimpleMarket(PRICING_DATE, SYMBOL, spot, RATE, 0.0, svi)
    price = pricer.BSPricer(m).price(payment)

    if spot < BARRIER:
        region = "breached"
    elif spot < NOTIONAL:
        region = "protected"
    elif spot < CAP:
        region = "participating"
    else:
        region = "capped"

    print(f"{spot:>7.2f}  | {price:>7.2f} | {region}")
