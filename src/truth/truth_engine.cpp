#include "truth/truth_engine.h"
#include "common/logging.h"
#include "core/backtest_engine.h"
#include "metrics/metrics_engine.h"
#include <cmath>
#include <algorithm>

namespace qp {

TruthReport TruthEngine::validate(IStrategy& strategy,
                                    const std::vector<Bar>& bars,
                                    const BacktestConfig& bt_config,
                                    const BacktestResult& base_result,
                                    const BacktestSnapshot& base_metrics,
                                    const MCResult& mc_result,
                                    const WFAResult& wfa_result,
                                    const OptResult& opt_result,
                                    const TruthEngineConfig& config) {
    TruthReport report;
    report.base_metrics = base_metrics;

    Logger::log_info("Truth Engine: validating strategy...");

    int checks_passed = 0;
    int total_checks = 6;

    // 1. Lookahead bias check
    report.lookahead_clean = check_lookahead(base_result.trades, bars);
    if (report.lookahead_clean) {
        checks_passed++;
    } else {
        report.flags.push_back("LOOKAHEAD_BIAS_DETECTED");
    }

    // 2. Slippage robustness
    report.slippage_robust = check_slippage_robustness(strategy, bars, bt_config,
                                                         base_metrics,
                                                         config.slippage_perturbation_bps);
    if (report.slippage_robust) {
        checks_passed++;
    } else {
        report.flags.push_back("SLIPPAGE_FRAGILE");
    }

    // 3. Parameter stability
    report.param_stable = check_param_stability(opt_result);
    if (report.param_stable) {
        checks_passed++;
    } else {
        report.flags.push_back("PARAMETER_UNSTABLE");
    }

    // 4. WFA consistency
    report.wfa_consistent = check_wfa_consistency(wfa_result,
                                                    config.wfa_consistency_threshold);
    if (report.wfa_consistent) {
        checks_passed++;
    } else {
        report.flags.push_back("WFA_INCONSISTENT");
    }

    // 5. Monte Carlo survival
    report.mc_survived = check_mc_survival(mc_result, config.mc_survival_threshold);
    if (report.mc_survived) {
        checks_passed++;
    } else {
        report.flags.push_back("MC_SURVIVAL_LOW");
    }

    // 6. Regime independence
    report.regime_independent = check_regime_independence(base_result.trades, bars);
    if (report.regime_independent) {
        checks_passed++;
    } else {
        report.flags.push_back("REGIME_DEPENDENT");
    }

    report.score = (static_cast<double>(checks_passed) / total_checks) * 100.0;
    report.passed = (report.score >= 60.0) &&
                    base_metrics.sharpe_ratio >= config.min_sharpe &&
                    base_metrics.max_drawdown_pct <= config.max_drawdown_pct &&
                    base_metrics.profit_factor >= config.min_profit_factor;

    Logger::log_info("Truth Engine: score=" + std::to_string(report.score) +
                    " passed=" + (report.passed ? "YES" : "NO") +
                    " flags=" + std::to_string(report.flags.size()));

    return report;
}

bool TruthEngine::check_lookahead(const std::vector<Trade>& trades,
                                    const std::vector<Bar>& bars) {
    if (trades.empty() || bars.empty()) return true;

    Timestamp first_bar_time = bars.front().time;
    for (const auto& trade : trades) {
        if (trade.entry_time < first_bar_time) {
            return false;
        }
        if (trade.exit_time < trade.entry_time) {
            return false;
        }
    }
    return true;
}

bool TruthEngine::check_slippage_robustness(IStrategy& strategy,
                                              const std::vector<Bar>& bars,
                                              const BacktestConfig& base_config,
                                              const BacktestSnapshot& base_metrics,
                                              double perturbation_bps) {
    BacktestConfig perturbed_config = base_config;
    perturbed_config.slippage_bps = base_config.slippage_bps + perturbation_bps;

    BacktestEngine engine;
    auto clone = strategy.clone();
    BacktestResult perturbed_result = engine.run(*clone, bars, perturbed_config);
    BacktestSnapshot perturbed_metrics = MetricsEngine::compute(
        perturbed_result.trades, perturbed_result.equity_curve,
        perturbed_config.initial_capital);

    // Strategy is robust if perturbed Sharpe doesn't drop more than 50%
    if (base_metrics.sharpe_ratio <= 0.0) return false;
    double ratio = perturbed_metrics.sharpe_ratio / base_metrics.sharpe_ratio;
    return ratio >= 0.5;
}

bool TruthEngine::check_param_stability(const OptResult& opt_result) {
    if (opt_result.all_trials.size() < 3) return true;

    // Check if the top 20% of parameter combinations have similar performance
    std::vector<double> objectives;
    for (const auto& trial : opt_result.all_trials) {
        objectives.push_back(trial.objective_value);
    }
    std::sort(objectives.rbegin(), objectives.rend());

    int top_n = std::max(3, static_cast<int>(objectives.size() * 0.2));
    double best = objectives[0];
    if (best <= 0.0) return false;

    double worst_in_top = objectives[std::min(top_n - 1, static_cast<int>(objectives.size()) - 1)];
    double stability = worst_in_top / best;
    return stability >= 0.5;
}

bool TruthEngine::check_wfa_consistency(const WFAResult& wfa_result, double threshold) {
    return wfa_result.consistency_score >= threshold;
}

bool TruthEngine::check_mc_survival(const MCResult& mc_result, double threshold) {
    return mc_result.survival_rate >= threshold;
}

bool TruthEngine::check_regime_independence(const std::vector<Trade>& trades,
                                              const std::vector<Bar>& bars) {
    if (trades.size() < 10 || bars.empty()) return true;

    // Split trades into first half and second half by time
    size_t mid = trades.size() / 2;
    double first_half_pnl = 0.0;
    double second_half_pnl = 0.0;

    for (size_t i = 0; i < mid; ++i) first_half_pnl += trades[i].pnl;
    for (size_t i = mid; i < trades.size(); ++i) second_half_pnl += trades[i].pnl;

    // Both halves should be profitable, or at least not wildly different
    if (first_half_pnl <= 0.0 && second_half_pnl <= 0.0) return false;
    if (first_half_pnl <= 0.0 || second_half_pnl <= 0.0) return false;

    double ratio = std::min(first_half_pnl, second_half_pnl) /
                   std::max(first_half_pnl, second_half_pnl);
    return ratio >= 0.2;
}

} // namespace qp
