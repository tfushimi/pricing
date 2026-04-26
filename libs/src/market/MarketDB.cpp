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

// TODO linearly interpolate CurvePoints
double MarketDB::getDiscountFactor(double) const {
    if (!_discount_factor_points.empty()) {
        return _discount_factor_points.front().getValue();
    }

    std::vector<string> maturity_dates(100);
    std::vector<double> df(100);

    const string ccy = "USD";
    sql << "SELECT maturity_date, discount_factor FROM discount_factor WHERE ccy = ? ORDER BY "
           "maturity_date ASC",
        into(maturity_dates), into(df), use(ccy);

    if (maturity_dates.empty()) {
        throw std::runtime_error("No discount factor in MarketDB");
    }

    for (std::size_t i = 0; i < maturity_dates.size(); i++) {
        _discount_factor_points.emplace_back(fromString(maturity_dates[i]), df[i]);
    }

    return _discount_factor_points.front().getValue();
}

// TODO linearly interpolate CurvePoints
double MarketDB::getForward(const std::string& symbol, double) const {
    if (const auto it = _forward_points.find(symbol); it != _forward_points.end()) {
        return it->second.front().getValue();
    }

    std::vector<string> maturity_dates(100);
    std::vector<double> forward_prices(100);

    sql << "SELECT maturity_date, forward FROM forward_price WHERE symbol = ? ORDER BY "
           "maturity_date ASC",
        into(maturity_dates), into(forward_prices), use(symbol);

    if (maturity_dates.empty()) {
        throw std::runtime_error("No forward price in MarketDB");
    }

    std::vector<CurvePoint> result;

    for (std::size_t i = 0; i < maturity_dates.size(); i++) {
        result.emplace_back(fromString(maturity_dates[i]), forward_prices[i]);
    }

    _forward_points.emplace(symbol, std::move(result));

    return _forward_points.at(symbol).front().getValue();
}

const BSVolSlice& MarketDB::getBSVolSlice(const std::string&, const Date&) const {
    throw std::runtime_error("BSVolSlice not implemented");
}
}  // namespace market