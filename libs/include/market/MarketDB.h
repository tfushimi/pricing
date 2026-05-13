#pragma once

#include <soci/soci.h>

#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>

#include "BSVolSlice.h"
#include "Curve.h"
#include "Market.h"
#include "common/Date.h"

namespace market {

// SQLite-backed Market implementation using SOCI. Market data is loaded lazily on first access
// and cached in memory. url is a SOCI SQLite3 connection string, e.g. "db=market.db".
// To support other databases (e.g. PostgreSQL), replace soci::sqlite3 with the appropriate
// SOCI backend in the constructor.
class MarketDB final : public Market {
   public:
    explicit MarketDB(calendar::Date pricingDate, const std::string& url);
    ~MarketDB() override = default;
    std::optional<double> getPrice(const std::string& symbol,
                                   const calendar::Date& date) const override;
    double getDiscountFactor(double T) const override;
    double getForward(const std::string& symbol, double T) const override;
    const BSVolSlice& getBSVolSlice(const std::string& symbol,
                                    const calendar::Date& date) const override;

   private:
    void loadVolSlicesForSymbol(const std::string& symbol) const;
    void buildAndCacheVolSlice(const std::string& symbol, const std::string& dateStr,
                               const std::vector<double>& strikes,
                               const std::vector<double>& vols) const;

    mutable soci::session sql;
    mutable std::map<std::pair<std::string, calendar::Date>, double> _prices;
    mutable std::optional<LogLinearInterpolatedCurve> _discountCurve;
    mutable std::map<std::string, LogLinearInterpolatedCurve> _forwardCurves;
    mutable std::set<std::string> _loadedVolSymbols;
    mutable std::map<std::pair<std::string, calendar::Date>, std::unique_ptr<BSVolSlice>>
        _bsVolSlices;
};
}  // namespace market