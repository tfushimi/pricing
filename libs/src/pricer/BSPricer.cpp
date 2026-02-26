#include "pricer/BSPricer.h"

#include "market/Market.h"
#include "numerics/types.h"
#include "payoff/PayoffNode.h"
#include "payoff/types.h"
#include "pricer/BSFormula.h"
#include "payoff/MarketVisitor.h"
#include "payoff/ConstantFoldVisitor.h"

using namespace payoff;
using namespace numerics::linear;
using namespace market;
using namespace pricer;
using namespace bs;

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

        return blackDigitalFormula(bsVolSlice.forward(), K, bsVolSlice.time(), dF, bsVolSlice.vol(),
                                   bsVolSlice.dVolDStrike(K));
    };

    return slope * (Call(lo) - Call(hi)) + (slope * lo + intercept) * DigitalCall(lo) -
           (slope * hi + intercept) * DigitalCall(hi);
}

PayoffNodePtr applyMarket(const PayoffNodePtr& payoff, const Market& market) {

    MarketVisitor marketVisitor(market);

    const auto tempPayoff = marketVisitor.evaluate(payoff);

    ConstantFoldVisitor constantFoldVisitor;

    return constantFoldVisitor.evaluate(tempPayoff);
}

double BSPricer::price(const PayoffNodePtr& payoff, const Market& market) {

    const auto newPayoff = applyMarket(payoff, market);
    PLFVisitor plfVisitor;
    const auto payoffPLF = plfVisitor.evaluate(newPayoff);
    const auto fixingDate = plfVisitor.getFixingDate();
    const std::unique_ptr<BSVolSlice> bsVolSlice = market.getBSVolSlice(fixingDate);
    const double dF = market.getDiscountFactor(fixingDate);

    double price = 0.0;

    for (const Segment& segment : payoffPLF.getSegments()) {
        price += priceSegment(segment.getSlope(), segment.getIntercept(), segment.getLeft(),
                              segment.getRight(), dF, *bsVolSlice.get());
    }

    return price;
}