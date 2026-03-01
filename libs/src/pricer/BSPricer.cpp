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
using namespace pricer;
using namespace bs;

double BSPricer::safeEndPoint(const double slope, const double intercept, const double endpoint) {
    if (endpoint == NEG_INF) {
        return slope < 0.0 ? POS_INF : (slope > 0.0 ? NEG_INF : intercept);
    }

    if (endpoint == POS_INF) {
        return slope < 0.0 ? NEG_INF : (slope > 0.0 ? POS_INF : intercept);
    }

    return slope * endpoint + intercept;
}

double BSPricer::priceSegment(const Segment& segment, const double dF,
                              const BSVolSlice& bsVolSlice) {
    auto Call = [&](const double K) -> double {
        if (K <= 0.0) {
            return dF * bsVolSlice.forward();  // C(0) = disc * F
        }

        if (K == POS_INF) {
            return 0.0;
        }

        return blackCallFormula(bsVolSlice.forward(), K, bsVolSlice.time(), dF, bsVolSlice.vol(K));
    };

    auto DigitalCall = [&](const double K) -> double {
        if (K <= 0.0) {
            return dF;  // DigCall(0) = disc
        }

        if (K == POS_INF) {
            return 0.0;
        }

        return blackDigitalFormula(bsVolSlice.forward(), K, bsVolSlice.time(), dF,
                                   bsVolSlice.vol(K), bsVolSlice.dVolDStrike(K));
    };

    const auto slope = segment.getSlope();
    const auto intercept = segment.getIntercept();
    const auto lo = segment.getLeft();
    const auto hi = segment.getRight();

    const auto left = safeEndPoint(slope, intercept, lo);
    const auto right = safeEndPoint(slope, intercept, hi);

    const auto call = slope * (Call(lo) - Call(hi));
    const auto base = (left == NEG_INF ? 0.0 : left) * DigitalCall(lo);
    const auto excess = (right == POS_INF ? 0.0 : right) * DigitalCall(hi);

    return call + base - excess;
}

double BSPricer::price(const PayoffNodePtr& payoff, const Market& market) {
    const auto newPayoff = applyMarket(payoff, market);
    const auto fixingDates = getFixings(newPayoff);
    if (fixingDates.size() != 1) {
        throw std::invalid_argument("Payoff should have a single fixing, found " +
                                    std::to_string(fixingDates.size()));
    }
    const auto fixingDate = fixingDates.begin()->getDate();

    const auto bsVolSlice = market.getBSVolSlice(fixingDate);

    if (!bsVolSlice) {
        throw std::invalid_argument("BSVolSlice not found in market");
    }

    const auto discountCurve = market.getDiscountCurve(fixingDate);

    if (!discountCurve) {
        throw std::invalid_argument("Discount curve not found");
    }

    const double dF = discountCurve->get(fixingDate);

    double price = 0.0;

    const auto payoffPLF = toPiecewiseLinearFunction(newPayoff);

    for (const Segment& segment : payoffPLF.getSegments()) {
        price += priceSegment(segment, dF, *bsVolSlice);
    }

    return price;
}