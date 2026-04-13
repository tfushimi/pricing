#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "market/SimpleMarket.h"
#include "common/Date.h"
#include "payoff/Observable.h"
#include "payoff/Payoff.h"
#include "payoff/Transforms.h"
#include "pricer/BSPricer.h"

using namespace calendar;
using namespace market;
using namespace payoff;
using namespace pricer;
using namespace nlohmann;

int main() {
    std::ifstream file("payoff.json");
    if (!file) {
        std::cerr << "Failed to open payoff.json" << std::endl;
        return 1;
    }

    const auto j = json::parse(file);
    const auto payoff = payoffFromJson(j);

    const auto [symbols, fixingDates] = getSymbolsAndFixingDates(payoff);

    if (symbols.size() != 1) {
        std::cerr << "Expected exactly one symbol, got " << symbols.size() << std::endl;
        return 1;
    }

    const std::string symbol = *symbols.begin();
    // ATM call: spot=100, strike=100, vol=0.2, T=1yr, r=0 -> BS price ≈ 7.97
    const Date pricingDate = makeDate(2002, 12, 1);
    const SimpleMarket market{pricingDate, symbol, 100, 0.0, 0.0, 0.2};

    std::cout << "BS price: " << bsPricer(payoff, market) << std::endl;

    return 0;
}
