// clang-format off
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>  // needed for automatic list -> vector conversion
// clang-format on

#include "RegisterBindings.h"
#include "common/Date.h"
#include "market/Market.h"
#include "market/SimpleMarket.h"

namespace py = pybind11;

using namespace calendar;
using namespace market;

void register_market(py::module& m) {
    m.def(
        "year_fraction",
        [](const py::object& from, const py::object& to) {
            return calendar::yearFraction(toDate(from), toDate(to));
        },
        py::arg("from_date"), py::arg("to_date"));


    py::class_<BSVolSlice>(m, "BSVolSlice")
        .def("forward", &BSVolSlice::forward)
        .def("time", &BSVolSlice::time)
        .def("vol", &BSVolSlice::vol);

    py::class_<Market>(m, "Market")
        .def("get_pricing_date", [](const Market& self) { return fromDate(self.getPricingDate()); })
        .def(
            "get_price",
            [](const Market& self, const std::string& symbol,
               const py::object& date) -> py::object {
                const auto price = self.getPrice(symbol, toDate(date));
                if (price.has_value()) {
                    return py::float_(price.value());
                }
                return py::none();
            },
            py::arg("symbol"), py::arg("date"))
        .def(
            "get_discount_factor",
            [](const Market& self, const double T) { return self.getDiscountFactor(T); },
            py::arg("T"))
        .def(
            "get_discount_factor",
            [](const Market& self, const py::object& date) {
                return self.getDiscountFactor(toDate(date));
            },
            py::arg("date"))
        .def(
            "get_forward",
            [](const Market& self, const std::string& symbol, const double T) {
                return self.getForward(symbol, T);
            },
            py::arg("symbol"), py::arg("T"))
        .def(
            "get_forward",
            [](const Market& self, const std::string& symbol, const py::object& date) {
                return self.getForward(symbol, toDate(date));
            },
            py::arg("symbol"), py::arg("date"))
        .def(
            "get_bs_vol_slice",
            [](const Market& self, const std::string& symbol, const py::object& date)
                -> const BSVolSlice& { return self.getBSVolSlice(symbol, toDate(date)); },
            py::return_value_policy::reference, py::arg("symbol"), py::arg("date"));

    py::class_<SVIParams>(m, "SVIParams")
        .def(py::init<double, double, double, double, double>(), py::arg("a"), py::arg("b"),
             py::arg("rho"), py::arg("m"), py::arg("sigma"))
        .def_readonly("a", &SVIParams::a)
        .def_readonly("b", &SVIParams::b)
        .def_readonly("rho", &SVIParams::rho)
        .def_readonly("m", &SVIParams::m)
        .def_readonly("sigma", &SVIParams::sigma);

    py::class_<SimpleMarket, Market>(m, "SimpleMarket")
        .def(
            py::init([](const py::object& pricingDate, const std::string& symbol, const double spot,
                        const double rate, const double dividend, const double vol) {
                return SimpleMarket(toDate(pricingDate), symbol, spot, rate, dividend, vol);
            }),
            py::arg("pricing_date"), py::arg("symbol"), py::arg("spot"), py::arg("rate"),
            py::arg("dividend"), py::arg("vol"))
        .def(
            py::init([](const py::object& pricingDate, const std::string& symbol, const double spot,
                        const double rate, const double dividend, const SVIParams& svi) {
                return SimpleMarket(toDate(pricingDate), symbol, spot, rate, dividend, svi);
            }),
            py::arg("pricing_date"), py::arg("symbol"), py::arg("spot"), py::arg("rate"),
            py::arg("dividend"), py::arg("svi"));
}