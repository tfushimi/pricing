#pragma once

#include <cassert>
#include <format>
#include <iostream>
#include <string>
#include <vector>

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

// Helper function to print table
inline void printTable(const std::string& paramName, const std::vector<std::string>& columnNames,
                       const std::vector<double>& params,
                       const std::vector<std::vector<double>>& columns) {
    assert(columns.size() == columnNames.size());
    assert(std::all_of(columns.begin(), columns.end(),
                       [&](const auto& col) { return col.size() == params.size(); }));

    std::string headerCols;
    for (std::size_t i = 0; i < columnNames.size(); ++i) {
        if (i > 0)
            headerCols += "  |  ";
        headerCols += columnNames[i];
    }
    const std::string header = std::format("  {}  |  {}", paramName, headerCols);
    const std::string separator(header.size(), '-');

    std::cout << separator << "\n" << header << "\n" << separator << "\n";

    for (std::size_t i = 0; i < params.size(); ++i) {
        std::cout << params[i];
        for (const auto& col : columns) {
            std::cout << " | " << col[i];
        }
        std::cout << "\n";
    }
}