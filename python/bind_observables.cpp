#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>  // needed for automatic list -> vector conversion

#include "common/Date.h"
#include "payoff/Observable.h"
#include "payoff/Transforms.h"
#include "register.h"

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
             [](const ObservableNodePtr& self, double val) { return self + constant(val); })
        .def("__radd__",
             [](const ObservableNodePtr& self, double val) { return constant(val) + self; })
        .def("__sub__",
             [](const ObservableNodePtr& self, double val) { return self - constant(val); })
        .def("__rsub__",
             [](const ObservableNodePtr& self, double val) { return constant(val) - self; })
        .def("__mul__",
             [](const ObservableNodePtr& self, double val) { return self * constant(val); })
        .def("__rmul__",
             [](const ObservableNodePtr& self, double val) { return constant(val) * self; })
        .def("__truediv__",
             [](const ObservableNodePtr& self, double val) { return self / constant(val); })
        .def("__rtruediv__",
             [](const ObservableNodePtr& self, double val) { return constant(val) / self; })
        .def("__gt__", [](const ObservableNodePtr& self, double val) { return self > val; })
        .def("__ge__", [](const ObservableNodePtr& self, double val) { return self >= val; })
        .def("__repr__", [](const ObservableNodePtr& self) { return self->toString(); });

    auto toObservable = [](const py::object& item) -> ObservableNodePtr {
        if (py::isinstance<ObservableNodePtr>(item)) {
            return item.cast<ObservableNodePtr>();
        }
        return constant(item.cast<double>());
    };

    m.def("Ite", &ite, py::arg("condition"), py::arg("then"), py::arg("else"));
    m.def("Max", [toObservable](std::vector<py::object> items) {
        std::vector<ObservableNodePtr> nodes;
        for (const auto& item : items) {
            nodes.push_back(toObservable(item));
        }
        return max(std::move(nodes));
    });
    m.def("Max", [toObservable](py::object a, py::object b) {
        return max(toObservable(a), toObservable(b));
    });
    m.def("Min", [toObservable](std::vector<py::object> items) {
        std::vector<ObservableNodePtr> nodes;
        for (const auto& item : items) {
            nodes.push_back(toObservable(item));
        }
        return min(std::move(nodes));
    });
    m.def("Min", [toObservable](py::object a, py::object b) {
        return min(toObservable(a), toObservable(b));
    });
    m.def("Sum", [toObservable](std::vector<py::object> items) {
        std::vector<ObservableNodePtr> nodes;
        for (const auto& item : items) {
            nodes.push_back(toObservable(item));
        }
        return sum(std::move(nodes));
    });

    m.def(
        "Fixing",
        [](const std::string& symbol, const std::string& date) {
            return fixing(symbol, fromString(date));
        },
        py::arg("symbol"), py::arg("date"));
}