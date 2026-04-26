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
    if (const auto it = _prices.find(symbol); it != _prices.end()) {
        return it->second;
    }

    double price = 0.0;
    indicator ind;
    const string dateStr = toString(date);

    sql << "SELECT price FROM price WHERE symbol = ? AND date = ?", into(price, ind), use(symbol),
        use(dateStr);

    if (ind == i_ok) {
        _prices[symbol] = price;

        return price;
    }

    return std::nullopt;
}

double MarketDB::getDiscountFactor(double) const {
    return 0;
}

double MarketDB::getForward(const std::string&, double) const {
    return 0;
}

const BSVolSlice& MarketDB::getBSVolSlice(const std::string&, const Date&) const {
    throw std::runtime_error("BSVolSlice not implemented");
}
}  // namespace market