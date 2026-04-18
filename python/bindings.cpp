#include <pybind11/pybind11.h>

#include "register.h"

namespace py = pybind11;

PYBIND11_MODULE(pypricing, m) {
    m.doc() = "pricing library bindings";
    auto payoff = m.def_submodule("payoff");

    register_observables(payoff);
}
