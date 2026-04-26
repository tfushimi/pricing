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

// TODO linearly interpolate between stored pillar dates; currently returns the nearest maturity
double MarketDB::getDiscountFactor(double) const {
    if (!_discountFactorPoints.empty()) {
        return _discountFactorPoints.front().getValue();
    }

    const string ccy = "USD";
    const rowset rs =
        (sql.prepare
             << "SELECT maturity_date, discount_factor FROM discount_factor WHERE ccy = ? ORDER BY "
                "maturity_date ASC",
         use(ccy));

    for (const auto& r : rs) {
        _discountFactorPoints.emplace_back(fromString(r.get<string>(0)), r.get<double>(1));
    }

    if (_discountFactorPoints.empty()) {
        throw std::runtime_error("No discount factor in MarketDB");
    }

    return _discountFactorPoints.front().getValue();
}

// TODO linearly interpolate between stored pillar dates; currently returns the nearest maturity
double MarketDB::getForward(const std::string& symbol, double) const {
    if (const auto it = _forwardPoints.find(symbol); it != _forwardPoints.end()) {
        return it->second.front().getValue();
    }

    const rowset rs =
        (sql.prepare
             << "SELECT maturity_date, forward FROM forward_price WHERE symbol = ? ORDER BY "
                "maturity_date ASC",
         use(symbol));

    std::vector<CurvePoint> result;

    for (const auto& r : rs) {
        result.emplace_back(fromString(r.get<string>(0)), r.get<double>(1));
    }

    if (result.empty()) {
        throw std::runtime_error("No forward price in MarketDB");
    }

    _forwardPoints.emplace(symbol, std::move(result));

    return _forwardPoints.at(symbol).front().getValue();
}

// TODO interpolate BSVolSlice; currently returns a flat vol slice
const BSVolSlice& MarketDB::getBSVolSlice(const std::string& symbol, const Date& date) const {
    const auto key = make_pair(symbol, date);
    if (const auto it = _volPoints.find(key); it != _volPoints.end()) {
        return flatVolSlice;
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