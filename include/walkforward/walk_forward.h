#pragma once

#include "common/types.h"
#include "core/backtest_engine.h"
#include "metrics/backtest_snapshot.h"
#include "strategy/strategy_base.h"
#include "optimization/param_space.h"
#include <vector>
#include <functional>

namespace qp {

struct WFAConfig {
    int num_windows = 5;
    double in_sample_ratio = 0.7;
    std::string optimization_objective = "sharpe";
};

struct WFAWindow {
    int in_sample_start = 0;
    int in_sample_end = 0;
    int out_of_sample_start = 0;
    int out_of_sample_end = 0;
    StrategyParams best_params;
    BacktestSnapshot in_sample_metrics;
    BacktestSnapshot out_of_sample_metrics;
};

struct WFAResult {
    std::vector<WFAWindow> windows;
    double consistency_score = 0.0;
    double avg_oos_sharpe = 0.0;
    bool is_consistent = false;
};

class WalkForward {
public:
    using OptimizerFunc = std::function<StrategyParams(
        IStrategy& strategy,
        const std::vector<Bar>& bars,
        const ParamSpace& space,
        const BacktestConfig& bt_config,
        const std::string& objective)>;

    static WFAResult run(IStrategy& strategy,
                         const std::vector<Bar>& bars,
                         const ParamSpace& param_space,
                         const BacktestConfig& bt_config,
                         const WFAConfig& wfa_config,
                         OptimizerFunc optimizer_func);
};

} // namespace qp
