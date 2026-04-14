/**
 * @file ImpliedVolCurves.cpp
 *
 * Computes the Heston implied volatility smile across a range of log-strikes,
 * reproducing Figure 4.3 of Gatheral (2006), "The Volatility Surface".
 *
 * For a given maturity T, the implied volatility is computed by:
 *   1. Pricing a European call via the Heston characteristic function.
 *   2. Inverting the Black formula via bisection to recover implied vol.
 *
 * Heston-Nandi parameters: v0=0.04, kappa=10, theta=0.04, xi=1, rho=-1.
 * Assumptions: zero interest rates and dividends (dF=1, F=S).
 */
#include <vector>

#include "HestonNandi.h"
#include "common/TableUtils.h"
#include "pricer/ImpliedVol.h"

using namespace std;
using namespace pricer;

int main() {
    constexpr double F = 100.0;
    constexpr double dF = 1.0;  // zero interest rates
    constexpr double T = 0.5;
    constexpr int n = 60;

    // log-strike k = log(K/F) from -0.6 to 0.2
    vector<double> logStrikes, vols;
    logStrikes.reserve(n);
    vols.reserve(n);

    for (int i = 0; i < n; ++i) {
        const double k = -0.6 + i * (0.8 / (n - 1));
        const double K = F * std::exp(k);
        logStrikes.push_back(k);
        vols.push_back(hestonImpliedVol(F, K, T, dF, hestonParams));
    }

    printTable({"logStrike", "impliedVol"}, {logStrikes, vols});

    return 0;
}