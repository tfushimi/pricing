#pragma once

#include "common/Types.h"

namespace mc {
inline double mean(const Sample& s) {
    return s.sum() / static_cast<double>(s.size());
}

inline double variance(const Sample& s) {
    const double avg = mean(s);
    return mean(s * s) - avg * avg;
}
}  // namespace mc