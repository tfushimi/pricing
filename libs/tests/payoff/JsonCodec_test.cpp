#include <gtest/gtest.h>

#include "payoff/Observable.h"
#include "payoff/Transforms.h"

using namespace calendar;
using namespace payoff;
using namespace nlohmann;
using namespace std;

const auto fixingDate = makeDate(2026, 3, 20);
const auto settlementDate = makeDate(2026, 3, 27);
const auto spy = fixing("SPX", fixingDate);
constexpr double doubleValue = 123.45;

void assertFixingJson(const json& j) {
    ASSERT_EQ(j["type"].get<string>(), "Fixing");
    ASSERT_EQ(j["symbol"].get<string>(), "SPX");
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
    const vector payoffs = {spy + doubleValue, spy * doubleValue, spy / doubleValue,
                            spy > doubleValue, spy >= doubleValue};

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

    ASSERT_EQ(j["condition"]["type"].get<string>(), "GreaterThan");
    assertFixingJson(j["condition"]["left"]);
    assertConstantJson(j["condition"]["right"]);

    assertFixingJson(j["then"]);
    assertConstantJson(j["else"]);
}

void assertCashPaymentJson(const json& j) {
    ASSERT_EQ(j["type"].get<string>(), "CashPayment");
    assertFixingJson(j["amount"]);
    ASSERT_EQ(j["settlementDate"].get<string>(), toString(settlementDate));
}

TEST(JsonEncoderTest, CashPaymentTest) {
    const auto payoff = cashPayment(spy, settlementDate);
    const json j = toJson(payoff);
    assertCashPaymentJson(j);
}

TEST(JsonEncoderTest, CombinedPaymentTest) {
    const auto left = cashPayment(spy, settlementDate);
    const auto right = cashPayment(doubleValue, settlementDate);
    const json j = toJson(left + right);

    ASSERT_EQ(j["type"].get<string>(), "CombinedPayment");

    assertCashPaymentJson(j["left"]);

    ASSERT_EQ(j["right"]["type"].get<string>(), "CashPayment");
    assertConstantJson(j["right"]["amount"]);
}

TEST(JsonEncoderTest, BranchPaymentTest) {
    const auto condition = spy > doubleValue;
    const auto thenPayoff = cashPayment(spy, settlementDate);
    const auto elsePayoff = cashPayment(doubleValue, settlementDate);
    const json j = toJson(branchPayment(condition, thenPayoff, elsePayoff));

    ASSERT_EQ(j["type"].get<string>(), "BranchPayment");

    ASSERT_EQ(j["condition"]["type"].get<string>(), "GreaterThan");
    assertFixingJson(j["condition"]["left"]);
    assertConstantJson(j["condition"]["right"]);

    assertCashPaymentJson(j["then"]);

    ASSERT_EQ(j["else"]["type"].get<string>(), "CashPayment");
    assertConstantJson(j["else"]["amount"]);
}

// Decoder tests — roundtrip: encode -> decode -> encode and compare

TEST(JsonDecoderTest, FixingRoundtrip) {
    ASSERT_EQ(toJson(observableFromJson(toJson(spy))), toJson(spy));
}

TEST(JsonDecoderTest, ConstantRoundtrip) {
    ASSERT_EQ(toJson(observableFromJson(toJson(doubleValue))), toJson(doubleValue));
}

TEST(JsonDecoderTest, BinaryNodeRoundtrip) {
    const vector payoffs = {spy + doubleValue, spy * doubleValue, spy / doubleValue,
                            spy > doubleValue, spy >= doubleValue};

    for (const auto& payoff : payoffs) {
        ASSERT_EQ(toJson(observableFromJson(toJson(payoff))), toJson(payoff));
    }
}

TEST(JsonDecoderTest, VectorNodeRoundtrip) {
    const vector payoffs = {max(spy, doubleValue), min(spy, doubleValue), sum(spy, doubleValue)};

    for (const auto& payoff : payoffs) {
        ASSERT_EQ(toJson(observableFromJson(toJson(payoff))), toJson(payoff));
    }
}

TEST(JsonDecoderTest, IfThenElseRoundtrip) {
    const auto payoff = ite(spy > doubleValue, spy, doubleValue);
    ASSERT_EQ(toJson(observableFromJson(toJson(payoff))), toJson(payoff));
}

TEST(JsonDecoderTest, CashPaymentRoundtrip) {
    const auto payoff = cashPayment(spy, settlementDate);
    ASSERT_EQ(toJson(payoffFromJson(toJson(payoff))), toJson(payoff));
}

TEST(JsonDecoderTest, CombinedPaymentRoundtrip) {
    const auto payoff = cashPayment(spy, settlementDate) + cashPayment(doubleValue, settlementDate);
    ASSERT_EQ(toJson(payoffFromJson(toJson(payoff))), toJson(payoff));
}

TEST(JsonDecoderTest, BranchPaymentRoundtrip) {
    const auto payoff = branchPayment(spy > doubleValue, cashPayment(spy, settlementDate),
                                      cashPayment(doubleValue, settlementDate));
    ASSERT_EQ(toJson(payoffFromJson(toJson(payoff))), toJson(payoff));
}
