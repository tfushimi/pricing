#pragma once

class BSVolSlice {
   public:
    BSVolSlice() = default;
    virtual ~BSVolSlice() = default;
    virtual double forward() const = 0;  // fair strike of forward contract
    virtual double time() const = 0;
    virtual double vol() const = 0;
    virtual double dVolDStrike(double strike) const = 0;
};