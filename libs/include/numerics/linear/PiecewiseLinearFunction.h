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
    using const_iterator = std::vector<Segment>::const_iterator;

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
    static PiecewiseLinearFunction linear(double slope, double intercept, double lo, double hi);
    static PiecewiseLinearFunction linear(double slope, double intercept);
    static PiecewiseLinearFunction constant(double x);

    // iterator
    const_iterator begin() const { return _segments.begin(); }
    const_iterator end() const { return _segments.end(); }

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

    // Max/Min/IfThenElse
    static PiecewiseLinearFunction max(const PiecewiseLinearFunction& f,
                                       const PiecewiseLinearFunction& g);

    static PiecewiseLinearFunction min(const PiecewiseLinearFunction& f,
                                       const PiecewiseLinearFunction& g);

    static PiecewiseLinearFunction ite(const PiecewiseLinearFunction& cond,
                                       const PiecewiseLinearFunction& then_,
                                       const PiecewiseLinearFunction& else_);

    std::string toString() const;

    int numSegments() const { return _segments.size(); };

    int numBreakPoints() const { return _segments.size() - 1; };

   private:
    void validate() const;

    // Accessors
    const std::vector<Segment>& getSegments() const { return _segments; }
    std::vector<double> getBreakPoints() const;

    // create a pair of new PLs with shared breakpoints
    static std::pair<PiecewiseLinearFunction, PiecewiseLinearFunction> align(
        const PiecewiseLinearFunction& f, const PiecewiseLinearFunction& g);

    // create a tuple of new PLs with shared breakpoints
    static std::tuple<PiecewiseLinearFunction, PiecewiseLinearFunction, PiecewiseLinearFunction>
    align3(const PiecewiseLinearFunction& f, const PiecewiseLinearFunction& g,
           const PiecewiseLinearFunction& h);

    // creates a new PL by merging adjacent segments if possible
    PiecewiseLinearFunction merged() const;

    // create a new PL with additional breakpoints
    PiecewiseLinearFunction withBreakPoints(const std::vector<double>& breakPoints) const;

    static PiecewiseLinearFunction applyMaxMin(const PiecewiseLinearFunction& f,
                                               const PiecewiseLinearFunction& g, bool isMax);

    static PiecewiseLinearFunction greaterThanInner(const PiecewiseLinearFunction& f,
                                                    const PiecewiseLinearFunction& g,
                                                    bool isStrict);

    std::vector<Segment> _segments;
};
}  // namespace numerics::linear
