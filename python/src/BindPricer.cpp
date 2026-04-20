// clang-format off
#include <pybind11/pybind11.h>
// clang-format on

#include "RegisterBindings.h"
#include "common/Types.h"
#include "pricer/BSFormula.h"
#include "pricer/BSPricer.h"
#include "pricer/HestonFormula.h"
#include "pricer/HestonPricer.h"
#include "pricer/ImpliedVol.h"
#include "pricer/MCPricerWrapper.h"

namespace py = pybind11;

void register_pricer(py::module& m) {
    py::class_<HestonParams>(m, "HestonParams")
        .def(py::init<double, double, double, double, double>(), py::arg("v0"), py::arg("kappa"),
             py::arg("theta"), py::arg("xi"), py::arg("rho"))
        .def_readonly("v0", &HestonParams::v0)
        .def_readonly("kappa", &HestonParams::kappa)
        .def_readonly("theta", &HestonParams::theta)
        .def_readonly("xi", &HestonParams::xi)
        .def_readonly("rho", &HestonParams::rho);

    m.def("implied_vol", &pricer::impliedVol, py::arg("price"), py::arg("F"), py::arg("K"),
          py::arg("T"), py::arg("dF"), py::arg("tol") = 1e-8);

    m.def("heston_implied_vol", &pricer::hestonImpliedVol, py::arg("F"), py::arg("K"),
          py::arg("T"), py::arg("dF"), py::arg("params"));

    m.def("bs_call", &pricer::bsCallFormula, py::arg("F"), py::arg("K"), py::arg("T"),
          py::arg("dF"), py::arg("vol"));

    m.def("bs_digital_call", &pricer::bsDigitalCallFormula, py::arg("F"), py::arg("K"),
          py::arg("T"), py::arg("dF"), py::arg("vol"), py::arg("dVolDStrike"));

    m.def("heston_call", &pricer::hestonCallFormula, py::arg("F"), py::arg("K"), py::arg("T"),
          py::arg("dF"), py::arg("params"));

    m.def("heston_digital_call", &pricer::hestonDigitalCallFormula, py::arg("F"), py::arg("K"),
          py::arg("T"), py::arg("dF"), py::arg("params"));

    py::class_<pricer::BSPricer>(m, "BSPricer")
        .def(py::init<const market::Market&>(), py::arg("market"))
        .def("price", &pricer::BSPricer::price, py::arg("payoff"));

    py::class_<pricer::HestonPricer>(m, "HestonPricer")
        .def(py::init<const market::Market&, const HestonParams&>(), py::arg("market"),
             py::arg("params"))
        .def("price", &pricer::HestonPricer::price, py::arg("payoff"));

    py::class_<pricer::Scenarios>(m, "Scenarios");

    py::class_<pricer::HestonMCPricer>(m, "HestonMCPricer")
        .def(py::init<const market::Market&, const HestonParams&, int, double, int, int>(),
             py::arg("market"), py::arg("params"), py::arg("n_paths"),
             py::arg("max_dt") = 1.0 / 12.0, py::arg("n_threads") = 1, py::arg("seed") = 0)
        .def("generate_scenarios", &pricer::HestonMCPricer::generateScenarios, py::arg("payoff"))
        .def("price_from_scenarios", &pricer::HestonMCPricer::priceFromScenarios,
             py::arg("payoff"), py::arg("scenarios"))
        .def("price", &pricer::HestonMCPricer::price, py::arg("payoff"));

    py::class_<pricer::ApproxLocalVolMCPricer>(m, "ApproxLocalVolMCPricer")
        .def(py::init<const market::Market&, const HestonParams&, int, double, int, int>(),
             py::arg("market"), py::arg("params"), py::arg("n_paths"),
             py::arg("max_dt") = 1.0 / 12.0, py::arg("n_threads") = 1, py::arg("seed") = 0)
        .def("generate_scenarios", &pricer::ApproxLocalVolMCPricer::generateScenarios,
             py::arg("payoff"))
        .def("price_from_scenarios", &pricer::ApproxLocalVolMCPricer::priceFromScenarios,
             py::arg("payoff"), py::arg("scenarios"))
        .def("price", &pricer::ApproxLocalVolMCPricer::price, py::arg("payoff"));
}
