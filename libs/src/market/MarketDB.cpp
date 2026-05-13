#include "market/MarketDB.h"

#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>

#include <algorithm>
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

void MarketDB::buildAndCacheVolSlice(const std::string& symbol, const std::string& dateStr,
                                     const std::vector<double>& strikes,
                                     const std::vector<double>& vols) const {
    if (dateStr.empty() || strikes.empty()) {
        return;
    }

    const auto maturityDate = fromString(dateStr);
    const double time = yearFraction(getPricingDate(), maturityDate);
    const double forward = getForward(symbol, time);
    _bsVolSlices.emplace(make_pair(symbol, maturityDate),
                         std::make_unique<InterpolatedBSVolSlice>(forward, time, strikes, vols));
}

void MarketDB::loadVolSlicesForSymbol(const std::string& symbol) const {
    const auto pricingDateStr = toString(getPricingDate());
    const rowset rs =
        (sql.prepare
             << "SELECT maturity_date, strike, implied_vol FROM implied_vol "
                "WHERE symbol = ? AND maturity_date > ? ORDER BY maturity_date ASC, strike ASC",
         use(symbol), use(pricingDateStr));

    std::vector<double> strikes, vols;
    std::string currentDateStr;

    for (const auto& r : rs) {
        const std::string matDateStr = r.get<string>(0);

        // new maturity, so build a vol slice based on the accumulated strikes and vols
        if (matDateStr != currentDateStr) {
            buildAndCacheVolSlice(symbol, currentDateStr, strikes, vols);

            strikes.clear();
            vols.clear();

            // start accumulating
            currentDateStr = matDateStr;
        }

        // accumulate strikes and vols
        strikes.push_back(r.get<double>(1));
        vols.push_back(r.get<double>(2));
    }

    buildAndCacheVolSlice(symbol, currentDateStr, strikes, vols);
}

const BSVolSlice& MarketDB::getBSVolSlice(const std::string& symbol, const Date& date) const {
    const auto key = make_pair(symbol, date);
    if (const auto it = _bsVolSlices.find(key); it != _bsVolSlices.end()) {
        return *it->second;
    }

    if (!_loadedVolSymbols.contains(symbol)) {
        loadVolSlicesForSymbol(symbol);
        _loadedVolSymbols.insert(symbol);
    }

    if (const auto it = _bsVolSlices.find(key); it != _bsVolSlices.end()) {
        return *it->second;
    }

    // Collect available expiry dates for this symbol in sorted order (map is ordered by key).
    std::vector<Date> availableMaturityDates;
    for (const auto& [k, _] : _bsVolSlices) {
        if (k.first == symbol) {
            availableMaturityDates.push_back(k.second);
        }
    }

    if (availableMaturityDates.empty()) {
        throw std::runtime_error("No implied vol data found for " + symbol);
    }

    // Flat extrapolation beyond the observed expiry range.
    if (date <= availableMaturityDates.front()) {
        return *_bsVolSlices.at({symbol, availableMaturityDates.front()});
    }

    if (date >= availableMaturityDates.back()) {
        return *_bsVolSlices.at({symbol, availableMaturityDates.back()});
    }

    // Find the two bracketing expiry dates (nearDate < date < farDate).
    const auto it = std::lower_bound(availableMaturityDates.begin(), availableMaturityDates.end(), date);
    const Date& farDate = *it;
    const Date& nearDate = *(it - 1);

    const BSVolSlice& near = *_bsVolSlices.at({symbol, nearDate});
    const BSVolSlice& far = *_bsVolSlices.at({symbol, farDate});
    const double t = yearFraction(getPricingDate(), date);
    const double fwd = getForward(symbol, t);

    const auto [inserted, ok] = _bsVolSlices.emplace(
        key, std::make_unique<TemporallyInterpolatedBSVolSlice>(fwd, t, near, far));

    return *inserted->second;
}
}  // namespace market