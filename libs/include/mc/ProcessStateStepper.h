#pragma once

#include <map>

#include "TimeGrid.h"
#include "common/Date.h"
#include "numerics/RNG.h"

namespace mc {
using calendar::Date;

// TODO try concept
template <typename ProcessType>
class ProcessStateStepper {
    using State = ProcessType::State;

   public:
    explicit ProcessStateStepper(const ProcessType& process) : _process(process) {}

    Scenario run(const TimeGrid& timeGrid, std::size_t nPaths,
                 std::unique_ptr<numerics::rng::RNG> rng) const {
        Scenario scenario;
        State state = _process.initialState(nPaths);

        for (std::size_t i = 0; i + 1 < timeGrid.size(); i++) {
            const auto currentTime = timeGrid.time(i);
            const auto dt = timeGrid.time(i + 1) - currentTime;

            std::vector<Sample> dW(_process.nNormals(), Sample(0.0, nPaths));

            for (auto& w : dW) {
                rng->fill(w);
            }

            state = _process.step(state, currentTime, dt, dW);

            if (timeGrid.isFixingTime(i + 1)) {
                scenario[timeGrid.date(i + 1)] = _process.value(state, currentTime + dt);
            }
        }

        return scenario;
    }

   private:
    const ProcessType _process;
};
}  // namespace mc