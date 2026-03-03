#pragma once
#include "RNG.h"
#include "TimeGrid.h"
#include "common/types.h"

template <typename ProcessType>
class ProcessStateStepper {
    using State = ProcessType::State;

   public:
    explicit ProcessStateStepper(const ProcessType& process) : _process(process) {}

    Scenario run(const mc::TimeGrid& timeGrid, std::size_t nPaths, mc::RNG& rng) const {
        Scenario scenario;
        State state = _process.initialState(nPaths);

        for (std::size_t i = 0; i + 1 < timeGrid.size(); i++) {
            const double t = timeGrid.time(i);
            const double dt = timeGrid.time(i + 1) - t;

            std::vector<Sample> dW(_process.nNormals(), Sample(0.0, nPaths));
            for (auto& w : dW) {
                rng.fill(w);
            }

            state = _process.step(state, t, dt, dW);

            if (timeGrid.isFixingTime(i + 1)) {
                scenario[timeGrid.date(i + 1)] = _process.spot(state);
            }
        }

        return scenario;
    }

   private:
    const ProcessType& _process;
};