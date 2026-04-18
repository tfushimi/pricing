#include <string>
#include <vector>

#include "common/Date.h"
#include "nlohmann/json.hpp"
#include "payoff/Observable.h"
#include "payoff/Payoff.h"
#include "payoff/Transforms.h"

using namespace nlohmann;
using namespace std;

namespace payoff {
namespace {

class JsonEncoder final : public ObservableVisitor<json>, public PayoffVisitor<json> {
   public:
    using ObservableVisitor::evaluate;
    using PayoffVisitor::evaluate;

    JsonEncoder() = default;
    ~JsonEncoder() override = default;

   protected:
    json visit(const Fixing& node) override {
        json j;

        j["type"] = node.typeName();
        j["symbol"] = node.getSymbol();
        j["fixingDate"] = calendar::toString(node.getDate());

        return j;
    }

    json visit(const Constant& node) override {
        json j;

        j["type"] = node.typeName();
        j["value"] = node.getValue();

        return j;
    }

    json visit(const Add& node) override { return encodeBinaryNode(node, node.typeName()); }

    json visit(const Sum& node) override { return encodeVectorNode(node, node.typeName()); }

    json visit(const Multiply& node) override { return encodeBinaryNode(node, node.typeName()); }

    json visit(const Divide& node) override { return encodeBinaryNode(node, node.typeName()); }

    json visit(const Max& node) override { return encodeVectorNode(node, node.typeName()); }

    json visit(const Min& node) override { return encodeVectorNode(node, node.typeName()); }

    json visit(const GreaterThan& node) override { return encodeBinaryNode(node, node.typeName()); }

    json visit(const GreaterThanOrEqual& node) override {
        return encodeBinaryNode(node, node.typeName());
    }

    json visit(const IfThenElse& node) override {
        json j;

        j["type"] = node.typeName();

        j["condition"] = evaluate(node.getCondition());
        j["then"] = evaluate(node.getThen());
        j["else"] = evaluate(node.getElse());

        return j;
    }

    json visit(const CashPayment& node) override {
        json j;

        j["type"] = node.typeName();
        j["amount"] = evaluate(node.getAmount());
        j["settlementDate"] = calendar::toString(node.getSettlementDate());

        return j;
    }

    json visit(const CombinedPayment& node) override {
        json j;

        j["type"] = node.typeName();
        j["left"] = evaluate(node.getLeft());
        j["right"] = evaluate(node.getRight());

        return j;
    }

    json visit(const BranchPayment& node) override {
        json j;

        j["type"] = node.typeName();
        j["condition"] = evaluate(node.getCondition());
        j["then"] = evaluate(node.getThenPayoff());
        j["else"] = evaluate(node.getElsePayoff());

        return j;
    }

   private:
    json encodeBinaryNode(const BinaryNode& node, const string& type) {
        json j;

        j["type"] = type;
        j["left"] = evaluate(node.getLeft());
        j["right"] = evaluate(node.getRight());

        return j;
    }

    json encodeVectorNode(const VectorNode& node, const string& type) {
        json j;

        std::vector<json> nodes;
        nodes.reserve(node.size());
        for (const auto& element : node) {
            nodes.push_back(evaluate(element));
        }

        j["type"] = type;
        j["operands"] = nodes;

        return j;
    }
};
}  // namespace

json toJson(const ObservableNodePtr& observable) {
    return JsonEncoder().evaluate(observable);
}

json toJson(const PayoffNodePtr& payoff) {
    return JsonEncoder().evaluate(payoff);
}

std::vector<ObservableNodePtr> decodeOperands(const json& j) {
    std::vector<ObservableNodePtr> operands;
    for (const auto& element : j["operands"]) {
        operands.push_back(observableFromJson(element));
    }
    return operands;
}

ObservableNodePtr observableFromJson(const json& j) {
    const string type = j["type"].get<string>();

    if (type == Fixing::NAME) {
        return fixing(j["symbol"].get<string>(),
                      calendar::fromString(j["fixingDate"].get<string>()));
    }
    if (type == Constant::NAME) {
        return constant(j["value"].get<double>());
    }
    if (type == Add::NAME) {
        return add(observableFromJson(j["left"]), observableFromJson(j["right"]));
    }
    if (type == Sum::NAME) {
        return sum(decodeOperands(j));
    }
    if (type == Multiply::NAME) {
        return multiply(observableFromJson(j["left"]), observableFromJson(j["right"]));
    }
    if (type == Divide::NAME) {
        return divide(observableFromJson(j["left"]), observableFromJson(j["right"]));
    }
    if (type == Max::NAME) {
        return max(decodeOperands(j));
    }
    if (type == Min::NAME) {
        return min(decodeOperands(j));
    }
    if (type == GreaterThan::NAME) {
        return greaterThan(observableFromJson(j["left"]), observableFromJson(j["right"]));
    }
    if (type == GreaterThanOrEqual::NAME) {
        return greaterThanOrEqual(observableFromJson(j["left"]), observableFromJson(j["right"]));
    }
    if (type == IfThenElse::NAME) {
        return ite(observableFromJson(j["condition"]), observableFromJson(j["then"]),
                   observableFromJson(j["else"]));
    }

    throw std::invalid_argument("Unknown observable type: " + type);
}

PayoffNodePtr payoffFromJson(const json& j) {
    const string type = j["type"].get<string>();

    if (type == CashPayment::NAME) {
        return cashPayment(observableFromJson(j["amount"]),
                           calendar::fromString(j["settlementDate"].get<string>()));
    }
    if (type == CombinedPayment::NAME) {
        return combinedPayment(payoffFromJson(j["left"]), payoffFromJson(j["right"]));
    }
    if (type == BranchPayment::NAME) {
        return branchPayment(observableFromJson(j["condition"]), payoffFromJson(j["then"]),
                             payoffFromJson(j["else"]));
    }

    throw std::invalid_argument("Unknown payoff type: " + type);
}
}  // namespace payoff