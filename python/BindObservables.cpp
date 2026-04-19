// clang-format off
#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>  // needed for automatic list -> vector conversion
// clang-format on

#include "RegisterBindings.h"
#include "common/Date.h"
#include "payoff/Observable.h"

namespace py = pybind11;

using namespace calendar;
using namespace payoff;

void register_observables(py::module& m) {
    py::class_<ObservableNodePtr>(m, "ObservableNodePtr")
        .def(py::self + py::self)
        .def(py::self - py::self)
        .def(py::self * py::self)
        .def(py::self / py::self)
        .def(-py::self)
        .def(py::self > py::self)
        .def(py::self >= py::self)
        .def("__add__",
             [](const ObservableNodePtr& self, const double val) { return self + constant(val); })
        .def("__radd__",
             [](const ObservableNodePtr& self, const double val) { return constant(val) + self; })
        .def("__sub__",
             [](const ObservableNodePtr& self, const double val) { return self - constant(val); })
        .def("__rsub__",
             [](const ObservableNodePtr& self, const double val) { return constant(val) - self; })
        .def("__mul__",
             [](const ObservableNodePtr& self, const double val) { return self * constant(val); })
        .def("__rmul__",
             [](const ObservableNodePtr& self, const double val) { return constant(val) * self; })
        .def("__truediv__",
             [](const ObservableNodePtr& self, const double val) { return self / constant(val); })
        .def("__rtruediv__",
             [](const ObservableNodePtr& self, const double val) { return constant(val) / self; })
        .def("__gt__", [](const ObservableNodePtr& self, const double val) { return self > val; })
        .def("__ge__", [](const ObservableNodePtr& self, const double val) { return self >= val; })
        .def("__repr__", [](const ObservableNodePtr& self) { return self->toString(); });

    m.def("Max", [](const std::vector<py::object>& items) {
        std::vector<ObservableNodePtr> nodes;
        for (const auto& item : items) {
            nodes.push_back(toObservable(item));
        }
        return max(std::move(nodes));
    });
    m.def("Max", [](const py::object& left, const py::object& right) {
        return max(toObservable(left), toObservable(right));
    });
    m.def("Min", [](const std::vector<py::object>& items) {
        std::vector<ObservableNodePtr> nodes;
        for (const auto& item : items) {
            nodes.push_back(toObservable(item));
        }
        return min(std::move(nodes));
    });
    m.def("Min", [](const py::object& left, const py::object& right) {
        return min(toObservable(left), toObservable(right));
    });
    m.def("Sum", [](const std::vector<py::object>& items) {
        std::vector<ObservableNodePtr> nodes;
        for (const auto& item : items) {
            nodes.push_back(toObservable(item));
        }
        return sum(std::move(nodes));
    });

    m.def(
        "Ite",
        [](const py::object& condition, const py::object& _then, const py::object& _else) {
            return ite(toObservable(condition), toObservable(_then), toObservable(_else));
        },
        py::arg("condition"), py::arg("then"), py::arg("else"));

    m.def(
        "Fixing",
        [](const std::string& symbol, const py::object& date) {
            return fixing(symbol, toDate(date));
        },
        py::arg("symbol"), py::arg("date"));
}