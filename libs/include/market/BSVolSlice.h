#pragma once

namespace market {
class BSVolSlice {
   public:
    BSVolSlice() = default;
    virtual ~BSVolSlice() = default;
    virtual double forward() const = 0;  // fair strike of forward contract
    virtual double time() const = 0;
    virtual double vol(double strike) const = 0;

    virtual double dVolDStrike(const double strike) const {
        constexpr double dK = 1e-4;
        return (vol(strike + dK) - vol(strike - dK)) / (2.0 * dK);
    }
};

class FlatVolSlice final : public BSVolSlice {
   public:
    FlatVolSlice(const double forward, const double time, const double vol)
        : _forward(forward), _time(time), _vol(vol) {}

    double forward() const override { return _forward; }
    double time() const override { return _time; }
    double vol(const double strike) const override { return _vol; }
    double dVolDStrike(const double strike) const override { return 0.0; }

   private:
    double _forward;
    double _time;
    double _vol;
};
}  // namespace market