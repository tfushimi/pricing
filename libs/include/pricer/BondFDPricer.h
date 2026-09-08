/*
 * Crank-Nicolson finite differences for the affine term-structure PDE.
 *
 * F_tau = mu(r) * F_r + 0.5 * var(r) * F_rr - r * F, F(0, r) = 1
 *
 * central differences inside; at both ends F_rr = 0 is imposed
 * The triagonal systems are solved by the Thomas algorithm.
 */
#pragma once
#include <algorithm>
#include <cmath>
#include <vector>

#include "BondFormula.h"

namespace pricer::bond {

struct Grid {
    std::vector<double> r;
    int i0;    // index of the node sitting exactly on r0
    double dr; // finite-difference
};

inline Grid makeGrid(const double r0, const double rMin, const double rMax, const int m) {
    const double drNominal = (rMax - rMin) / m;
    const int nLow = std::max(1, static_cast<int>(std::lround((r0 - rMin) / drNominal)));
    const double dr = (r0 - rMin) / nLow;
    const int nHigh = static_cast<int>(std::ceil((rMax - r0) / dr));
    Grid grid{.r = {}, .i0 = nLow, .dr = dr};
    grid.r.reserve(nLow + nHigh + 1);
    for (int i = -nLow; i <= nHigh; i++) {
        grid.r.push_back(r0 + dr * i);
    }
    grid.r[nLow] = r0; // exact, free of accumulated rounding
    grid.r.front() = rMin;
    return grid;
}

// Tridiagonal bands of L (down[i] * F[i-1], di[i] * F[i], up[i] * F[i+1])
inline void buildBands(const Affine& affine, const Grid& grid, std::vector<double>& down, std::vector<double>& di, std::vector<double>& up) {

    const int m = static_cast<int>(grid.r.size());
    const double dr = grid.dr;
    const double dr2 = dr * dr;
    down.assign(m, 0.0);
    di.assign(m, 0.0);
    up.assign(m, 0.0);

    for (int i = 0; i < m; i++) {
        const double mu = affine.b + affine.beta * grid.r[i];
        const double var = std::max(affine.a + affine.alpha * grid.r[i], 0.0);

        down[i] = 0.5 * var / dr2 - mu / (2 * dr);
        di[i] = -var / dr2 - grid.r[i];
        up[i] = 0.5 * var / dr2 + mu / (2 * dr);
    }

    const double mu0 = affine.b + affine.beta * grid.r.front();
    const double muM = affine.b + affine.beta * grid.r.back();
    di.front() = -mu0 / dr - grid.r.front();
    up.front() = mu0/dr;
    di.back() = muM / dr - grid.r.back();
    down.back() = -muM / dr;
}

// Thomas algorithm to solve a tridiagonal system in place (bands are copied)
inline void thomasSolver(std::vector<double> down, std::vector<double> di, std::vector<double> up, std::vector<double>& x) {

    const int m = static_cast<int>(x.size());

    // forward elimination
    for (int i = 1; i < m; i++) {
        const double w = down[i] / di[i-1];
        di[i] -= w * up[i-1];
        x[i] -= w * x[i-1];
    }

    // backward substitution
    x[m-1] /= di[m-1];
    for (int i = m-2; i >= 0; i--) {

        x[i] = (x[i] - up[i] * x[i+1]) / di[i];
    }
}

// One Crank-Nicolson step: (I - 0.5 * dt * L) * F_new = (I + 0.5 * dt * L) * F_old
inline void crankNicolsonStep(const std::vector<double>& down, const std::vector<double>& di, std::vector<double>& up, const double dt, std::vector<double>& F) {
    const int m = static_cast<int>(F.size());
    std::vector<double> rhs(m);
    for (int i = 0; i < m; i++) {
        double LF = di[i] * F[i];
        if (i > 0) {
            LF += down[i] * F[i-1];
        }
        if (i + 1 < m) {
            LF += up[i] * F[i+1];
        }
        rhs[i] = F[i] + 0.5 * dt * LF;
    }

    std::vector<double> l(m), d(m), u(m);

    for (int i = 0; i < m; i++) {
        l[i] = -0.5 * dt * down[i];
        d[i] = 1.0 - 0.5 * dt * di[i];
        u[i] = -0.5 * dt * up[i];
    }
    thomasSolver(l, d, u, rhs);
    F = rhs;
}

inline double bondFDPricer(const Affine& affine, const double r0, double T, double r_min, double r_max, const int m = 400, const int n = 400) {
    const Grid grid = makeGrid(r0, r_min, r_max, m);
    std::vector<double> down, di, up;
    buildBands(affine, grid, down, di, up);
    std::vector F(grid.r.size(), 1.0);
    for (int i = 0; i < n; i++) {
       crankNicolsonStep(down, di, up, T / n, F);
    }

    return F[grid.i0];
}
}
