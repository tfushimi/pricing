// clang-format off
#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>  // needed for automatic list -> vector conversion
#include <pybind11_json/pybind11_json.hpp>  // needed for python json <-> nlohmann json conversion
// clang-format on
#include <nlohmann/json.hpp>

#include "RegisterBindings.h"
#include "common/Date.h"
#include "payoff/Observable.h"
#include "payoff/Payoff.h"
#include "payoff/Transforms.h"

namespace py = pybind11;

using namespace calendar;
using namespace payoff;

void register_payoffs(py::module& m) {
    py::class_<PayoffNodePtr>(m, "Payoff")
        .def(py::self + py::self)
        .def("__repr__", [](const PayoffNodePtr& self) { return self->toString(); })
        .def("to_json", [](const PayoffNodePtr& self) { return toJson(self); })
    .def("get_symbols", [](const PayoffNodePtr& self) {
        const auto [symbols, _] = getSymbolsAndFixingDates(self);
        std::vector<std::string> result;
        result.assign(symbols.begin(), symbols.end());
        return result;
    });

    m.def("from_json", [](const nlohmann::json& json) { return payoffFromJson(json); });

    m.def(
        "CashPayment",
        [](const py::object& amount, const py::object& settlementDate) {
            return cashPayment(toObservable(amount), toDate(settlementDate));
        },
        py::arg("amount"), py::arg("settlement_date"));

    m.def(
        "BranchPayment",
        [](const py::object& condition, const py::object& _then, const py::object& _else) {
            return branchPayment(toObservable(condition), toPayoff(_then), toPayoff(_else));
        },
        py::arg("condition"), py::arg("then"), py::arg("else"));
}