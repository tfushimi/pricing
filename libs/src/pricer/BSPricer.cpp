#include "pricer/BSPricer.h"

#include "common/types.h"
#include "market/Market.h"
#include "numerics/linear/PiecewiseLinearFunction.h"
#include "numerics/linear/Segment.h"
#include "payoff/Observable.h"
#include "payoff/Transforms.h"
#include "pricer/BSFormula.h"

using namespace payoff;
using namespace numerics::linear;
using namespace market;

namespace pricer {

double BSPricer::visit(const CashPayment& node) {
    const auto cashPayment = applyMarket(node, _market);

    const auto fixingDates = getFixings(cashPayment);
    if (fixingDates.size() != 1) {
        throw std::invalid_argument("Payoff should have a single fixing, found " +
                                    std::to_string(fixingDates.size()));
    }

    const auto symbol = fixingDates.begin()->getSymbol();
    const auto fixingDate = fixingDates.begin()->getDate();

    const auto& bsVolSlice = _market.getBSVolSlice(symbol, fixingDate);

    const double dF = _market.getDiscountFactor(node.getSettlementDate());

    double price = 0.0;

    const auto plf = toPiecewiseLinearFunction(cashPayment.getAmountPtr());

    for (const Segment& segment : plf) {
        price += priceSegment(segment, dF, bsVolSlice);
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
        return bsCallFormula(F, K, T, dF, bsVolSlice.vol(K));
    };

    auto DigitalCall = [&](const double K) -> double {
        if (K == 0.0) {
            return dF;
        }
        if (K == POS_INF) {
            return 0.0;
        }
        return bsDigitalFormula(F, K, T, dF, bsVolSlice.vol(K), bsVolSlice.dVolDStrike(K));
    };

    const double leftEndPoint = slope * lo + intercept;
    const double rightEndPoint = hi == POS_INF ? 0.0 : slope * hi + intercept;

    return slope * (Call(lo) - Call(hi)) + leftEndPoint * DigitalCall(lo) -
           rightEndPoint * DigitalCall(hi);
}
}  // namespace pricer