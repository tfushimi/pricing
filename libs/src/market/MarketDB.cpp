#include "market/MarketDB.h"

#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>

#include <iostream>
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

// TODO linearly interpolate CurvePoints
double MarketDB::getDiscountFactor(double) const {
    if (!_discountFactorPoints.empty()) {
        return _discountFactorPoints.front().getValue();
    }

    std::vector<string> maturityDates(100);
    std::vector<double> discountFactors(100);

    const string ccy = "USD";
    sql << "SELECT maturity_date, discount_factor FROM discount_factor WHERE ccy = ? ORDER BY "
           "maturity_date ASC",
        into(maturityDates), into(discountFactors), use(ccy);

    if (maturityDates.empty()) {
        throw std::runtime_error("No discount factor in MarketDB");
    }

    for (std::size_t i = 0; i < maturityDates.size(); i++) {
        _discountFactorPoints.emplace_back(fromString(maturityDates[i]), discountFactors[i]);
    }

    return _discountFactorPoints.front().getValue();
}

// TODO linearly interpolate CurvePoints
double MarketDB::getForward(const std::string& symbol, double) const {
    if (const auto it = _forwardPoints.find(symbol); it != _forwardPoints.end()) {
        return it->second.front().getValue();
    }

    std::vector<string> maturityDates(100);
    std::vector<double> forwardPrices(100);

    sql << "SELECT maturity_date, forward FROM forward_price WHERE symbol = ? ORDER BY "
           "maturity_date ASC",
        into(maturityDates), into(forwardPrices), use(symbol);

    if (maturityDates.empty()) {
        throw std::runtime_error("No forward price in MarketDB");
    }

    std::vector<CurvePoint> result;

    for (std::size_t i = 0; i < maturityDates.size(); i++) {
        result.emplace_back(fromString(maturityDates[i]), forwardPrices[i]);
    }

    _forwardPoints.emplace(symbol, std::move(result));

    return _forwardPoints.at(symbol).front().getValue();
}

// TODO interpolate BSVolSlice
const BSVolSlice& MarketDB::getBSVolSlice(const std::string& symbol, const Date& date) const {
    std::vector<string> maturityDates(100);
    std::vector<double> strikes(100);
    std::vector<double> impliedVols(100);

    const string dateStr = toString(date);
    sql << "SELECT maturity_date, strike, implied_vol FROM implied_vol WHERE symbol = ? AND "
           "maturity_date = ? ORDER BY strike ASC",
        into(maturityDates), into(strikes), into(impliedVols), use(symbol), use(dateStr);

    if (maturityDates.empty()) {
        throw std::runtime_error("No implied_vol in MarketDB");
    }

    std::vector<VolPoint> result;

    for (std::size_t i = 0; i < maturityDates.size(); i++) {
        result.emplace_back(fromString(maturityDates[i]), strikes[i], impliedVols[i]);
    }

    const auto key = make_pair(symbol, date);
    _volPoints.emplace(key, std::move(result));

    return flatVolSlice;
}
}  // namespace market