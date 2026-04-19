#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>  // needed for automatic list -> vector conversion

#include "common/Date.h"
#include "payoff/Observable.h"
#include "payoff/Transforms.h"
#include "RegisterBindings.h"

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

    auto toObservable = [](const py::object& item) -> ObservableNodePtr {
        if (py::isinstance<ObservableNodePtr>(item)) {
            return item.cast<ObservableNodePtr>();
        }
        return constant(item.cast<double>());
    };

    m.def("Max", [toObservable](const std::vector<py::object>& items) {
        std::vector<ObservableNodePtr> nodes;
        for (const auto& item : items) {
            nodes.push_back(toObservable(item));
        }
        return max(std::move(nodes));
    });
    m.def("Max", [toObservable](const py::object& left, const py::object& right) {
        return max(toObservable(left), toObservable(right));
    });
    m.def("Min", [toObservable](const std::vector<py::object>& items) {
        std::vector<ObservableNodePtr> nodes;
        for (const auto& item : items) {
            nodes.push_back(toObservable(item));
        }
        return min(std::move(nodes));
    });
    m.def("Min", [toObservable](const py::object& left, const py::object& right) {
        return min(toObservable(left), toObservable(right));
    });
    m.def("Sum", [toObservable](const std::vector<py::object>& items) {
        std::vector<ObservableNodePtr> nodes;
        for (const auto& item : items) {
            nodes.push_back(toObservable(item));
        }
        return sum(std::move(nodes));
    });

    m.def(
        "Ite",
        [toObservable](const py::object& condition, const py::object& _then,
                       const py::object& _else) {
            return ite(toObservable(condition), toObservable(_then), toObservable(_else));
        },
        py::arg("condition"), py::arg("then"), py::arg("else"));

    m.def(
        "Fixing",
        [](const std::string& symbol, const py::object& date) {
            if (py::isinstance<py::str>(date)) {
                return fixing(symbol, fromString(date.cast<std::string>()));
            }
            if (py::hasattr(date, "year") && py::hasattr(date, "month") &&
                py::hasattr(date, "day")) {
                return fixing(
                    symbol, makeDate(date.attr("year").cast<int>(), date.attr("month").cast<int>(),
                                     date.attr("day").cast<int>()));
            }
            throw py::type_error("date must be a str (YYYY-MM-DD) or datetime.date");
        },
        py::arg("symbol"), py::arg("date"));
}