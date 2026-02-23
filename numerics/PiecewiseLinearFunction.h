#pragma once

#include <algorithm>
#include <vector>

#include "Segment.h"

/**
 * Represents a piecewise linear function as a list of sorted segments.
 *
 * - PL is closed under +, -, max, min as well as * and / if one operand is constant.
 * - Assume that the most left endpoint is -inf and the most right endpoint is inf.
 */
namespace numerics::linear {
class PiecewiseLinearFunction {
   public:
    explicit PiecewiseLinearFunction(std::vector<Segment> segments)
        : _segments(std::move(segments)) {
        validate();
    }

    // Rule of five
    PiecewiseLinearFunction(const PiecewiseLinearFunction&) = default;
    PiecewiseLinearFunction(PiecewiseLinearFunction&&) = default;
    PiecewiseLinearFunction& operator=(const PiecewiseLinearFunction&) = default;
    PiecewiseLinearFunction& operator=(PiecewiseLinearFunction&&) = default;
    ~PiecewiseLinearFunction() = default;

    // static constructors
    static PiecewiseLinearFunction linear(double slope, double intercept, double lo,
                                                double hi);
    static PiecewiseLinearFunction linear(double slope, double intercept);
    static PiecewiseLinearFunction constant(double x);

    // Accessors
    const std::vector<Segment>& getSegments() const { return _segments; }
    std::vector<double> getBreakPoints() const;

    // operator
    double operator()(double x) const;

    // Arithmetic operators
    PiecewiseLinearFunction operator+(const PiecewiseLinearFunction& other) const;
    PiecewiseLinearFunction operator-() const;
    PiecewiseLinearFunction operator-(const PiecewiseLinearFunction& other) const;
    PiecewiseLinearFunction operator*(const PiecewiseLinearFunction& other) const;
    PiecewiseLinearFunction operator/(const PiecewiseLinearFunction& other) const;
    PiecewiseLinearFunction operator>(const PiecewiseLinearFunction& other) const;
    PiecewiseLinearFunction operator>=(const PiecewiseLinearFunction& other) const;

    // Max/Min
    static PiecewiseLinearFunction max(const PiecewiseLinearFunction& f,
                                       const PiecewiseLinearFunction& g);

    static PiecewiseLinearFunction min(const PiecewiseLinearFunction& f,
                                       const PiecewiseLinearFunction& g);

    std::string toString() const;

   private:
    void validate() const;

    // create a pair of new PLs with shared breakpoints
    static std::pair<PiecewiseLinearFunction, PiecewiseLinearFunction> align(
        const PiecewiseLinearFunction& f, const PiecewiseLinearFunction& g);

    // creates a new PL by merging adjacent segments if possible
    PiecewiseLinearFunction merged() const;

    // returns a sorted union of breakpoints of two functions
    static std::vector<double> mergedBreakPoints(const PiecewiseLinearFunction& f,
                                                 const PiecewiseLinearFunction& g);

    // create a new PL with additional breakpoints
    PiecewiseLinearFunction withBreakPoints(const std::vector<double>& breakPoints) const;

    static PiecewiseLinearFunction applyMaxMin(const PiecewiseLinearFunction& f,
                                               const PiecewiseLinearFunction& g, bool isMax);

    static PiecewiseLinearFunction greaterThanInner(const PiecewiseLinearFunction& f,
        const PiecewiseLinearFunction& g, bool isStrict);

    std::vector<Segment> _segments;
};
}  // namespace numerics::pwl
