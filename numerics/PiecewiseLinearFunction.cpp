#include "PiecewiseLinearFunction.h"

#include <set>

#include "types.h"

using namespace numerics::pwl;

PiecewiseLinearFunction PiecewiseLinearFunction::createLinear(const double intercept, const double slope, const double left, const double right) {

    std::vector<Segment> segments;
    segments.push_back(Segment(slope, intercept, left, right));
    return PiecewiseLinearFunction(segments);
}

PiecewiseLinearFunction PiecewiseLinearFunction::createConstant(const double x) {

    std::vector<Segment> segments;
    segments.push_back(Segment(0, x, MIN_VALUE, MAX_VALUE));
    return PiecewiseLinearFunction(segments);
}

std::vector<double> PiecewiseLinearFunction::getBreakPoints() const {

    std::vector<double> breakPoints;

    for (int i = 0; i < _segments.size()-1; ++i) {

        breakPoints.push_back(_segments[i].getRight());
    }

    return breakPoints;
}

double PiecewiseLinearFunction::operator()(const double x) const {
    // TODO binary search?
    for (const auto& segment : _segments) {

        if (segment.contains(x)) {

            return segment(x);
        }
    }

    return _segments.back().getSlope();
}

PiecewiseLinearFunction PiecewiseLinearFunction::operator+(const PiecewiseLinearFunction &other) const {

    auto [first, second] = align(*this, other);

    std::vector<Segment> segments;

    for (int i = 0; i < first._segments.size(); ++i) {

        Segment& seg1 = first._segments[i];
        Segment& seg2 = second._segments[i];

        segments.push_back(Segment(seg1.getSlope() + seg2.getSlope(), seg1.getIntercept() + seg2.getIntercept(), seg1.getLeft(), seg1.getRight()));
    }

    auto result = PiecewiseLinearFunction(segments);

    result.merge();

    return result;
}

PiecewiseLinearFunction PiecewiseLinearFunction::operator-(const PiecewiseLinearFunction& other) const {

    const PiecewiseLinearFunction negativeOther = createConstant(-1.0) * other;

    return *this + negativeOther;
}

PiecewiseLinearFunction PiecewiseLinearFunction::operator*(const PiecewiseLinearFunction& other) const {

    auto [first, second] = align(*this, other);

    std::vector<Segment> segments;

    for (int i = 0; i < first._segments.size(); ++i) {

        Segment& seg1 = first._segments[i];
        Segment& seg2 = second._segments[i];

        if (seg1.getSlope() == 0) {

            segments.push_back(Segment(seg1.getIntercept() * seg2.getSlope(), seg1.getIntercept() * seg2.getIntercept(), seg1.getLeft(), seg1.getRight()));
        }
        else if (seg2.getSlope() == 0) {

            segments.push_back(Segment(seg1.getSlope() * seg2.getIntercept(), seg1.getIntercept() * seg2.getIntercept(), seg1.getLeft(), seg1.getRight()));
        }
        else {

            throw std::invalid_argument("Product of two non-constant segments is not linear ");
        }
    }

    auto result = PiecewiseLinearFunction(segments);

    result.merge();

    return result;
}

PiecewiseLinearFunction PiecewiseLinearFunction::operator/(const PiecewiseLinearFunction& other) const {

    auto [first, second] = align(*this, other);

    std::vector<Segment> segments;

    for (int i = 0; i < first._segments.size(); ++i) {

        Segment& seg1 = first._segments[i];
        Segment& seg2 = second._segments[i];

        if (seg2.getSlope() != 0) {

            throw std::invalid_argument("Cannot divide PWL by PWL");
        }

        if (seg2.getIntercept() == 0) {

            throw std::invalid_argument("Cannot divide PWL by zero");
        }

        segments.push_back(Segment(seg1.getSlope() / seg2.getIntercept(), seg1.getIntercept() / seg2.getIntercept(), seg1.getLeft(), seg1.getRight()));
    }

    auto result = PiecewiseLinearFunction(segments);

    result.merge();

    return result;
}
//
// static PiecewiseLinearFunction PiecewiseLinearFunction::max(const PiecewiseLinearFunction& f, const PiecewiseLinearFunction& g) {
//
//     return nullptr;
// }
//
// static PiecewiseLinearFunction PiecewiseLinearFunction::min(const PiecewiseLinearFunction& f, const PiecewiseLinearFunction& g) {
//
//     return nullptr;
// }

// align both f and g to share breakpoints
std::pair<PiecewiseLinearFunction, PiecewiseLinearFunction> PiecewiseLinearFunction::align(const PiecewiseLinearFunction& f, const PiecewiseLinearFunction& g) {

    // internally sorted
    std::set<double> uniqueBreakPoints;

    uniqueBreakPoints.insert(f.getBreakPoints().begin(), f.getBreakPoints().end());
    uniqueBreakPoints.insert(g.getBreakPoints().begin(), g.getBreakPoints().end());

    const std::vector breakPoints(uniqueBreakPoints.begin(), uniqueBreakPoints.end());

    auto newF = f.insert(breakPoints);
    auto newG = g.insert(breakPoints);

    return std::make_pair(newF, newG);
}

PiecewiseLinearFunction PiecewiseLinearFunction::insert(const std::vector<double>& breakPoints) const {

    std::vector<Segment> segments;

    for (auto segment : _segments) {

        std::vector<double> innerBreakPoints;

        innerBreakPoints.push_back(segment.getLeft());

        for (double breakPoint : breakPoints) {

            if (segment.contains(breakPoint)) {

                innerBreakPoints.push_back(breakPoint);
            }
        }

        innerBreakPoints.push_back(segment.getRight());

        for (int i = 0; i < innerBreakPoints.size() - 1; ++i) {

            segments.push_back(Segment(segment.getSlope(), segment.getIntercept(), innerBreakPoints[i], innerBreakPoints[i + 1]));
        }
    }

    return PiecewiseLinearFunction(segments);
}

PiecewiseLinearFunction& PiecewiseLinearFunction::merge() {

    if (_segments.size() == 0) {

        return *this;
    }

    std::vector<Segment> segments;

    segments.push_back(_segments[0]);

    for (int i = 1; i < _segments.size(); ++i) {

        Segment& curr = _segments[i];
        Segment& prev = segments.back();

        if (prev.getSlope() == curr.getSlope() && prev.getIntercept() == curr.getIntercept()) {

            prev = Segment(prev.getSlope(), prev.getIntercept(), prev.getLeft(), curr.getRight());
        }
        else {

            segments.push_back(curr);
        }
    }

    _segments = std::move(segments);

    return *this;
}