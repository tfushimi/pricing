#pragma once

#include <vector>
#include <algorithm>
#include "Segment.h"

namespace numerics::pwl {
    class PiecewiseLinearFunction {
    public:
        explicit PiecewiseLinearFunction(std::vector<Segment> segments): _segments(std::move(segments)) {
            validate();
        }

        // Rule of five
        PiecewiseLinearFunction(const PiecewiseLinearFunction&) = default;
        PiecewiseLinearFunction(PiecewiseLinearFunction&&) = default;
        PiecewiseLinearFunction& operator=(const PiecewiseLinearFunction&) = default;
        PiecewiseLinearFunction& operator=(PiecewiseLinearFunction&&) = default;
        ~PiecewiseLinearFunction() = default;

        // static constructors
        static PiecewiseLinearFunction createLinear(const double slope, const double intercept,
                                                    const double lo, const double hi) {

            return PiecewiseLinearFunction({
               Segment(0.0,       0.0,       NEG_INF, lo ),
               Segment(slope, intercept,     lo,      hi ),
               Segment(0.0,       0.0,       hi,  POS_INF),
           });
        }

        static PiecewiseLinearFunction createLinear(const double slope, const double intercept) {

            return PiecewiseLinearFunction({
                Segment(slope, intercept, NEG_INF, POS_INF)
            });
        }

        static PiecewiseLinearFunction createConstant(const double x) {

            return PiecewiseLinearFunction({
                Segment(0.0, x, NEG_INF, POS_INF)
            });
        }

        const std::vector<Segment>& getSegments() const {
            return _segments;
        }

        std::vector<double> getBreakPoints() const {
            // the most left and right breakpoints are -inf and +inf

            std::vector<double> breakPoints;

            breakPoints.reserve(getSegments().size()-1);

            for (int i = 0; i < _segments.size()-1; ++i) {

                breakPoints.push_back(_segments[i].getRight());
            }

            return breakPoints;
        }

        double operator()(const double x) const {
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
        PiecewiseLinearFunction operator+(const PiecewiseLinearFunction& other) const {

            auto [f, g] = align(*this, other);

            std::vector<Segment> segments;

            for (int i = 0; i < f._segments.size(); ++i) {

                Segment& seg1 = f._segments[i];
                Segment& seg2 = g._segments[i];

                segments.push_back(Segment(seg1.getSlope() + seg2.getSlope(), seg1.getIntercept() + seg2.getIntercept(), seg1.getLeft(), seg1.getRight()));
            }

            auto result = PiecewiseLinearFunction(segments);

            return result.merged();
        }

        PiecewiseLinearFunction operator-() const {

            std::vector<Segment> segments;

            for (const auto seg : _segments) {

                segments.push_back(Segment(-seg.getSlope(), -seg.getIntercept(), seg.getLeft(), seg.getRight()));
            }

            return PiecewiseLinearFunction(std::move(segments));
        }

        PiecewiseLinearFunction operator-(const PiecewiseLinearFunction& other) const {

            return *this + (-other);
        }

        PiecewiseLinearFunction operator*(const PiecewiseLinearFunction& other) const {

            auto [f, g] = align(*this, other);

            std::vector<Segment> segs;
            segs.reserve(f._segments.size());

            for (std::size_t i = 0; i < f._segments.size(); ++i) {

                const auto& a = f._segments[i];
                const auto& b = g._segments[i];

                if (a.getSlope() == 0.0) {

                    const double c = a.getIntercept();
                    segs.emplace_back(c * b.getSlope(), c * b.getIntercept(),
                                      a.getLeft(), a.getRight());
                } else if (b.getSlope() == 0.0) {

                    const double c = b.getIntercept();
                    segs.emplace_back(c * a.getSlope(), c * a.getIntercept(),
                                      a.getLeft(), a.getRight());
                } else {
                    throw std::invalid_argument(
                        "operator*: product of two non-constant segments is not linear");
                }
            }

            return PiecewiseLinearFunction(std::move(segs)).merged();
        }

        PiecewiseLinearFunction operator/(const PiecewiseLinearFunction& other) const {

            auto [f, g] = align(*this, other);

            std::vector<Segment> segs;

            segs.reserve(f._segments.size());

            for (std::size_t i = 0; i < f._segments.size(); ++i) {

                const auto& fSeg = f._segments[i];
                const auto& gSeg = g._segments[i];

                if (gSeg.getSlope() != 0.0) {
                    throw std::invalid_argument(
                        "operator/: cannot divide by a non-constant PWL segment");
                }

                if (gSeg.getIntercept() == 0.0) {
                    throw std::invalid_argument(
                        "operator/: division by zero");
                }

                const double c = gSeg.getIntercept();

                segs.emplace_back(fSeg.getSlope() / c, fSeg.getIntercept() / c,
                                  fSeg.getLeft(), fSeg.getRight());
            }

            return PiecewiseLinearFunction(std::move(segs)).merged();
        }

        // Max/Min
        static PiecewiseLinearFunction max(const PiecewiseLinearFunction& f,
                                           const PiecewiseLinearFunction& g) {
            return applyMaxMin(f, g, true);
        }

        static PiecewiseLinearFunction min(const PiecewiseLinearFunction& f,
                                           const PiecewiseLinearFunction& g) {
            return applyMaxMin(f, g, false);
        }

        // Alignment helpers
        static std::pair<PiecewiseLinearFunction, PiecewiseLinearFunction>
        align(const PiecewiseLinearFunction& f, const PiecewiseLinearFunction& g) {
            const auto breaks = mergedBreakPoints(f, g);
            return { f.withBreakPoints(breaks), g.withBreakPoints(breaks) };
        }

        std::string toString() const {

            std::string result;

            for (const auto& seg : _segments) {

                result += seg.toString();
            }

            return result;;
        }
    private:
        void validate() const {
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
                if (_segments[i].getRight() != _segments[i+1].getLeft()) {

                    throw std::invalid_argument("PiecewiseLinearFunction: segments are not contiguous");
                }
            }
        }

