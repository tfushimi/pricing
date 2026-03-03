#pragma once

#include <vector>

#include "common/types.h"

namespace mc {

// TODO implement
class TimeGrid {
   public:
    TimeGrid(const std::vector<Date>& fixingDates, Date referenceDate, double maxDt = 1.0 / 52.0);

    // number of time points (including t=0)
    std::size_t size() const;

    double time(std::size_t i) const;

    bool isFixingTime(std::size_t i) const;

    Date date(std::size_t i) const;
};
}  // namespace mc