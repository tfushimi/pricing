#include "models/BSPricer.h"

#include "market/Market.h"
#include "models/bsformula.h"
#include "numerics/types.h"
#include "payoff/PayoffNode.h"
#include "payoff/PLVisitor.h"

using namespace payoff;
using namespace numerics::linear;

double BSPricer::priceSegment(const double slope, const double intercept, const double lo,
                              const double hi, const double dF, const BSVolSlice& bsVolSlice) {
    auto Call = [&](const double K) -> double {
        if (K <= 0.0) {
            return dF * bsVolSlice.forward();  // C(0) = disc * F
        }

        if (K == POS_INF) {
            return 0.0;
        }

        return blackCallFormula(bsVolSlice.forward(), K, bsVolSlice.time(), dF, bsVolSlice.vol());
    };

    auto DigitalCall = [&](const double K) -> double {
        if (K <= 0.0) {
            return dF;  // DigCall(0) = disc
        }

        if (K == POS_INF) {
            return 0.0;
        }

        return blackDigitalFormula(bsVolSlice.forward(), K, bsVolSlice.time(), dF, bsVolSlice.vol(), bsVolSlice.dVolDStrike(K));
    };

    return slope * (Call(lo) - Call(hi)) + (slope * lo + intercept) * DigitalCall(lo) -
           (slope * hi + intercept) * DigitalCall(hi);
}

double BSPricer::price(const PayoffNodePtr& payoff, const Market& market) {
    // TODO applyMarket to replace Fixing with any observables in market

    PLVisitor plVisitor;
    const auto payoffPLF = plVisitor.evaluate(payoff);
    const auto fixingDate = plVisitor.getFixingDate();
    const std::unique_ptr<BSVolSlice> bsVolSlice = market.getBSVolSlice(fixingDate);
    const double dF = market.getDiscountFactor(fixingDate);

    double price = 0.0;

    for (const Segment& segment : payoffPLF.getSegments()) {

        price += priceSegment(segment.getSlope(), segment.getIntercept(), segment.getLeft(), segment.getRight(), dF, *bsVolSlice.get());
    }

    return price;
}