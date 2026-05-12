#pragma once

#include <stdexcept>
#include <vector>

#include "numerics/linear/Segment.h"

namespace numerics::interpolation {

struct Coefficients {
    const double a{};
    const double b{};
    const double c{};
    const double d{};
};

class CubicFunction {
   public:
    CubicFunction(const double left, const double right, const double knot,
                  const Coefficients& coefficients)
        : _left(left), _right(right), _knot(knot), _coefficients(coefficients) {}
    ~CubicFunction() = default;
    double operator()(const double x) const {
        if (x < _left || x > _right) {
            throw std::invalid_argument("outside range of [" + std::to_string(_left) + "," +
                                        std::to_string(x) + "]: x=" + std::to_string(x));
        }
        const auto [a, b, c, d] = _coefficients;
        const auto h = x - _knot;
        return a + b * h + c * h * h + d * h * h * h;
    }

    double derivative(const double x) const {
        if (x < _left || x > _right) {
            throw std::invalid_argument("outside range of [" + std::to_string(_left) + "," +
                                        std::to_string(x) + "]: x=" + std::to_string(x));
        }
        const auto [a, b, c, d] = _coefficients;
        const auto h = x - _knot;
        return b + 2.0 * c * h + 3.0 * d * h * h;
    }

    double getLeft() const { return _left; }

    double getRight() const { return _right; }

   private:
    double _left;
    double _right;
    double _knot;  // knot x[i]: polynomial origin, separate from range bounds
    Coefficients _coefficients;
};

// Piecewise cubic interpolator with natural boundary conditions:
//   S''(x_0) = S''(x_{n-1}) = 0
// Coefficients are solved via the tridiagonal Thomas algorithm.
// Outside [x_0, x_{n-1}] the spline extrapolates linearly using the slope at each boundary knot.
class NaturalCubicSplineInterpolator {
   public:
    NaturalCubicSplineInterpolator(const std::vector<double>& x, const std::vector<double>& y)
        : _functions(buildFunctions(x, y)) {}
    ~NaturalCubicSplineInterpolator() = default;
    double operator()(const double x) const {
        for (const auto& function : _functions) {
            if (x < function.getRight()) {
                return function(x);
            }
        }
        throw std::invalid_argument("Invalid argument: x=" + std::to_string(x));
    }

    double derivative(const double x) const {
        for (const auto& function : _functions) {
            if (x < function.getRight()) {
                return function.derivative(x);
            }
        }
        throw std::invalid_argument("Invalid argument: x=" + std::to_string(x));
    }

   private:
    std::vector<CubicFunction> _functions;

    static std::vector<CubicFunction> buildFunctions(const std::vector<double>& x,
                                                     const std::vector<double>& y) {
        if (x.size() != y.size()) {
            throw std::invalid_argument("x and y must have the same length");
        }
        if (x.size() < 2) {
            throw std::invalid_argument(
                "NaturalCubicSplineInterpolator requires at least 2 points");
        }

        const auto n = x.size();

        // step 1
        std::vector<double> h(n - 1);

        for (size_t i = 0; i < n - 1; i++) {
            h[i] = x[i + 1] - x[i];
        }

        // step 2
        std::vector<double> alpha(n - 1);

        for (size_t i = 1; i < n - 1; i++) {
            alpha[i] = (3 / h[i]) * (y[i + 1] - y[i]) - (3 / h[i - 1]) * (y[i] - y[i - 1]);
        }

        // step 3
        std::vector<double> l(n);
        std::vector<double> mu(n);
        std::vector<double> z(n);

        l[0] = 1;
        mu[0] = 0;
        z[0] = 0;

        // step 4
        for (size_t i = 1; i < n - 1; ++i) {
            l[i] = 2 * (x[i + 1] - x[i - 1]) - h[i - 1] * mu[i - 1];
            mu[i] = h[i] / l[i];
            z[i] = (alpha[i] - h[i - 1] * z[i - 1]) / l[i];
        }

        // step 5
        l[n - 1] = 1;
        z[n - 1] = 0;

        // step 6
        std::vector<double> b(n);
        std::vector<double> c(n);
        std::vector<double> d(n);

        c[n - 1] = 0;

        for (auto j = static_cast<ptrdiff_t>(n - 2); j >= 0; j--) {
            c[j] = z[j] - mu[j] * c[j + 1];
            b[j] = (y[j + 1] - y[j]) / h[j] - h[j] * (c[j + 1] + 2 * c[j]) / 3;
            d[j] = (c[j + 1] - c[j]) / (3 * h[j]);
        }

        // step 7
        const double slopeRight =
            b[n - 2] + 2.0 * c[n - 2] * h[n - 2] + 3.0 * d[n - 2] * h[n - 2] * h[n - 2];

        std::vector<CubicFunction> functions;
        functions.emplace_back(linear::NEG_INF, x[0], x[0], Coefficients{y[0], b[0], 0.0, 0.0});
        for (size_t i = 0; i < n - 1; i++) {
            functions.emplace_back(x[i], x[i + 1], x[i], Coefficients{y[i], b[i], c[i], d[i]});
        }
        functions.emplace_back(x[n - 1], linear::POS_INF, x[n - 1],
                               Coefficients{y[n - 1], slopeRight, 0.0, 0.0});

        return functions;
    }
};
}  // namespace numerics::interpolation