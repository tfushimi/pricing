#include <gtest/gtest.h>

#include "payoff/Observable.h"
#include "payoff/Transforms.h"

using namespace calendar;
using namespace payoff;
using namespace nlohmann;
using namespace std;

const auto fixingDate = makeDate(2026, 3, 20);
const auto spy = fixing("SPY", fixingDate);
constexpr double doubleValue = 123.45;

void assertFixingJson(const json& j) {
    ASSERT_EQ(j["type"].get<string>(), "Fixing");
    ASSERT_EQ(j["symbol"].get<string>(), "SPY");
    ASSERT_EQ(j["fixingDate"].get<string>(), toString(fixingDate));
}

void assertConstantJson(const json& j) {
    ASSERT_EQ(j["type"].get<string>(), "Constant");
    ASSERT_EQ(j["value"].get<double>(), doubleValue);
}

TEST(JsonEncoderTest, FixingTest) {
    json j = toJson(spy);
    assertFixingJson(j);
}

TEST(JsonEncoderTest, ConstantTest) {
    json j = toJson(doubleValue);
    assertConstantJson(j);
}

TEST(JsonEncoderTest, BinaryNodeTest) {
    const vector payoffs = {spy + doubleValue, spy * doubleValue, spy / doubleValue, spy > doubleValue, spy >= doubleValue};

    const vector<string> types = {"Add", "Multiply", "Divide", "GreaterThan", "GreaterThanOrEqual"};

    ASSERT_EQ(payoffs.size(), types.size());

    for (size_t i = 0; i < payoffs.size(); ++i) {
        json j = toJson(payoffs[i]);

        ASSERT_EQ(j["type"].get<string>(), types[i]);
        assertFixingJson(j["left"]);
        assertConstantJson(j["right"]);
    }
}

TEST(JsonEncoderTest, VectorNodeTest) {
    const vector payoffs = {
        max(spy, doubleValue),
        min(spy, doubleValue),
        sum(spy, doubleValue),
    };

    const vector<string> types = {"Max", "Min", "Sum"};

    ASSERT_EQ(payoffs.size(), types.size());

    for (size_t i = 0; i < payoffs.size(); ++i) {
        json j = toJson(payoffs[i]);

        ASSERT_EQ(j["type"].get<string>(), types[i]);

        const auto array = j["operands"].get<std::vector<json>>();
        ASSERT_EQ(array.size(), 2);
        assertFixingJson(array[0]);
        assertConstantJson(array[1]);
    }
}

TEST(JsonEncoderTest, IfThenElseTest) {
    const auto payoff = ite(spy > doubleValue, spy, doubleValue);

    json j = toJson(payoff);

    ASSERT_EQ(j["type"].get<string>(), "IfThenElse");

    ASSERT_EQ(j["cond"]["type"].get<string>(), "GreaterThan");
    assertFixingJson(j["cond"]["left"]);
    assertConstantJson(j["cond"]["right"]);

    assertFixingJson(j["then"]);
    assertConstantJson(j["else"]);
}