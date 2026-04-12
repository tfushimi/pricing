#include "mc/TimeGrid.h"

#include <gtest/gtest.h>

#include "common/Date.h"

using namespace calendar;
using namespace mc;

class TimeGridTest : public ::testing::Test {
   protected:
    const Date startDate = makeDate(2025, 1, 1);
};

TEST_F(TimeGridTest, ExceptionsIfFixingDatesNotSorted) {
    const std::vector fixingDates = {makeDate(2025, 6, 1), makeDate(2025, 3, 1)};
    EXPECT_THROW(TimeGrid(fixingDates, startDate), std::invalid_argument);
}

TEST_F(TimeGridTest, ExceptionsIfFixingDateOnStartDate) {
    const std::vector fixingDates = {startDate};
    EXPECT_THROW(TimeGrid(fixingDates, startDate), std::invalid_argument);
}

TEST_F(TimeGridTest, ExceptionsIfFixingDateBeforeStartDate) {
    const std::vector fixingDates = {makeDate(2024, 12, 1)};
    EXPECT_THROW(TimeGrid(fixingDates, startDate), std::invalid_argument);
}

TEST_F(TimeGridTest, EmptyFixingDates) {
    const TimeGrid grid({}, startDate);
    EXPECT_EQ(grid.size(), 1);  // only startDate
    EXPECT_EQ(grid.date(0), startDate);
    EXPECT_NEAR(grid.time(0), 0.0, 1e-15);
    EXPECT_FALSE(grid.isFixingTime(0));
}

TEST_F(TimeGridTest, SingleFixingDate) {
    const Date fixingDate = makeDate(2025, 1, 2);
    const TimeGrid grid({fixingDate}, startDate, 1.0 / 12.0);

    // startDate and fixingDate
    EXPECT_EQ(grid.size(), 2);

    EXPECT_EQ(grid.date(0), startDate);
    EXPECT_NEAR(grid.time(0), 0.0, 1e-15);
    EXPECT_FALSE(grid.isFixingTime(0));

    EXPECT_EQ(grid.date(1), fixingDate);
    EXPECT_NEAR(grid.time(1), yearFraction(startDate, fixingDate), 1e-15);
    EXPECT_TRUE(grid.isFixingTime(1));
}

TEST_F(TimeGridTest, SubstepsInsertedWhenGapExceedsMaxDt) {
    // gap is 1 year, maxDt = 1/12 -> expect ~12 substeps
    const Date fixingDate = makeDate(2026, 1, 1);
    const TimeGrid grid({fixingDate}, startDate, 1.0 / 12.0);

    EXPECT_EQ(grid.size(), 13);

    // first is startDate, last is fixingDate
    EXPECT_EQ(grid.date(0), startDate);
    EXPECT_EQ(grid.date(grid.size() - 1), fixingDate);

    // fixing dates must be increasing
    for (std::size_t i = 1; i < grid.size(); ++i) {
        EXPECT_GT(grid.time(i), grid.time(i - 1));
    }

    // only the last date is a fixing time
    for (std::size_t i = 0; i + 1 < grid.size(); ++i) {
        EXPECT_FALSE(grid.isFixingTime(i));
    }

    EXPECT_TRUE(grid.isFixingTime(grid.size() - 1));
}

TEST_F(TimeGridTest, MultipleFixingDates) {
    const std::vector fixingDates = {makeDate(2025, 6, 1), makeDate(2026, 1, 1),
                                     makeDate(2026, 7, 1)};
    const TimeGrid grid(fixingDates, startDate, 1.0 / 12.0);

    // fixing dates must be increasing
    for (std::size_t i = 1; i < grid.size(); ++i) {
        EXPECT_GT(grid.time(i), grid.time(i - 1));
    }

    // every fixing date must appear and be marked as fixing time
    for (const auto& fd : fixingDates) {
        bool found = false;
        for (std::size_t i = 0; i < grid.size(); ++i) {
            if (grid.date(i) == fd) {
                EXPECT_TRUE(grid.isFixingTime(i));
                found = true;
                break;
            }
        }
        EXPECT_TRUE(found);
    }
}
