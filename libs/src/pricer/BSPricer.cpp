#include "pricer/BSPricer.h"

#include "common/types.h"
#include "market/Market.h"
#include "numerics/linear/PiecewiseLinearFunction.h"
#include "numerics/linear/Segment.h"
#include "payoff/PayoffNode.h"
#include "payoff/Transforms.h"
#include "pricer/BSFormula.h"

using namespace payoff;
using namespace numerics::linear;
using namespace market;

namespace pricer {

double BSPricer::price(const PaymentNodePtr& payment, const Market& market) {
    const auto* cashPayment = dynamic_cast<const CashPayment*>(payment.get());

    if (cashPayment) {
        return price(cashPayment->getAmount(), market, cashPayment->getSettlementDate());
    }

    // TODO Support CombinePayment and MultiplyPayment
    throw std::invalid_argument("invalid payoff");
}

double BSPricer::price(const PayoffNodePtr& payoff, const Market& market,
                       const Date settlementDate) {
    const auto newPayoff = applyMarket(payoff, market);
    const auto fixingDates = getFixings(newPayoff);
    if (fixingDates.size() != 1) {
        throw std::invalid_argument("Payoff should have a single fixing, found " +
                                    std::to_string(fixingDates.size()));
    }

    const auto symbol = fixingDates.begin()->getSymbol();
    const auto fixingDate = fixingDates.begin()->getDate();

    const auto bsVolSlice = market.getBSVolSlice(symbol, fixingDate);

    if (!bsVolSlice) {
        throw std::invalid_argument("BSVolSlice not found in market");
    }

    const auto discountCurve = market.getDiscountCurve(market.getPricingDate());

    if (!discountCurve) {
        throw std::invalid_argument("Discount curve not found");
    }

    const double dF = discountCurve->get(settlementDate);

    double price = 0.0;

    const auto payoffPLF = toPiecewiseLinearFunction(newPayoff);

    for (const Segment& segment : payoffPLF.getSegments()) {
        price += priceSegment(segment, dF, *bsVolSlice);
    }

    return price;
}

double BSPricer::priceSegment(const Segment& segment, const double dF,
                              const BSVolSlice& bsVolSlice) {
    const auto F = bsVolSlice.forward();
    const auto T = bsVolSlice.time();
    const auto slope = segment.getSlope();
    const auto intercept = segment.getIntercept();

    // Under GBM, S >= 0 always — trim segment to non-negative domain
    // P(S < 0) = 0 so segments below zero contribute nothing
    if (segment.getRight() <= 0.0) {
        return 0.0;
    }
    const auto lo = std::max(segment.getLeft(), 0.0);
    const auto hi = segment.getRight();  // may be POS_INF

    auto Call = [&](const double K) -> double {
        if (K == 0.0) {
            return dF * F;
        }
        if (K == POS_INF) {
            return 0.0;
        }
        return blackCallFormula(F, K, T, dF, bsVolSlice.vol(K));
    };

    auto DigitalCall = [&](const double K) -> double {
        if (K == 0.0) {
            return dF;
        }
        if (K == POS_INF) {
            return 0.0;
        }
        return blackDigitalFormula(F, K, T, dF, bsVolSlice.vol(K), bsVolSlice.dVolDStrike(K));
    };

    const double leftEndPoint = slope * lo + intercept;
    const double rightEndPoint = hi == POS_INF ? 0.0 : slope * hi + intercept;

    return slope * (Call(lo) - Call(hi)) + leftEndPoint * DigitalCall(lo) -
           rightEndPoint * DigitalCall(hi);
}
}  // namespace pricer