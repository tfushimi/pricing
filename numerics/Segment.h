#pragma once
#include <optional>
#include <stdexcept>
#include <string>

#include "types.h"

namespace numerics::linear {

/**
 * Represents f(S) = slope * S + intercept for S in [lo, hi)
 *
 * Convention: [lo, hi) - closed left, open right.
 * This guarantees every finite point belongs to exactly one segment.
 */
class Segment {
   public:
    Segment(const double slope, const double intercept, const double left, const double right)
        : _slope(slope), _intercept(intercept), _lo(left), _hi(right) {
        if (_hi < _lo) {
            throw std::invalid_argument("Invalid endpoints: left=" + std::to_string(_lo) +
                                        ",right=" + std::to_string(_hi));
        }
    }
    ~Segment() = default;
    Segment(const Segment& other) = default;
    Segment& operator=(const Segment& other) = default;

    // Accessors
    double getSlope() const { return _slope; }
    double getIntercept() const { return _intercept; }
    double getLeft() const { return _lo; }
    double getRight() const { return _hi; }

    // Containment
    bool contains(const double x) const { return _lo <= x && (_hi > x || x == POS_INF); }
    bool containsInterior(const double x) const { return _lo < x && x < _hi; }

    double operator()(const double x) const {
        if (contains(x)) {
            return _intercept + _slope * x;
        }
        return 0;
    }

    std::optional<double> crossing(const Segment& other) const {
        if (_lo != other._lo || _hi != other._hi) {
            throw std::invalid_argument("endpoints do not match: this=" + toString() +
                                        ",other=" + other.toString());
        }

        if (_slope == other._slope) {
            return std::nullopt;
        }

        const double dslope = _slope - other._slope;

        if (std::abs(dslope) < 1e-12) {
            return std::nullopt;
        }

        const double crossing = (other._intercept - _intercept) / dslope;

        if (containsInterior(crossing)) {
            return crossing;
        }

        return std::nullopt;
    }

    // A representative interior point, safe for ±inf endpoints.
    // Used to determine which of two segments dominates when there is
    // no crossing (or to determine which side of a crossing dominates).
    double midpoint() const {
        if (_lo == NEG_INF && _hi == POS_INF)
            return 0.0;
        if (_lo == NEG_INF)
            return _hi - 1.0;
        if (_hi == POS_INF)
            return _lo + 1.0;
        return _lo + (_hi - _lo) / 2.0;  // avoids (lo+hi)/2 overflow
    }

    // Midpoint of [lo, x), safe for lo == -inf.
    double midpointLeft(const double x) const {
        if (_lo == NEG_INF)
            return x - 1.0;
        return _lo + (x - _lo) / 2.0;
    }

    // Midpoint of [x, hi), safe for hi == +inf.
    double midpointRight(const double x) const {
        if (_hi == POS_INF)
            return x + 1.0;
        return x + (_hi - x) / 2.0;
    }

    std::string toString() const {
        return "Segment[intercept=" + std::to_string(_intercept) +
               +",slope=" + std::to_string(_slope) + ",left=" + std::to_string(_lo) +
               ",right=" + std::to_string(_hi) + "]";
    }

   private:
    double _slope;
    double _intercept;
    double _lo;
    double _hi;
};
}  // namespace numerics::linear
