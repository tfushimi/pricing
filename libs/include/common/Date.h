#pragma once

#include <chrono>
#include <format>
#include <iomanip>
#include <sstream>
#include <string>

namespace calendar {

using Date = std::chrono::sys_days;

inline std::string toString(const Date& date) {
    return std::format("{:%Y-%m-%d}", date);  // e.g. "2026-01-15"
}

inline Date makeDate(const int year, const int month, const int day) {
    return Date{std::chrono::year{year} / std::chrono::month{static_cast<unsigned>(month)} /
                std::chrono::day{static_cast<unsigned>(day)}};
}

inline Date fromString(const std::string& s) {
    std::tm tm{};
    std::istringstream ss(s);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    return makeDate(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
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