        PiecewiseLinearFunction merged() const {

            if (_segments.size() == 0) {

                return *this;
            }

            std::vector<Segment> segments;

            segments.push_back(_segments[0]);

            for (int i = 1; i < _segments.size(); i++) {

                const Segment& curr = _segments[i];
                Segment& prev = segments.back();

                if (prev.getSlope() == curr.getSlope() && prev.getIntercept() == curr.getIntercept()) {

                    prev = Segment(prev.getSlope(), prev.getIntercept(), prev.getLeft(), curr.getRight());
                }
                else {

                    segments.push_back(curr);
                }
            }

            return PiecewiseLinearFunction(std::move(segments));
        }

        static std::vector<double> mergedBreakPoints(const PiecewiseLinearFunction& f, const PiecewiseLinearFunction& g) {

            const auto fb = f.getBreakPoints();
            const auto gs = g.getBreakPoints();

            std::vector<double> merged;

            merged.reserve(fb.size() + gs.size());

            std::set_union(fb.begin(), fb.end(), gs.begin(), gs.end(), std::back_inserter(merged));

            return merged;
        }

        PiecewiseLinearFunction withBreakPoints(const std::vector<double>& breakPoints) const {
            std::vector<Segment> segs;

            for (const auto& seg : _segments) {

                // Collect interior breakpoints for this segment
                std::vector pts = { seg.getLeft() };

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

                    segs.emplace_back(seg.getSlope(), seg.getIntercept(), pts[i], pts[i+1]);
                }
            }

            return PiecewiseLinearFunction(std::move(segs));
        }

        static PiecewiseLinearFunction applyMaxMin(const PiecewiseLinearFunction& f,
                                                   const PiecewiseLinearFunction& g,
                                                   const bool isMax) {
            auto [fa, ga] = align(f, g);
            std::vector<Segment> segs;
            segs.reserve(fa._segments.size());

            for (std::size_t i = 0; i < fa._segments.size(); ++i) {
                const auto& sf = fa._segments[i];
                const auto& sg = ga._segments[i];
                auto cross = sf.crossing(sg);

                if (!cross) {
                    // No crossing: one segment dominates throughout
                    double mid = sf.midpoint();
                    bool fWins = isMax ? (sf(mid) >= sg(mid)) : (sf(mid) <= sg(mid));
                    const auto& w = fWins ? sf : sg;
                    segs.emplace_back(w.getSlope(), w.getIntercept(),
                                      sf.getLeft(), sf.getRight());
                } else {
                    // Crossing at x: split into [lo, x) and [x, hi)
                    double x    = *cross;
                    double midL = sf.midpointLeft(x);
                    double midR = sf.midpointRight(x);

                    bool fWinsLeft  = isMax ? (sf(midL) >= sg(midL)) : (sf(midL) <= sg(midL));
                    bool fWinsRight = isMax ? (sf(midR) >= sg(midR)) : (sf(midR) <= sg(midR));

                    const auto& wL = fWinsLeft  ? sf : sg;
                    const auto& wR = fWinsRight ? sf : sg;

                    segs.emplace_back(wL.getSlope(), wL.getIntercept(), sf.getLeft(), x);
                    segs.emplace_back(wR.getSlope(), wR.getIntercept(), x, sf.getRight());
                }
            }
            return PiecewiseLinearFunction(std::move(segs)).merged();
        }

        std::vector<Segment> _segments;
    };
}
