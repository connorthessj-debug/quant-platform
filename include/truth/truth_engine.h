#pragma once

#include "common/types.h"
#include "common/config.h"
#include "core/backtest_engine.h"
#include "metrics/backtest_snapshot.h"
#include "montecarlo/monte_carlo.h"
#include "walkforward/walk_forward.h"
#include "optimization/optimizer.h"
#include <vector>
#include <string>

namespace qp {

struct TruthReport {
    double score = 0.0;
    bool passed = false;
    bool lookahead_clean = true;
    bool slippage_robust = false;
    bool param_stable = false;
    bool wfa_consistent = false;
    bool mc_survived = false;
    bool regime_independent = false;
    std::vector<std::string> flags;
    BacktestSnapshot base_metrics;
    BacktestSnapshot perturbed_metrics;
};

class TruthEngine {
public:
    TruthReport validate(IStrategy& strategy,
                         const std::vector<Bar>& bars,
                         const BacktestConfig& bt_config,
                         const BacktestResult& base_result,
                         const BacktestSnapshot& base_metrics,
                         const MCResult& mc_result,
                         const WFAResult& wfa_result,
                         const OptResult& opt_result,
                         const TruthEngineConfig& config);

private:
    bool check_lookahead(const std::vector<Trade>& trades,
                         const std::vector<Bar>& bars);
    bool check_slippage_robustness(IStrategy& strategy,
                                    const std::vector<Bar>& bars,
                                    const BacktestConfig& base_config,
                                    const BacktestSnapshot& base_metrics,
                                    double perturbation_bps);
    bool check_param_stability(const OptResult& opt_result);
    bool check_wfa_consistency(const WFAResult& wfa_result, double threshold);
    bool check_mc_survival(const MCResult& mc_result, double threshold);
    bool check_regime_independence(const std::vector<Trade>& trades,
                                   const std::vector<Bar>& bars);
};

} // namespace qp
