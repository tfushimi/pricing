#pragma once

#include <stdexcept>
#include <vector>

#include "numerics/linear/PiecewiseLinearFunction.h"
#include "numerics/linear/Segment.h"

namespace numerics::interpolation {

// Piecewise linear interpolator over a set of (x, y) knots. Linearly extrapolates beyond
// the leftmost and rightmost knots using the slope of the nearest segment.
class LinearInterpolator {
   public:
    LinearInterpolator(const std::vector<double>& x, const std::vector<double>& y)
        : _function(buildSegments(x, y)) {}

    double operator()(const double x) const { return _function(x); }

   private:
    linear::PiecewiseLinearFunction _function;

    static std::vector<linear::Segment> buildSegments(const std::vector<double>& x,
                                                      const std::vector<double>& y) {
        if (x.size() != y.size()) {
            throw std::invalid_argument("x and y must have the same length");
        }
        if (x.size() < 2) {
            throw std::invalid_argument("LinearInterpolator requires at least 2 points");
        }

        std::vector<linear::Segment> segments;
        segments.reserve(x.size() - 1);

        for (std::size_t i = 0; i < x.size() - 1; ++i) {
            if (x[i] >= x[i + 1]) {
                throw std::invalid_argument("x values must be strictly increasing");
            }

            const double slope = (y[i + 1] - y[i]) / (x[i + 1] - x[i]);
            const double intercept = y[i] - slope * x[i];
            const double left = i == 0 ? linear::NEG_INF : x[i];
            const double right = i == x.size() - 2 ? linear::POS_INF : x[i + 1];
            segments.emplace_back(slope, intercept, left, right);
        }

        return segments;
    }
};

}  // namespace numerics::interpolation
