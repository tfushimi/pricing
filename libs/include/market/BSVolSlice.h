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

}  // namespace market