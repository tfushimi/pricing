#pragma once

#include <vector>

#include "Segment.h"

namespace numerics::pwl {
    class PiecewiseLinearFunction {
    public:
        explicit PiecewiseLinearFunction(const std::vector<Segment>& segments) : _segments(segments) {}
        ~PiecewiseLinearFunction() {}
        PiecewiseLinearFunction(const PiecewiseLinearFunction& other) : _segments(other._segments) {}
        PiecewiseLinearFunction& operator=(const PiecewiseLinearFunction& other) {
            _segments = other._segments;
            return *this;
        }
        static PiecewiseLinearFunction createLinear(double intercept, double slope, double left, double right);
        static PiecewiseLinearFunction createConstant(double x);
        std::vector<double> getBreakPoints() const;
        const std::vector<Segment>& getSegments() const {
            return _segments;
        }
        double operator()(double x) const;
        PiecewiseLinearFunction operator+(const PiecewiseLinearFunction& other) const;
        PiecewiseLinearFunction operator-(const PiecewiseLinearFunction& other) const;
        PiecewiseLinearFunction operator*(const PiecewiseLinearFunction& other) const;
        PiecewiseLinearFunction operator/(const PiecewiseLinearFunction& other) const;
        static PiecewiseLinearFunction max(const PiecewiseLinearFunction& f, const PiecewiseLinearFunction& g);
        static PiecewiseLinearFunction min(const PiecewiseLinearFunction& f, const PiecewiseLinearFunction& g);
        // align both f and g to share breakpoints
        static std::pair<PiecewiseLinearFunction, PiecewiseLinearFunction> align(const PiecewiseLinearFunction& f, const PiecewiseLinearFunction& g);
        // insert breakpoints inside this segment
        PiecewiseLinearFunction insert(const std::vector<double>& breakPoints) const;
        // remove redundant breakpoints
        PiecewiseLinearFunction& merge();
    private:

        std::vector<Segment> _segments;
    };
}
