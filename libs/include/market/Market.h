#pragma once

#include <memory>
#include <string>

#include "BSVolSlice.h"

class Market {
   public:
    Market() = default;
    virtual ~Market() = default;
    virtual std::unique_ptr<BSVolSlice> getBSVolSlice(const std::string& date) = 0;
};