// clang-format off
#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>  // needed for automatic list -> vector conversion
// clang-format on

#include "RegisterBindings.h"
#include "common/Date.h"
#include "payoff/Observable.h"
#include "payoff/Payoff.h"

namespace py = pybind11;

using namespace calendar;
using namespace payoff;

void register_payoffs(py::module& m) {
    py::class_<PayoffNodePtr>(m, "Payoff").def(py::self + py::self)
    .def("__repr__", [](const PayoffNodePtr& self) { return self->toString(); });

    m.def(
        "CashPayment",
        [](const py::object& amount, const py::object& settlementDate) {
            return cashPayment(toObservable(amount), toDate(settlementDate));
        },
        py::arg("amount"), py::arg("settlementDate"));
    m.def(
        "BranchPayment",
        [](const py::object& condition, const py::object& _then, const py::object& _else) {
            return branchPayment(toObservable(condition), toPayoff(_then), toPayoff(_else));
        },
        py::arg("condition"), py::arg("then"), py::arg("else"));
}