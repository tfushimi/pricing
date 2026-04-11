#pragma once

#include <chrono>
#include <format>
#include <string>

namespace calendar {

using Date = std::chrono::sys_days;

inline std::string toString(const Date& date) {
    return std::format("{:%Y-%m-%d}", date);  // e.g. "2026-01-15"
}

inline Date makeDate(const int year, const int month, const int day) {
    return Date{std::chrono::year{year} / std::chrono::month{static_cast<unsigned>(month)} / std::chrono::day{static_cast<unsigned>(day)}};
}

// Year fraction between two dates — feeds directly into BS pricing
inline double yearFraction(const Date from, const Date to) {
    return (to - from).count() / 365.25;
}
}  // namespace calendar

// Hash for Date so it can be used as unordered_map key
template <>
struct std::hash<calendar::Date> {
    std::size_t operator()(const calendar::Date& date) const noexcept {
        // sys_days is just an integer count — trivially hashable
        return std::hash<int>{}(date.time_since_epoch().count());
    }
};
