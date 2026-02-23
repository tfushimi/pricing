#include "PiecewiseLinearFunction.h"

using namespace numerics::linear;

PiecewiseLinearFunction PiecewiseLinearFunction::linear(const double slope,
                                                              const double intercept,
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
    for (const auto& seg : _segments) {
        if (seg.contains(x)) {
            return seg(x);
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
        Segment& seg1 = f._segments[i];
        Segment& seg2 = g._segments[i];

        segments.push_back(Segment(seg1.getSlope() + seg2.getSlope(),
                                   seg1.getIntercept() + seg2.getIntercept(), seg1.getLeft(),
                                   seg1.getRight()));
    }

    return PiecewiseLinearFunction(std::move(segments)).merged();
}

PiecewiseLinearFunction PiecewiseLinearFunction::operator-() const {
    std::vector<Segment> segments;

    for (const auto seg : _segments) {
        segments.push_back(
            Segment(-seg.getSlope(), -seg.getIntercept(), seg.getLeft(), seg.getRight()));
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

    std::vector<Segment> segs;
    segs.reserve(f._segments.size());

    for (std::size_t i = 0; i < f._segments.size(); ++i) {
        const auto& a = f._segments[i];
        const auto& b = g._segments[i];

        if (a.getSlope() == 0.0) {
            const double c = a.getIntercept();
            segs.emplace_back(c * b.getSlope(), c * b.getIntercept(), a.getLeft(), a.getRight());
        } else if (b.getSlope() == 0.0) {
            const double c = b.getIntercept();
            segs.emplace_back(c * a.getSlope(), c * a.getIntercept(), a.getLeft(), a.getRight());
        } else {
            throw std::invalid_argument(
                "operator*: product of two non-constant segments is not linear");
        }
    }

    return PiecewiseLinearFunction(std::move(segs)).merged();
}

PiecewiseLinearFunction PiecewiseLinearFunction::operator/(
    const PiecewiseLinearFunction& other) const {
    auto [f, g] = align(*this, other);

    std::vector<Segment> segments;

    segments.reserve(f._segments.size());

    for (std::size_t i = 0; i < f._segments.size(); ++i) {
        const auto& fSeg = f._segments[i];
        const auto& gSeg = g._segments[i];

        if (gSeg.getSlope() != 0.0) {
            throw std::invalid_argument("operator/: cannot divide by a non-constant PWL segment");
        }

        if (gSeg.getIntercept() == 0.0) {
            throw std::invalid_argument("operator/: division by zero");
        }

        const double c = gSeg.getIntercept();

        segments.emplace_back(fSeg.getSlope() / c, fSeg.getIntercept() / c, fSeg.getLeft(),
                              fSeg.getRight());
    }

    return PiecewiseLinearFunction(std::move(segments)).merged();
}

PiecewiseLinearFunction PiecewiseLinearFunction::operator>(const PiecewiseLinearFunction& other) const {
    return greaterThanInner(*this, other, true);
}

PiecewiseLinearFunction PiecewiseLinearFunction::operator>=(const PiecewiseLinearFunction& other) const {
    return greaterThanInner(*this, other, false);
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

std::string PiecewiseLinearFunction::toString() const {
    std::string result;

    for (const auto& seg : _segments) {
        result += seg.toString();
    }

    return result;
    ;
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
    const auto breaks = mergedBreakPoints(f, g);
    return {f.withBreakPoints(breaks), g.withBreakPoints(breaks)};
}

// creates a new PL by merging adjacent segments if possible
PiecewiseLinearFunction PiecewiseLinearFunction::merged() const {
    if (_segments.size() == 0) {
        return *this;
    }

    std::vector<Segment> segments;

    segments.push_back(_segments[0]);

    for (std::size_t i = 1; i < _segments.size(); i++) {
        const Segment& curr = _segments[i];
        Segment& prev = segments.back();

        if (prev.getSlope() == curr.getSlope() && prev.getIntercept() == curr.getIntercept()) {
            prev = Segment(prev.getSlope(), prev.getIntercept(), prev.getLeft(), curr.getRight());
        } else {
            segments.push_back(curr);
        }
    }

    return PiecewiseLinearFunction(std::move(segments));
}

// returns a sorted union of breakpoints of two functions
std::vector<double> PiecewiseLinearFunction::mergedBreakPoints(const PiecewiseLinearFunction& f,
                                                               const PiecewiseLinearFunction& g) {
    const auto fb = f.getBreakPoints();
    const auto gs = g.getBreakPoints();

    std::vector<double> merged;

    merged.reserve(fb.size() + gs.size());

    std::set_union(fb.begin(), fb.end(), gs.begin(), gs.end(), std::back_inserter(merged));

    return merged;
}

// create a new PL with additional breakpoints
PiecewiseLinearFunction PiecewiseLinearFunction::withBreakPoints(
    const std::vector<double>& breakPoints) const {
    std::vector<Segment> segments;

    for (const auto& seg : _segments) {
        // Collect interior breakpoints for this segment
        std::vector pts = {seg.getLeft()};

        for (double bp : breakPoints) {
            if (seg.containsInterior(bp)) {
                pts.push_back(bp);
            }
        }

        pts.push_back(seg.getRight());

        // pts is already sorted because breakPoints is sorted and we only
        // take interior points, but sort defensively
        std::sort(pts.begin(), pts.end());

        for (std::size_t i = 0; i + 1 < pts.size(); ++i) {
            segments.emplace_back(seg.getSlope(), seg.getIntercept(), pts[i], pts[i + 1]);
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
        const auto& sf = fa._segments[i];
        const auto& sg = ga._segments[i];
        const auto cross = sf.crossing(sg);

        if (!cross) {
            // No crossing: one segment dominates throughout
            const double midpoint = sf.midpoint();
            const bool fWins = isMax ? sf(midpoint) >= sg(midpoint) : sf(midpoint) <= sg(midpoint);
            const auto& winner = fWins ? sf : sg;
            segments.emplace_back(winner.getSlope(), winner.getIntercept(), sf.getLeft(),
                              sf.getRight());
        } else {
            // Crossing at x: split into [lo, x) and [x, hi)
            const double splitAt = *cross;
            const double midLeft = sf.midpointLeft(splitAt);
            const double midRight = sf.midpointRight(splitAt);

            const bool fWinsLeft = isMax ? sf(midLeft) >= sg(midLeft) : sf(midLeft) <= sg(midLeft);
            const bool fWinsRight =
                isMax ? sf(midRight) >= sg(midRight) : sf(midRight) <= sg(midRight);

            const auto& winnerLeft = fWinsLeft ? sf : sg;
            const auto& winnerRight = fWinsRight ? sf : sg;

            segments.emplace_back(winnerLeft.getSlope(), winnerLeft.getIntercept(), sf.getLeft(),
                              splitAt);
            segments.emplace_back(winnerRight.getSlope(), winnerRight.getIntercept(), splitAt,
                              sf.getRight());
        }
    }

    return PiecewiseLinearFunction(std::move(segments)).merged();
}

PiecewiseLinearFunction PiecewiseLinearFunction::greaterThanInner(const PiecewiseLinearFunction& f,
                                                                  const PiecewiseLinearFunction& g,
                                                                  bool isStrict) {
    auto [fa, ga] = align(f, g);

    std::vector<Segment> segments;
    segments.reserve(fa._segments.size());

    for (std::size_t i = 0; i < fa._segments.size(); ++i) {
        const auto& sf = fa._segments[i];
        const auto& sg = ga._segments[i];
        const auto cross = sf.crossing(sg);

        if (!cross) {
            // No crossing: one segment dominates throughout
            const double midpoint = sf.midpoint();
            const bool fWins = isStrict ? sf(midpoint) > sg(midpoint) : sf(midpoint) >= sg(midpoint);
            segments.emplace_back(0.0, fWins ? 1.0 : 0.0, sf.getLeft(), sf.getRight());
        } else {
            // Crossing at x:
            // if isStrict = true, split into [lo, x + epsilon) and [x + epsilon, hi)
            // if isStrict = false, split into [lo, x) and [x, hi)
            const double splitAt = isStrict ? *cross + 1e-6 : *cross;

            // Validate split is still inside the segment
            if (!sf.containsInterior(splitAt)) {
                // crossing is so close to a boundary that epsilon pushes it out, fall back to midpoint evaluation
                const double mid = sf.midpoint();
                const bool fWins = isStrict ? sf(mid) > sg(mid) : sf(mid) >= sg(mid);
                segments.emplace_back(0.0, fWins ? 1.0 : 0.0, sf.getLeft(), sf.getRight());
                continue;
            }

            const double midLeft = sf.midpointLeft(splitAt);
            const double midRight = sf.midpointRight(splitAt);

            const bool fWinsLeft = isStrict ? sf(midLeft) > sg(midLeft) : sf(midLeft) >= sg(midLeft);
            const bool fWinsRight = isStrict ? sf(midRight) > sg(midRight) : sf(midRight) >= sg(midRight);

            segments.emplace_back(0.0, fWinsLeft ? 1.0 : 0.0, sf.getLeft(), splitAt);
            segments.emplace_back(0.0, fWinsRight ? 1.0 : 0.0, splitAt, sf.getRight());
        }
    }

    return PiecewiseLinearFunction(std::move(segments)).merged();
}
