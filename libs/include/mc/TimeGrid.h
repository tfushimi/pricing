#pragma once

#include <chrono>
#include <vector>

#include "common/Date.h"

namespace mc {
using calendar::Date;
using calendar::yearFraction;

class TimeGrid {
   public:
    TimeGrid(const std::vector<Date>& fixingDates, Date startDate, double maxDt = 1.0 / 52.0);

    std::size_t size() const { return _simulationDates.size(); }

    double time(std::size_t i) const { return _times[i]; }

    bool isFixingTime(std::size_t i) const { return _isFixingTime[i]; }

    Date date(std::size_t i) const { return _simulationDates[i]; }

   private:
    std::vector<Date> _simulationDates;
    std::vector<double> _times;
    std::vector<bool> _isFixingTime;
    Date _startDate;
};

}  // namespace mc