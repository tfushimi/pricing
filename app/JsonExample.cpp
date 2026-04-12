#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "common/Date.h"
#include "payoff/Observable.h"
#include "payoff/Transforms.h"

using namespace calendar;
using namespace payoff;
using namespace market;
using namespace nlohmann;

int main() {
    const Date fixingDate = makeDate(2026, 1, 15);
    const auto payoff = max(fixing("SPY", fixingDate) - 100.0, 0.0);

    const auto j = toJson(payoff);

    std::cout << j.dump(4) << std::endl;

    return 0;
}
