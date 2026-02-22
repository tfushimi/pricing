#pragma once
#include <stdexcept>
#include <string>
#include <utility>
#include <optional>

namespace numerics::pwl {
class Segment {
public:
    Segment(const double slope, const double intercept, const double left, const double right) : _slope(slope), _intercept(intercept), _left(left), _right(right) {
        if (_right < _left) {

            throw std::invalid_argument("Invalid endpoints: left=" + std::to_string(_left) + ",right=" + std::to_string(_right));
        }
    }
    ~Segment() = default;
    Segment(const Segment& other) = default;
    Segment& operator=(const Segment& other) = default;
    double getSlope() const { return _slope; }
    double getIntercept() const { return _intercept; }
    double getLeft() const { return _left; }
    double getRight() const { return _right; }
    bool contains(const double x) const { return _left <= x && _right >= x; }
    double operator()(const double x) const {
        if (contains(x)) {
            return _intercept + _slope * x;
        }
        return 0;
    }
    std::optional<double> getCrossingPoint(const Segment& other) const {

        if (_left != other._left || _right != other._right) {

            throw std::invalid_argument("endpoints do not match: this=" + toString() + ",other=" + other.toString());
        }

        if (_slope == other._slope) {

            return std::nullopt;
        }

        const double dslope = _slope - other._slope;

        if (std::abs(dslope) < std::numeric_limits<double>::epsilon()) {

            return std::nullopt;
        }

        const double crossing = (other._intercept - _intercept) / dslope;

        if (contains(crossing)) {

            return crossing;
        }

        return std::nullopt;
    }
    std::pair<Segment, Segment> splitAt(const double x) const {

        if (!contains(x)) {

            throw std::invalid_argument("Cannot split at x: left=" + std::to_string(_left) + ",right=" + std::to_string(_right));
        }

        return std::make_pair(Segment(_slope, _intercept, _left, x), Segment(_slope, _intercept, x, _right));
    }
    std::string toString() const {

        return "Segment[intercept=" + std::to_string(_intercept) + + ",slope=" + std::to_string(_slope) + ",left=" + std::to_string(_left) + ",right=" + std::to_string(_right) + "]";
    }
private:
    double _slope;
    double _intercept;
    double _left;
    double _right;
};
}
