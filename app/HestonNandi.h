#pragma once

#include <cassert>
#include <format>
#include <iostream>

#include "common/Types.h"
#include "mc/Process.h"
#include "pricer/LocalVolFormula.h"

constexpr double SPOT = 100.0;
inline double forward(const double) {
    return SPOT;
}

// Heston model with Heston-Nandi parameters
constexpr HestonParams hestonParams{
    .v0 = 0.04, .kappa = 10.0, .theta = 0.04, .xi = 1.0, .rho = -1.0};
const mc::HestonProcess heston{forward, hestonParams};

// LocalVol model with the approximate formula
const mc::LocalVolProcess::LocalVolFunction localVolFunc = [](const Sample& logZ,
                                                              const double time) {
    return pricer::approximateLocalVol(hestonParams, logZ, time);
};
const mc::LocalVolProcess localVol{forward, localVolFunc};