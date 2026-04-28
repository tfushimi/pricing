#include "market/MarketDB.h"

#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>

#include <optional>
#include <string>

#include "common/Date.h"
#include "market/BSVolSlice.h"

using namespace market;
using namespace calendar;
using namespace soci;
using namespace std;

namespace market {

MarketDB::MarketDB(const Date pricingDate, const string& url)
    : Market(pricingDate), sql(sqlite3, url) {}

std::optional<double> MarketDB::getPrice(const std::string& symbol, const Date& date) const {
    const auto key = make_pair(symbol, date);
    if (const auto it = _prices.find(key); it != _prices.end()) {
        return it->second;
    }

    const string dateStr = toString(date);
    const rowset<double> rs =
        (sql.prepare << "SELECT price FROM price WHERE symbol = ? AND date = ?", use(symbol),
         use(dateStr));

    for (const double price : rs) {
        _prices[key] = price;
        return price;
    }

    return std::nullopt;
}

double MarketDB::getDiscountFactor(const double T) const {
    if (T < 0.0) {
        throw std::invalid_argument("T should be >= 0");
    }

    if (_discountCurve.has_value()) {
        return (*_discountCurve)(T);
    }

    const auto dateStr = toString(getPricingDate());
    const rowset rs = (sql.prepare << "SELECT maturity_date, discount_factor FROM discount_factor "
                                      "WHERE ccy = 'USD' AND maturity_date > ? ORDER BY "
                                      "maturity_date ASC",
                       use(dateStr));

    // discount factor is 1.0 today
    std::vector maturityDates{getPricingDate()};
    std::vector discountFactors{1.0};
    for (const auto& r : rs) {
        maturityDates.push_back(fromString(r.get<string>(0)));
        discountFactors.push_back(r.get<double>(1));
    }

    if (maturityDates.size() == 1) {
        throw std::runtime_error("No discount factors found for USD");
    }

    const auto discountCurve =
        _discountCurve.emplace(getPricingDate(), maturityDates, discountFactors);  // log-linear

    return discountCurve(T);
}

double MarketDB::getForward(const std::string& symbol, const double T) const {
    if (const auto it = _forwardCurves.find(symbol); it != _forwardCurves.end()) {
        return it->second(T);
    }

    const auto dateStr = toString(getPricingDate());
    const rowset rs = (sql.prepare << "SELECT maturity_date, forward FROM forward_price WHERE "
                                      "symbol = ? AND maturity_date > ? ORDER BY "
                                      "maturity_date ASC",
                       use(symbol), use(dateStr));

    const auto price = getPrice(symbol, getPricingDate());

    if (!price.has_value()) {
        throw std::runtime_error("No forward price found");
    }

    std::vector maturityDates{getPricingDate()};
    std::vector forwardPrices{price.value()};
    for (const auto& r : rs) {
        maturityDates.push_back(fromString(r.get<string>(0)));
        forwardPrices.push_back(r.get<double>(1));
    }

    if (maturityDates.size() == 1) {
        throw std::runtime_error("No forward prices found for " + symbol);
    }

    const auto [forwardCurve, _] = _forwardCurves.emplace(
        std::piecewise_construct, std::forward_as_tuple(symbol),
        std::forward_as_tuple(getPricingDate(), maturityDates, forwardPrices));  // log-linear

    return forwardCurve->second(T);
}

// TODO interpolate BSVolSlice; currently returns a flat vol slice
const BSVolSlice& MarketDB::getBSVolSlice(const std::string& symbol, const Date& date) const {
    const auto key = make_pair(symbol, date);
    if (_volPoints.contains(key)) {
        return flatVolSlice;  // TODO return interpolated BSVolSlice from _volPoints
    }

    const string dateStr = toString(date);
    const rowset rs =
        (sql.prepare
             << "SELECT maturity_date, strike, implied_vol FROM implied_vol WHERE symbol = ? AND "
                "maturity_date = ? ORDER BY strike ASC",
         use(symbol), use(dateStr));

    std::vector<VolPoint> result;

    for (const auto& r : rs) {
        result.emplace_back(fromString(r.get<string>(0)), r.get<double>(1), r.get<double>(2));
    }

    if (result.empty()) {
        throw std::runtime_error("No implied_vol in MarketDB");
    }

    _volPoints.emplace(key, std::move(result));

    return flatVolSlice;
}
}  // namespace market