#pragma once

#include <chrono>

using Date = std::chrono::year_month_day;

inline Date makeDate(const int year, const int month, const int day) {
    return Date{std::chrono::year{year}, std::chrono::month{static_cast<unsigned>(month)},
                std::chrono::day{static_cast<unsigned>(day)}};
}

// Year fraction between two dates — feeds directly into BS pricing
inline double yearFraction(const Date from, const Date to) {
    const auto days = std::chrono::sys_days{to} - std::chrono::sys_days{from};
    return days.count() / 365.25;
}
