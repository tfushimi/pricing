// clang-format off
#include <pybind11/pybind11.h>
// clang-format on

#include "RegisterBindings.h"
#include "common/Types.h"
#include "pricer/BSFormula.h"
#include "pricer/HestonFormula.h"

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

    m.def("bs_call", &pricer::bsCallFormula, py::arg("F"), py::arg("K"), py::arg("T"),
          py::arg("dF"), py::arg("vol"));

    m.def("bs_digital_call", &pricer::bsDigitalCallFormula, py::arg("F"), py::arg("K"),
          py::arg("T"), py::arg("dF"), py::arg("vol"), py::arg("dVolDStrike"));

    m.def("heston_call", &pricer::hestonCallFormula, py::arg("F"), py::arg("K"), py::arg("T"),
          py::arg("dF"), py::arg("params"));

    m.def("heston_digital_call", &pricer::hestonDigitalCallFormula, py::arg("F"), py::arg("K"),
          py::arg("T"), py::arg("dF"), py::arg("params"));
}
