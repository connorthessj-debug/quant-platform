#include "optimization/optimizer.h"
#include "common/logging.h"
#include <limits>

namespace qp {

// Generate all parameter combinations from the ParamSpace
std::vector<StrategyParams> ParamSpace::generate_combinations() const {
    std::vector<StrategyParams> combos;
    if (ranges.empty()) return combos;

    // Build value lists for each range
    std::vector<std::vector<double>> value_lists;
    for (const auto& r : ranges) {
        std::vector<double> vals;
        for (double v = r.min_val; v <= r.max_val + 1e-9; v += r.step) {
            vals.push_back(v);
        }
        if (vals.empty()) vals.push_back(r.min_val);
        value_lists.push_back(vals);
    }

    // Cartesian product via iterative enumeration
    std::vector<size_t> indices(ranges.size(), 0);
    bool done = false;
    while (!done) {
        StrategyParams params;
        for (size_t i = 0; i < ranges.size(); ++i) {
            params.set(ranges[i].name, value_lists[i][indices[i]]);
        }
        combos.push_back(params);

        // Increment indices (odometer-style)
        int carry = static_cast<int>(ranges.size()) - 1;
        while (carry >= 0) {
            indices[carry]++;
            if (indices[carry] < value_lists[carry].size()) break;
            indices[carry] = 0;
            carry--;
        }
        if (carry < 0) done = true;
    }

    return combos;
}

double Optimizer::evaluate_objective(const BacktestSnapshot& snapshot,
                                      const std::string& objective) {
    if (objective == "sharpe") return snapshot.sharpe_ratio;
    if (objective == "sortino") return snapshot.sortino_ratio;
    if (objective == "profit_factor") return snapshot.profit_factor;
    if (objective == "expectancy") return snapshot.expectancy;
    if (objective == "pnl") return snapshot.total_pnl;
    return snapshot.sharpe_ratio;
}

OptResult Optimizer::run(IStrategy& strategy,
                          const std::vector<Bar>& bars,
                          const ParamSpace& param_space,
                          const BacktestConfig& bt_config,
                          const std::string& objective,
                          int max_iterations) {
    OptResult result;
    auto combos = param_space.generate_combinations();

    if (max_iterations > 0 && static_cast<int>(combos.size()) > max_iterations) {
        combos.resize(max_iterations);
    }

    Logger::log_info("Optimizer: evaluating " + std::to_string(combos.size()) + " combinations");

    BacktestEngine engine;
    double best_obj = -std::numeric_limits<double>::infinity();

    for (const auto& params : combos) {
        auto clone = strategy.clone();
        clone->configure(params);
        BacktestResult bt_result = engine.run(*clone, bars, bt_config);
        BacktestSnapshot snapshot = MetricsEngine::compute(bt_result.trades,
                                                            bt_result.equity_curve,
                                                            bt_config.initial_capital);

        double obj_val = evaluate_objective(snapshot, objective);

        OptTrialResult trial;
        trial.params = params;
        trial.metrics = snapshot;
        trial.objective_value = obj_val;
        result.all_trials.push_back(trial);

        if (obj_val > best_obj) {
            best_obj = obj_val;
            result.best_params = params;
            result.best_metrics = snapshot;
            result.best_objective = obj_val;
        }
    }

    result.total_evaluations = static_cast<int>(combos.size());

    Logger::log_info("Optimization complete: best " + objective + " = " +
                    std::to_string(best_obj));

    return result;
}

StrategyParams Optimizer::find_best(IStrategy& strategy,
                                     const std::vector<Bar>& bars,
                                     const ParamSpace& param_space,
                                     const BacktestConfig& bt_config,
                                     const std::string& objective) {
    OptResult result = run(strategy, bars, param_space, bt_config, objective);
    return result.best_params;
}

} // namespace qp
