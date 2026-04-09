#pragma once

#include <chrono>
#include <format>

namespace calendar {

using Date = std::chrono::year_month_day;

inline std::string toString(const Date& date) {
    return std::format("{}", date);  // e.g. "2026-01-15"
}

inline Date makeDate(const int year, const int month, const int day) {
    return Date{std::chrono::year{year}, std::chrono::month{static_cast<unsigned>(month)},
                std::chrono::day{static_cast<unsigned>(day)}};
}

// Year fraction between two dates — feeds directly into BS pricing
inline double yearFraction(const Date from, const Date to) {
    const auto days = std::chrono::sys_days{to} - std::chrono::sys_days{from};
    return days.count() / 365.25;
}
}  // namespace calendar

// Hash for Date so it can be used as unordered_map key
template <>
struct std::hash<calendar::Date> {
    std::size_t operator()(const calendar::Date& d) const noexcept {
        // sys_days is just an integer count — trivially hashable
        return std::hash<int>{}(std::chrono::sys_days{d}.time_since_epoch().count());
    }
};
