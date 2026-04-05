#include "walkforward/walk_forward.h"
#include "metrics/metrics_engine.h"
#include "common/logging.h"
#include <cmath>

namespace qp {

WFAResult WalkForward::run(IStrategy& strategy,
                            const std::vector<Bar>& bars,
                            const ParamSpace& param_space,
                            const BacktestConfig& bt_config,
                            const WFAConfig& wfa_config,
                            OptimizerFunc optimizer_func) {
    WFAResult result;

    int total_bars = static_cast<int>(bars.size());
    if (total_bars < 2 * wfa_config.num_windows) {
        Logger::log_error("Walk-forward: not enough bars for " +
                         std::to_string(wfa_config.num_windows) + " windows");
        return result;
    }

    int window_size = total_bars / wfa_config.num_windows;
    int is_size = static_cast<int>(window_size * wfa_config.in_sample_ratio);
    int oos_size = window_size - is_size;

    Logger::log_info("Walk-forward: " + std::to_string(wfa_config.num_windows) +
                    " windows, IS=" + std::to_string(is_size) +
                    " OOS=" + std::to_string(oos_size));

    BacktestEngine engine;
    double total_oos_sharpe = 0.0;
    int profitable_windows = 0;

    for (int w = 0; w < wfa_config.num_windows; ++w) {
        WFAWindow window;
        window.in_sample_start = w * window_size;
        window.in_sample_end = window.in_sample_start + is_size;
        window.out_of_sample_start = window.in_sample_end;
        window.out_of_sample_end = std::min(window.out_of_sample_start + oos_size, total_bars);

        // Extract in-sample bars
        std::vector<Bar> is_bars(bars.begin() + window.in_sample_start,
                                  bars.begin() + window.in_sample_end);

        // Optimize on in-sample
        auto clone = strategy.clone();
        StrategyParams best_params = optimizer_func(*clone, is_bars, param_space,
                                                      bt_config,
                                                      wfa_config.optimization_objective);
        window.best_params = best_params;

        // Run IS backtest with best params
        clone->configure(best_params);
        BacktestResult is_result = engine.run(*clone, is_bars, bt_config);
        window.in_sample_metrics = MetricsEngine::compute(is_result.trades,
                                                     is_result.equity_curve,
                                                     bt_config.initial_capital);

        // Extract OOS bars
        std::vector<Bar> oos_bars(bars.begin() + window.out_of_sample_start,
                                   bars.begin() + window.out_of_sample_end);

        // Run OOS backtest with same params
        clone->reset();
        clone->configure(best_params);
        BacktestResult oos_result = engine.run(*clone, oos_bars, bt_config);
        window.out_of_sample_metrics = MetricsEngine::compute(oos_result.trades,
                                                         oos_result.equity_curve,
                                                         bt_config.initial_capital);

        total_oos_sharpe += window.out_of_sample_metrics.sharpe_ratio;
        if (window.out_of_sample_metrics.total_pnl > 0.0) {
            profitable_windows++;
        }

        result.windows.push_back(window);
    }

    result.avg_oos_sharpe = total_oos_sharpe / wfa_config.num_windows;
    result.consistency_score = static_cast<double>(profitable_windows) / wfa_config.num_windows;
    result.is_consistent = result.consistency_score >= 0.6;

    Logger::log_info("Walk-forward complete: consistency=" +
                    std::to_string(result.consistency_score) +
                    " avg OOS Sharpe=" + std::to_string(result.avg_oos_sharpe));

    return result;
}

} // namespace qp
