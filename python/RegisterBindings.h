#pragma once
#include <pybind11/pybind11.h>

#include "payoff/Observable.h"
#include "payoff/Transforms.h"

inline payoff::ObservableNodePtr toObservable(const pybind11::object& item) {
    if (pybind11::isinstance<payoff::ObservableNodePtr>(item))
        return item.cast<payoff::ObservableNodePtr>();
    return payoff::constant(item.cast<double>());
}

void register_observables(pybind11::module& m);