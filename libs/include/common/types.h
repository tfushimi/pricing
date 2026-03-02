#pragma once

#include <chrono>
#include <format>
#include <functional>
#include <map>
#include <valarray>

using Date = std::chrono::year_month_day;

inline std::string toString(const Date& date) {
    return std::format("{}", date);  // e.g. "2026-01-15"
}

// Hash for Date so it can be used as unordered_map key
template <>
struct std::hash<Date> {
    std::size_t operator()(const Date& d) const noexcept {
        // sys_days is just an integer count — trivially hashable
        return std::hash<int>{}(std::chrono::sys_days{d}.time_since_epoch().count());
    }
};

inline Date makeDate(const int year, const int month, const int day) {
    return Date{std::chrono::year{year}, std::chrono::month{static_cast<unsigned>(month)},
                std::chrono::day{static_cast<unsigned>(day)}};
}

// Year fraction between two dates — feeds directly into BS pricing
inline double yearFraction(const Date from, const Date to) {
    const auto days = std::chrono::sys_days{to} - std::chrono::sys_days{from};
    return days.count() / 365.25;
}

// -inf/inf of double
constexpr double NEG_INF = -std::numeric_limits<double>::infinity();
constexpr double POS_INF = std::numeric_limits<double>::infinity();

// N path values at one fixing date
using Sample = std::valarray<double>;

// full evolution across fixing dates
using Scenario = std::map<Date, Sample>;