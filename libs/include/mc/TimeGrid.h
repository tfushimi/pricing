#pragma once

#include <chrono>
#include <cmath>
#include <vector>

#include "common/types.h"

namespace mc {

class TimeGrid {
   public:
    TimeGrid(const std::vector<Date>& fixingDates, const Date startDate,
             const double maxDt = 1.0 / 52.0)
        : _startDate(startDate) {
        // t=0: the start date itself
        _simulationDates.push_back(startDate);
        _times.push_back(0.0);
        _isFixingTime.push_back(false);

        Date prev = startDate;

        for (const Date& fixingDate : fixingDates) {
            const double dt = yearFraction(prev, fixingDate);

            if (dt > maxDt) {
                const std::size_t n = static_cast<std::size_t>(std::ceil(dt / maxDt));

                const auto prevSysDays = std::chrono::sys_days(prev);
                const auto fixSysDays = std::chrono::sys_days(fixingDate);
                const int totalDays = (fixSysDays - prevSysDays).count();

                for (std::size_t i = 1; i < n; ++i) {
                    const double alpha = static_cast<double>(i) / static_cast<double>(n);
                    const Date midDate = std::chrono::sys_days(
                        prevSysDays +
                        std::chrono::days(static_cast<int>(std::round(alpha * totalDays))));

                    _simulationDates.push_back(midDate);
                    _times.push_back(yearFraction(_startDate, midDate));
                    _isFixingTime.push_back(false);
                }
            }

            _simulationDates.push_back(fixingDate);
            _times.push_back(yearFraction(_startDate, fixingDate));
            _isFixingTime.push_back(true);

            prev = fixingDate;
        }
    }

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