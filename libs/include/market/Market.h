#pragma once

#include <optional>
#include <string>

#include "BSVolSlice.h"
#include "common/Date.h"

namespace market {

// Abstract market data interface. Implementations provide access to prices, rates, and
// volatility surfaces from any data source (in-memory, database, market data vendor, etc.).
class Market {
   public:
    explicit Market(const calendar::Date pricingDate) : _pricingDate(pricingDate) {}
    virtual ~Market() = default;

    // Returns the pricing date — the valuation date from which all time fractions are measured.
    calendar::Date getPricingDate() const { return _pricingDate; };

    // Returns the observed closing price of symbol on the given date, or nullopt if unavailable.
    // TODO maybe take FixingType like CLOSING?
    virtual std::optional<double> getPrice(const std::string& symbol,
                                           const calendar::Date& date) const = 0;

    // Returns the discount factor for time T (in years from pricing date).
    // TODO take currency like USD
    virtual double getDiscountFactor(double T) const = 0;
    double getDiscountFactor(const calendar::Date date) const {
        return getDiscountFactor(calendar::yearFraction(getPricingDate(), date));
    }

    // Returns the forward price of symbol at time T (in years from pricing date).
    virtual double getForward(const std::string& symbol, double T) const = 0;
    double getForward(const std::string& symbol, const calendar::Date date) const {
        return getForward(symbol, calendar::yearFraction(getPricingDate(), date));
    }

    // Returns the Black-Scholes implied volatility slice for symbol at the given expiry date.
    virtual const BSVolSlice& getBSVolSlice(const std::string& symbol,
                                            const calendar::Date& date) const = 0;

   private:
    const calendar::Date _pricingDate;
};
}  // namespace market