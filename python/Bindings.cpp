#include <pybind11/pybind11.h>

#include "RegisterBindings.h"

namespace py = pybind11;

PYBIND11_MODULE(pypricing, m) {
    m.doc() = "pricing library bindings";
    auto payoff = m.def_submodule("payoff");
    auto market = m.def_submodule("market");
    auto pricer = m.def_submodule("pricer");

    register_observables(payoff);
    register_payoffs(payoff);
    register_market(market);
    register_pricer(pricer);
}
