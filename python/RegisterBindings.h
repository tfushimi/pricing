#pragma once
#include <pybind11/pybind11.h>

#include "common/Date.h"
#include "payoff/Observable.h"
#include "payoff/Transforms.h"

inline payoff::ObservableNodePtr toObservable(const pybind11::object& item) {
    if (pybind11::isinstance<payoff::ObservableNodePtr>(item))
        return item.cast<payoff::ObservableNodePtr>();
    return payoff::constant(item.cast<double>());
}

inline payoff::PayoffNodePtr toPayoff(const pybind11::object& item) {
    return item.cast<payoff::PayoffNodePtr>();
}

inline calendar::Date toDate(const pybind11::object& date) {
    if (pybind11::isinstance<pybind11::str>(date))
        return calendar::fromString(date.cast<std::string>());
    if (pybind11::hasattr(date, "year") && pybind11::hasattr(date, "month") &&
        pybind11::hasattr(date, "day"))
        return calendar::makeDate(date.attr("year").cast<int>(), date.attr("month").cast<int>(),
                                  date.attr("day").cast<int>());
    throw pybind11::type_error("date must be a str (YYYY-MM-DD) or datetime.date");
}

void register_observables(pybind11::module& m);
void register_payoffs(pybind11::module& m);