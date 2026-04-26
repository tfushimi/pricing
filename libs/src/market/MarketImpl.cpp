#include "market/MarketImpl.h"

#include <optional>
#include <string>

#include "common/Date.h"
#include "market/BSVolSlice.h"

using namespace market;
using namespace calendar;

namespace market {

std::optional<double> MarketImpl::getPrice(const std::string&, const Date&) const {

    return 0;
}

double MarketImpl::getDiscountFactor(double) const {
    return 0;
}

double MarketImpl::getForward(const std::string&, double) const {

    return 0;
}

const BSVolSlice& MarketImpl::getBSVolSlice(const std::string&, const Date&) const {
    throw std::runtime_error("BSVolSlice not implemented");
}
}