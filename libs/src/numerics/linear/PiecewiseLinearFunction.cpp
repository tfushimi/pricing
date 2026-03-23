#include "numerics/linear/PiecewiseLinearFunction.h"

namespace numerics::linear {

PiecewiseLinearFunction PiecewiseLinearFunction::linear(const double slope, const double intercept,
                                                        const double lo, const double hi) {
    return PiecewiseLinearFunction({
        Segment(0.0, 0.0, NEG_INF, lo),
        Segment(slope, intercept, lo, hi),
        Segment(0.0, 0.0, hi, POS_INF),
    });
}

PiecewiseLinearFunction PiecewiseLinearFunction::linear(const double slope,
                                                        const double intercept) {
    return PiecewiseLinearFunction({Segment(slope, intercept, NEG_INF, POS_INF)});
}

PiecewiseLinearFunction PiecewiseLinearFunction::constant(const double x) {
    return PiecewiseLinearFunction({Segment(0.0, x, NEG_INF, POS_INF)});
}

std::vector<double> PiecewiseLinearFunction::getBreakPoints() const {
    // the most left and right breakpoints are -inf and +inf

    std::vector<double> breakPoints;

    breakPoints.reserve(getSegments().size() - 1);

    for (std::size_t i = 0; i < _segments.size() - 1; ++i) {
        breakPoints.push_back(_segments[i].getRight());
    }

    return breakPoints;
}

double PiecewiseLinearFunction::operator()(const double x) const {
    // Linear scan because #segments is typically small.
    for (const auto& segment : _segments) {
        if (segment.contains(x)) {
            return segment(x);
        }
    }

    // Should never reach here given validated segments cover (-inf, +inf).
    return _segments.back()(x);
}

// Arithmetic operators
PiecewiseLinearFunction PiecewiseLinearFunction::operator+(
    const PiecewiseLinearFunction& other) const {
    auto [f, g] = align(*this, other);

    std::vector<Segment> segments;

    for (std::size_t i = 0; i < f._segments.size(); ++i) {
        Segment& segmentF = f._segments[i];
        Segment& segmentG = g._segments[i];

        segments.emplace_back(segmentF.getSlope() + segmentG.getSlope(),
                              segmentF.getIntercept() + segmentG.getIntercept(), segmentF.getLeft(),
                              segmentF.getRight());
    }

    return PiecewiseLinearFunction(std::move(segments)).merged();
}

PiecewiseLinearFunction PiecewiseLinearFunction::operator-() const {
    std::vector<Segment> segments;

    for (const auto segment : _segments) {
        segments.emplace_back(-segment.getSlope(), -segment.getIntercept(), segment.getLeft(),
                              segment.getRight());
    }

    return PiecewiseLinearFunction(std::move(segments));
}

PiecewiseLinearFunction PiecewiseLinearFunction::operator-(
    const PiecewiseLinearFunction& other) const {
    return *this + (-other);
}

PiecewiseLinearFunction PiecewiseLinearFunction::operator*(
    const PiecewiseLinearFunction& other) const {
    auto [f, g] = align(*this, other);

    std::vector<Segment> segments;
    segments.reserve(f._segments.size());

    for (std::size_t i = 0; i < f._segments.size(); ++i) {
        const auto& segmentF = f._segments[i];
        const auto& segmentG = g._segments[i];

        if (segmentF.getSlope() == 0.0) {
            const double c = segmentF.getIntercept();
            segments.emplace_back(c * segmentG.getSlope(), c * segmentG.getIntercept(),
                                  segmentF.getLeft(), segmentF.getRight());
        } else if (segmentG.getSlope() == 0.0) {
            const double c = segmentG.getIntercept();
            segments.emplace_back(c * segmentF.getSlope(), c * segmentF.getIntercept(),
                                  segmentF.getLeft(), segmentF.getRight());
        } else {
            throw std::invalid_argument(
                "operator*: product of two non-constant segments is not linear");
        }
    }

    return PiecewiseLinearFunction(std::move(segments)).merged();
}

PiecewiseLinearFunction PiecewiseLinearFunction::operator/(
    const PiecewiseLinearFunction& other) const {
    auto [f, g] = align(*this, other);

    std::vector<Segment> segments;

    segments.reserve(f._segments.size());

    for (std::size_t i = 0; i < f._segments.size(); ++i) {
        const auto& segmentF = f._segments[i];
        const auto& segmentG = g._segments[i];

        if (segmentG.getSlope() != 0.0) {
            throw std::invalid_argument("operator/: cannot divide by a non-constant PWL segment");
        }

        if (segmentG.getIntercept() == 0.0) {
            throw std::invalid_argument("operator/: division by zero");
        }

        const double c = segmentG.getIntercept();

        segments.emplace_back(segmentF.getSlope() / c, segmentF.getIntercept() / c,
                              segmentF.getLeft(), segmentF.getRight());
    }

    return PiecewiseLinearFunction(std::move(segments)).merged();
}

PiecewiseLinearFunction PiecewiseLinearFunction::operator>(
    const PiecewiseLinearFunction& other) const {
    return greaterThanInner(*this, other, true);
}

PiecewiseLinearFunction PiecewiseLinearFunction::operator>=(
    const PiecewiseLinearFunction& other) const {
    return greaterThanInner(*this, other, false);
}

// Compound Assignment Operators
PiecewiseLinearFunction PiecewiseLinearFunction::operator+=(const PiecewiseLinearFunction& other) {
    *this = *this + other;
    return *this;
}


// Max/Min
PiecewiseLinearFunction PiecewiseLinearFunction::max(const PiecewiseLinearFunction& f,
                                                     const PiecewiseLinearFunction& g) {
    return applyMaxMin(f, g, true);
}

PiecewiseLinearFunction PiecewiseLinearFunction::min(const PiecewiseLinearFunction& f,
                                                     const PiecewiseLinearFunction& g) {
    return applyMaxMin(f, g, false);
}

PiecewiseLinearFunction PiecewiseLinearFunction::ite(const PiecewiseLinearFunction& cond,
                                                     const PiecewiseLinearFunction& then_,
                                                     const PiecewiseLinearFunction& else_) {
    auto [condPL, thenPL, elsePL] = align3(cond, then_, else_);

    std::vector<Segment> segments;

    for (size_t i = 0; i < thenPL._segments.size(); ++i) {
        const auto& segmentCond = condPL._segments[i];

        if (segmentCond.getSlope() != 0.0) {
            throw std::invalid_argument("condition should be constant");
        }

        const auto& segment =
            segmentCond.getIntercept() > 0.0 ? thenPL._segments[i] : elsePL._segments[i];

        segments.emplace_back(segment.getSlope(), segment.getIntercept(), segment.getLeft(),
                              segment.getRight());
    }

    return PiecewiseLinearFunction(std::move(segments)).merged();
}

std::string PiecewiseLinearFunction::toString() const {
    std::string result;

    for (const auto& segment : _segments) {
        result += segment.toString();
    }

    return result;
}

void PiecewiseLinearFunction::validate() const {
    if (_segments.empty()) {
        throw std::invalid_argument("PiecewiseLinearFunction: no segments");
    }
    if (_segments.front().getLeft() != NEG_INF) {
        throw std::invalid_argument("PiecewiseLinearFunction: first segment must start at -inf");
    }
    if (_segments.back().getRight() != POS_INF) {
        throw std::invalid_argument("PiecewiseLinearFunction: last segment must end at +inf");
    }
    for (std::size_t i = 0; i + 1 < _segments.size(); i++) {
        if (_segments[i].getRight() != _segments[i + 1].getLeft()) {
            throw std::invalid_argument("PiecewiseLinearFunction: segments are not contiguous");
        }
    }
}

// create a pair of new PLs with shared breakpoints
std::pair<PiecewiseLinearFunction, PiecewiseLinearFunction> PiecewiseLinearFunction::align(
    const PiecewiseLinearFunction& f, const PiecewiseLinearFunction& g) {
    // collect sorted breakpoints
    const auto breakPointsF = f.getBreakPoints();
    const auto breakPointsG = g.getBreakPoints();

    std::vector<double> breakPoints;

    breakPoints.reserve(breakPointsF.size() + breakPointsG.size());

    std::set_union(breakPointsF.begin(), breakPointsF.end(), breakPointsG.begin(),
                   breakPointsG.end(), std::back_inserter(breakPoints));

    return {f.withBreakPoints(breakPoints), g.withBreakPoints(breakPoints)};
}

std::tuple<PiecewiseLinearFunction, PiecewiseLinearFunction, PiecewiseLinearFunction>
PiecewiseLinearFunction::align3(const PiecewiseLinearFunction& f, const PiecewiseLinearFunction& g,
                                const PiecewiseLinearFunction& h) {
    // collect sorted breakpoints
    const auto breakPointsF = f.getBreakPoints();
    const auto breakPointsG = g.getBreakPoints();
    const auto breakPointsH = h.getBreakPoints();

    std::vector<double> temp;

    temp.reserve(breakPointsF.size() + breakPointsG.size());

    std::set_union(breakPointsF.begin(), breakPointsF.end(), breakPointsG.begin(),
                   breakPointsG.end(), std::back_inserter(temp));

    std::vector<double> breakPoints;

    breakPoints.reserve(temp.size() + breakPointsH.size());

    std::set_union(temp.begin(), temp.end(), breakPointsH.begin(), breakPointsH.end(),
                   std::back_inserter(breakPoints));

    return {f.withBreakPoints(breakPoints), g.withBreakPoints(breakPoints),
            h.withBreakPoints(breakPoints)};
}

// creates a new PL by merging adjacent segments if possible
PiecewiseLinearFunction PiecewiseLinearFunction::merged() const {
    if (_segments.empty()) {
        return *this;
    }

    std::vector<Segment> segments;

    segments.push_back(_segments[0]);

    for (std::size_t i = 1; i < _segments.size(); i++) {
        const Segment& curr = _segments[i];
        Segment& prevSegment = segments.back();

        if (prevSegment.getSlope() == curr.getSlope() &&
            prevSegment.getIntercept() == curr.getIntercept()) {
            prevSegment = Segment(prevSegment.getSlope(), prevSegment.getIntercept(),
                                  prevSegment.getLeft(), curr.getRight());
        } else {
            segments.push_back(curr);
        }
    }

    return PiecewiseLinearFunction(std::move(segments));
}

// create a new PL with additional breakpoints
PiecewiseLinearFunction PiecewiseLinearFunction::withBreakPoints(
    const std::vector<double>& breakPoints) const {
    std::vector<Segment> segments;

    for (const auto& segment : _segments) {
        // Collect interior breakpoints for this segment
        std::vector points = {segment.getLeft()};

        for (double bp : breakPoints) {
            if (segment.containsInterior(bp)) {
                points.push_back(bp);
            }
        }

        points.push_back(segment.getRight());

        // pts is already sorted because breakPoints is sorted and we only
        // take interior points, but sort defensively
        std::sort(points.begin(), points.end());

        for (std::size_t i = 0; i + 1 < points.size(); ++i) {
            segments.emplace_back(segment.getSlope(), segment.getIntercept(), points[i],
                                  points[i + 1]);
        }
    }

    return PiecewiseLinearFunction(std::move(segments));
}

PiecewiseLinearFunction PiecewiseLinearFunction::applyMaxMin(const PiecewiseLinearFunction& f,
                                                             const PiecewiseLinearFunction& g,
                                                             const bool isMax) {
    auto [fa, ga] = align(f, g);
    std::vector<Segment> segments;
    segments.reserve(fa._segments.size());

    for (std::size_t i = 0; i < fa._segments.size(); ++i) {
        const auto& segmentF = fa._segments[i];
        const auto& segmentG = ga._segments[i];
        const auto crossingPoint = segmentF.crossing(segmentG);

        if (!crossingPoint) {
            // No crossing: one segment dominates throughout
            const double midpoint = segmentF.midpoint();
            const bool fWins = isMax ? segmentF(midpoint) >= segmentG(midpoint)
                                     : segmentF(midpoint) <= segmentG(midpoint);
            const auto& winner = fWins ? segmentF : segmentG;
            segments.emplace_back(winner.getSlope(), winner.getIntercept(), segmentF.getLeft(),
                                  segmentF.getRight());
        } else {
            // Crossing at x: split into [lo, x) and [x, hi)
            const double splitAt = *crossingPoint;
            const double midLeft = segmentF.midpointLeft(splitAt);
            const double midRight = segmentF.midpointRight(splitAt);

            const bool fWinsLeft = isMax ? segmentF(midLeft) >= segmentG(midLeft)
                                         : segmentF(midLeft) <= segmentG(midLeft);
            const bool fWinsRight = isMax ? segmentF(midRight) >= segmentG(midRight)
                                          : segmentF(midRight) <= segmentG(midRight);

            const auto& winnerLeft = fWinsLeft ? segmentF : segmentG;
            const auto& winnerRight = fWinsRight ? segmentF : segmentG;

            segments.emplace_back(winnerLeft.getSlope(), winnerLeft.getIntercept(),
                                  segmentF.getLeft(), splitAt);
            segments.emplace_back(winnerRight.getSlope(), winnerRight.getIntercept(), splitAt,
                                  segmentF.getRight());
        }
    }

    return PiecewiseLinearFunction(std::move(segments)).merged();
}

PiecewiseLinearFunction PiecewiseLinearFunction::greaterThanInner(const PiecewiseLinearFunction& f,
                                                                  const PiecewiseLinearFunction& g,
                                                                  const bool isStrict) {
    auto [plfF, plfG] = align(f, g);

    std::vector<Segment> segments;
    segments.reserve(plfF._segments.size());

    for (std::size_t i = 0; i < plfF._segments.size(); ++i) {
        const auto& segmentF = plfF._segments[i];
        const auto& segmentG = plfG._segments[i];
        const auto crossingPoint = segmentF.crossing(segmentG);

        if (!crossingPoint) {
            // No crossing: one segment dominates throughout
            const double midpoint = segmentF.midpoint();
            const bool fWins = isStrict ? segmentF(midpoint) > segmentG(midpoint)
                                        : segmentF(midpoint) >= segmentG(midpoint);
            segments.emplace_back(0.0, fWins ? 1.0 : 0.0, segmentF.getLeft(), segmentF.getRight());
        } else {
            // Crossing at x:
            // if isStrict = true, split into [lo, x + epsilon) and [x + epsilon, hi)
            // if isStrict = false, split into [lo, x) and [x, hi)
            const double splitAt = isStrict ? *crossingPoint + 1e-6 : *crossingPoint;

            // Validate split is still inside the segment
            if (!segmentF.containsInterior(splitAt)) {
                // crossing is so close to a boundary that epsilon pushes it out, fall back to
                // midpoint evaluation
                const double mid = segmentF.midpoint();
                const bool fWins =
                    isStrict ? segmentF(mid) > segmentG(mid) : segmentF(mid) >= segmentG(mid);
                segments.emplace_back(0.0, fWins ? 1.0 : 0.0, segmentF.getLeft(),
                                      segmentF.getRight());
                continue;
            }

            const double midLeft = segmentF.midpointLeft(splitAt);
            const double midRight = segmentF.midpointRight(splitAt);

            const bool fWinsLeft = isStrict ? segmentF(midLeft) > segmentG(midLeft)
                                            : segmentF(midLeft) >= segmentG(midLeft);
            const bool fWinsRight = isStrict ? segmentF(midRight) > segmentG(midRight)
                                             : segmentF(midRight) >= segmentG(midRight);

            segments.emplace_back(0.0, fWinsLeft ? 1.0 : 0.0, segmentF.getLeft(), splitAt);
            segments.emplace_back(0.0, fWinsRight ? 1.0 : 0.0, splitAt, segmentF.getRight());
        }
    }

    return PiecewiseLinearFunction(std::move(segments)).merged();
}
}  // namespace numerics::linear
