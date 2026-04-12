#include "mc/TimeGrid.h"

#include <algorithm>
#include <stdexcept>
#include <vector>

#include "common/Date.h"

namespace mc {

TimeGrid::TimeGrid(const std::vector<Date>& fixingDates, const Date startDate, const double maxDt)
    : _startDate(startDate) {

    if (!std::is_sorted(fixingDates.begin(), fixingDates.end())) {
        throw std::invalid_argument("TimeGrid: fixingDates must be in ascending order");
    }

    if (!fixingDates.empty() && fixingDates.front() <= startDate) {
        throw std::invalid_argument("TimeGrid: fixingDates must be after startDate");
    }

    // t=0: the start date itself
    _simulationDates.push_back(startDate);
    _times.push_back(0.0);
    _isFixingTime.push_back(false);

    Date prev = startDate;

    for (const Date& fixingDate : fixingDates) {
        const double dt = yearFraction(prev, fixingDate);

        if (dt > maxDt) {
            // Insert n-1 intermediate simulation dates evenly spaced between prev and fixingDate.
            // Dates are rounded to the nearest calendar day, so individual steps may exceed maxDt
            // by up to half a day (~0.14% of a year). This is negligible for MC simulation.
            const std::size_t n = static_cast<std::size_t>(std::ceil(dt / maxDt));
            const int totalDays = (fixingDate - prev).count();

            for (std::size_t i = 1; i < n; ++i) {
                const double fraction = static_cast<double>(i) / static_cast<double>(n);
                const auto nextSimulationDate =
                    prev + std::chrono::days(static_cast<int>(std::round(fraction * totalDays)));

                if (nextSimulationDate != _simulationDates.back()) {
                    _simulationDates.push_back(nextSimulationDate);
                    _times.push_back(yearFraction(_startDate, nextSimulationDate));
                    _isFixingTime.push_back(false);
                }
            }
        }

        if (fixingDate != _simulationDates.back()) {
            _simulationDates.push_back(fixingDate);
            _times.push_back(yearFraction(_startDate, fixingDate));
            _isFixingTime.push_back(true);
        }

        prev = fixingDate;
    }
}
}  // namespace mc