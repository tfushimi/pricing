#include <string>
#include <vector>

#include "common/Date.h"
#include "nlohmann/json.hpp"
#include "payoff/Observable.h"
#include "payoff/Transforms.h"

using namespace nlohmann;
using namespace std;

namespace payoff {
namespace {

class JsonEncoder final : public ObservableVisitor<json> {
   public:
    JsonEncoder() = default;
    ~JsonEncoder() override = default;

   protected:
    json visit(const Fixing& node) override {
        json j;

        j["type"] = node.toString();
        j["symbol"] = node.getSymbol();
        j["fixingDate"] = calendar::toString(node.getDate());

        return j;
    }

    json visit(const Constant& node) override {
        json j;

        j["type"] = node.toString();
        j["value"] = node.getValue();

        return j;
    }

    json visit(const Add& node) override { return encodeBinaryNode(node, node.toString()); }

    json visit(const Sum& node) override { return encodeVectorNode(node, node.toString()); }

    json visit(const Multiply& node) override { return encodeBinaryNode(node, node.toString()); }

    json visit(const Divide& node) override { return encodeBinaryNode(node, node.toString()); }

    json visit(const Max& node) override { return encodeVectorNode(node, node.toString()); }

    json visit(const Min& node) override { return encodeVectorNode(node, node.toString()); }

    json visit(const GreaterThan& node) override { return encodeBinaryNode(node, node.toString()); }

    json visit(const GreaterThanOrEqual& node) override {
        return encodeBinaryNode(node, node.toString());
    }

    json visit(const IfThenElse& node) override {
        json j;

        j["type"] = node.toString();

        j["cond"] = evaluate(node.getCond());
        j["then"] = evaluate(node.getThen());
        j["else"] = evaluate(node.getElse());

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
}  // namespace payoff