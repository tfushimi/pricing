#include <fstream>
#include <iostream>
#include <set>
#include <nlohmann/json.hpp>

#include "FlatMarket.h"
#include "common/Date.h"
#include "payoff/Observable.h"
#include "payoff/Payoff.h"
#include "payoff/Transforms.h"
#include "pricer/BSPricer.h"

using namespace calendar;
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

    const auto fixings = getFixings(payoff);
    std::set<std::string> symbols;
    for (const auto& f : fixings) symbols.insert(f.getSymbol());

    if (symbols.size() != 1) {
        std::cerr << "Expected exactly one symbol, got " << symbols.size() << std::endl;
        return 1;
    }

    const std::string symbol = *symbols.begin();
    // ATM call: spot=100, strike=100, vol=0.2, T=1yr, r=0 -> BS price ≈ 7.97
    const Date pricingDate = makeDate(2002, 12, 1);
    const auto market = makeFlatMarket(pricingDate, symbol, 100);

    std::cout << "BS price: " << bsPricer(payoff, market) << std::endl;

    return 0;
}
