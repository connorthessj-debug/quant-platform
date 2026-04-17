#pragma once

#include "common/types.h"
#include "core/backtest_engine.h"
#include "strategy/strategy_base.h"
#include "optimization/param_space.h"
#include "optimization/optimizer.h"
#include <string>

namespace qp {

struct BayesianOptConfig {
    int n_initial = 10;          // Random samples before Bayesian iterations
    int n_iterations = 40;       // Bayesian iterations after initial
    int n_candidates = 1024;     // Candidate points for acquisition maximization
    double xi = 0.01;            // Exploration parameter for EI
    double length_scale = 0.2;   // RBF kernel length scale (normalized units)
    double signal_variance = 1.0;
    double noise_variance = 1e-4;
    unsigned seed = 42;
};

// Bayesian optimization using Gaussian Process surrogate model and
// Expected Improvement acquisition function.
class BayesianOptimizer {
public:
    static OptResult run(IStrategy& strategy,
                         const std::vector<Bar>& bars,
                         const ParamSpace& param_space,
                         const BacktestConfig& bt_config,
                         const std::string& objective,
                         const BayesianOptConfig& bo_config);
};

} // namespace qp
