#pragma once

#include "common/types.h"
#include "core/backtest_engine.h"
#include "metrics/backtest_snapshot.h"
#include "metrics/metrics_engine.h"
#include "strategy/strategy_base.h"
#include "optimization/param_space.h"
#include <vector>
#include <string>

namespace qp {

struct OptTrialResult {
    StrategyParams params;
    BacktestSnapshot metrics;
    double objective_value = 0.0;
};

struct OptResult {
    StrategyParams best_params;
    BacktestSnapshot best_metrics;
    double best_objective = 0.0;
    std::vector<OptTrialResult> all_trials;
    int total_evaluations = 0;
};

class Optimizer {
public:
    static OptResult run(IStrategy& strategy,
                         const std::vector<Bar>& bars,
                         const ParamSpace& param_space,
                         const BacktestConfig& bt_config,
                         const std::string& objective = "sharpe",
                         int max_iterations = 0);

    static StrategyParams find_best(IStrategy& strategy,
                                     const std::vector<Bar>& bars,
                                     const ParamSpace& param_space,
                                     const BacktestConfig& bt_config,
                                     const std::string& objective);

    static double evaluate_objective(const BacktestSnapshot& snapshot,
                                      const std::string& objective);
};

} // namespace qp
