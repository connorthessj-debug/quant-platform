#include "reporting/report_generator.h"
#include "common/logging.h"
#include "common/time_utils.h"
#include <fstream>
#include <sstream>
#include <iomanip>

namespace qp {

namespace {

std::string side_str(Side s) {
    return (s == Side::Buy) ? "BUY" : "SELL";
}

std::string fmt(double v, int prec = 4) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(prec) << v;
    return ss.str();
}

std::string flag_str(bool b) { return b ? "PASS" : "FAIL"; }

} // namespace

bool ReportGenerator::write_trade_log(const std::vector<Trade>& trades,
                                       const std::string& path) {
    std::ofstream f(path);
    if (!f.is_open()) {
        Logger::log_error("Failed to open trade log: " + path);
        return false;
    }
    f << "trade_id,side,entry_time,entry_price,exit_time,exit_price,quantity,pnl,commission\n";
    for (const auto& t : trades) {
        f << t.id << ","
          << side_str(t.side) << ","
          << timestamp_to_string(t.entry_time) << ","
          << fmt(t.entry_price, 4) << ","
          << timestamp_to_string(t.exit_time) << ","
          << fmt(t.exit_price, 4) << ","
          << fmt(t.quantity, 2) << ","
          << fmt(t.pnl, 4) << ","
          << fmt(t.commission, 4) << "\n";
    }
    Logger::log_info("Wrote trade log (" + std::to_string(trades.size()) +
                    " trades) to " + path);
    return true;
}

bool ReportGenerator::write_equity_curve(const EquityCurve& curve,
                                          const std::string& path) {
    std::ofstream f(path);
    if (!f.is_open()) {
        Logger::log_error("Failed to open equity curve file: " + path);
        return false;
    }
    f << "timestamp,equity\n";
    for (size_t i = 0; i < curve.times.size() && i < curve.equity.size(); ++i) {
        f << timestamp_to_string(curve.times[i]) << "," << fmt(curve.equity[i], 4) << "\n";
    }
    return true;
}

bool ReportGenerator::write_text_report(const BacktestResult& bt_result,
                                         const BacktestSnapshot& s,
                                         const MCResult& mc,
                                         const OptResult& opt,
                                         const WFAResult& wfa,
                                         const TruthReport& truth,
                                         const std::string& strategy_name,
                                         const std::string& path) {
    std::ofstream f(path);
    if (!f.is_open()) return false;

    f << "================================================================\n";
    f << "  QUANT PLATFORM REPORT — Strategy: " << strategy_name << "\n";
    f << "================================================================\n\n";

    f << "--- Backtest ---\n";
    f << "Total Trades:         " << s.total_trades << "\n";
    f << "Win / Loss:           " << s.winning_trades << " / " << s.losing_trades << "\n";
    f << "Win Rate:             " << fmt(s.win_rate * 100.0, 2) << "%\n";
    f << "Total PnL:            $" << fmt(s.total_pnl, 2) << "\n";
    f << "Total Return:         " << fmt(s.total_return_pct, 2) << "%\n";
    f << "Final Equity:         $" << fmt(bt_result.final_equity, 2) << "\n";
    f << "Commission Paid:      $" << fmt(bt_result.total_commission, 2) << "\n\n";

    f << "--- Risk-Adjusted Returns ---\n";
    f << "Sharpe Ratio:         " << fmt(s.sharpe_ratio, 4) << "\n";
    f << "Sortino Ratio:        " << fmt(s.sortino_ratio, 4) << "\n";
    f << "Calmar Ratio:         " << fmt(s.calmar_ratio, 4) << "\n";
    f << "Omega Ratio:          " << fmt(s.omega_ratio, 4) << "\n";
    f << "CAGR:                 " << fmt(s.cagr * 100.0, 2) << "%\n\n";

    f << "--- Drawdown ---\n";
    f << "Max Drawdown:         $" << fmt(s.max_drawdown, 2) << "\n";
    f << "Max Drawdown %:       " << fmt(s.max_drawdown_pct, 2) << "%\n";
    f << "Ulcer Index:          " << fmt(s.ulcer_index, 4) << "\n";
    f << "Recovery Factor:      " << fmt(s.recovery_factor, 4) << "\n\n";

    f << "--- Trade Statistics ---\n";
    f << "Profit Factor:        " << fmt(s.profit_factor, 4) << "\n";
    f << "Expectancy:           $" << fmt(s.expectancy, 2) << "\n";
    f << "Payoff Ratio:         " << fmt(s.payoff_ratio, 4) << "\n";
    f << "Avg Win:              $" << fmt(s.avg_win, 2) << "\n";
    f << "Avg Loss:             $" << fmt(s.avg_loss, 2) << "\n";
    f << "Max Consec. Wins:     " << s.max_consecutive_wins << "\n";
    f << "Max Consec. Losses:   " << s.max_consecutive_losses << "\n\n";

    f << "--- Return Distribution ---\n";
    f << "Skewness:             " << fmt(s.skewness, 4) << "\n";
    f << "Excess Kurtosis:      " << fmt(s.kurtosis, 4) << "\n";
    f << "Tail Ratio:           " << fmt(s.tail_ratio, 4) << "\n";
    f << "VaR 95%:              " << fmt(s.value_at_risk_95 * 100.0, 4) << "%\n";
    f << "CVaR 95%:             " << fmt(s.conditional_var_95 * 100.0, 4) << "%\n\n";

    f << "--- Monte Carlo (1000 iterations) ---\n";
    f << "Mean Final Equity:    $" << fmt(mc.mean_final_equity, 2) << "\n";
    f << "Median Final Equity:  $" << fmt(mc.median_final_equity, 2) << "\n";
    f << "5th Percentile:       $" << fmt(mc.percentile_5, 2) << "\n";
    f << "95th Percentile:      $" << fmt(mc.percentile_95, 2) << "\n";
    f << "Survival Rate:        " << fmt(mc.survival_rate * 100.0, 2) << "%\n";
    f << "Ruin Probability:     " << fmt(mc.ruin_probability * 100.0, 2) << "%\n\n";

    f << "--- Optimizer (Bayesian) ---\n";
    f << "Evaluations:          " << opt.total_evaluations << "\n";
    f << "Best Objective:       " << fmt(opt.best_objective, 4) << "\n";
    f << "Best Parameters:\n";
    for (const auto& kv : opt.best_params.values) {
        f << "  " << kv.first << " = " << fmt(kv.second, 4) << "\n";
    }
    f << "\n";

    f << "--- Walk-Forward Analysis ---\n";
    f << "Windows:              " << wfa.windows.size() << "\n";
    f << "Consistency Score:    " << fmt(wfa.consistency_score, 4) << "\n";
    f << "Avg OOS Sharpe:       " << fmt(wfa.avg_oos_sharpe, 4) << "\n";
    f << "Consistent:           " << (wfa.is_consistent ? "YES" : "NO") << "\n\n";

    f << "--- Truth Engine ---\n";
    f << "Score:                " << fmt(truth.score, 2) << "/100\n";
    f << "Passed:               " << (truth.passed ? "YES" : "NO") << "\n";
    f << "Lookahead Clean:      " << flag_str(truth.lookahead_clean) << "\n";
    f << "Slippage Robust:      " << flag_str(truth.slippage_robust) << "\n";
    f << "Parameter Stable:     " << flag_str(truth.param_stable) << "\n";
    f << "WFA Consistent:       " << flag_str(truth.wfa_consistent) << "\n";
    f << "MC Survived:          " << flag_str(truth.mc_survived) << "\n";
    f << "Regime Independent:   " << flag_str(truth.regime_independent) << "\n";
    if (!truth.flags.empty()) {
        f << "\nFlags Raised:\n";
        for (const auto& fl : truth.flags) f << "  - " << fl << "\n";
    }

    Logger::log_info("Wrote text report to " + path);
    return true;
}

bool ReportGenerator::write_html_report(const BacktestResult& bt_result,
                                         const BacktestSnapshot& s,
                                         const MCResult& mc,
                                         const OptResult& opt,
                                         const WFAResult& wfa,
                                         const TruthReport& truth,
                                         const std::string& strategy_name,
                                         const std::string& path) {
    std::ofstream f(path);
    if (!f.is_open()) return false;

    auto row = [&](const std::string& k, const std::string& v) {
        f << "<tr><td>" << k << "</td><td>" << v << "</td></tr>\n";
    };

    f << "<!DOCTYPE html><html><head><meta charset='utf-8'>\n";
    f << "<title>Quant Platform Report — " << strategy_name << "</title>\n";
    f << "<style>\n";
    f << "body{font-family:system-ui,sans-serif;max-width:1000px;margin:2em auto;padding:0 1em;color:#222}\n";
    f << "h1{border-bottom:2px solid #333;padding-bottom:.3em}\n";
    f << "h2{margin-top:2em;color:#005}\n";
    f << "table{border-collapse:collapse;width:100%;margin:.5em 0}\n";
    f << "td{padding:.35em .8em;border-bottom:1px solid #ddd}\n";
    f << "td:first-child{font-weight:600;width:40%;color:#333}\n";
    f << ".pass{color:#080;font-weight:700}.fail{color:#c00;font-weight:700}\n";
    f << ".big{font-size:1.4em;font-weight:700}\n";
    f << "</style></head><body>\n";

    f << "<h1>Quant Platform Report</h1>\n";
    f << "<p><strong>Strategy:</strong> " << strategy_name << "</p>\n";
    f << "<p class='big'>Truth Engine Score: <span class='"
      << (truth.passed ? "pass" : "fail") << "'>" << fmt(truth.score, 1)
      << "/100 — " << (truth.passed ? "PASSED" : "FAILED") << "</span></p>\n";

    f << "<h2>Backtest Summary</h2><table>\n";
    row("Total Trades", std::to_string(s.total_trades));
    row("Win / Loss", std::to_string(s.winning_trades) + " / " + std::to_string(s.losing_trades));
    row("Win Rate", fmt(s.win_rate * 100.0, 2) + "%");
    row("Total PnL", "$" + fmt(s.total_pnl, 2));
    row("Total Return", fmt(s.total_return_pct, 2) + "%");
    row("Final Equity", "$" + fmt(bt_result.final_equity, 2));
    row("Commission", "$" + fmt(bt_result.total_commission, 2));
    f << "</table>\n";

    f << "<h2>Risk-Adjusted Returns</h2><table>\n";
    row("Sharpe Ratio", fmt(s.sharpe_ratio, 4));
    row("Sortino Ratio", fmt(s.sortino_ratio, 4));
    row("Calmar Ratio", fmt(s.calmar_ratio, 4));
    row("Omega Ratio", fmt(s.omega_ratio, 4));
    row("CAGR", fmt(s.cagr * 100.0, 2) + "%");
    f << "</table>\n";

    f << "<h2>Drawdown & Risk</h2><table>\n";
    row("Max Drawdown", "$" + fmt(s.max_drawdown, 2));
    row("Max Drawdown %", fmt(s.max_drawdown_pct, 2) + "%");
    row("Ulcer Index", fmt(s.ulcer_index, 4));
    row("Recovery Factor", fmt(s.recovery_factor, 4));
    row("VaR 95%", fmt(s.value_at_risk_95 * 100.0, 4) + "%");
    row("CVaR 95%", fmt(s.conditional_var_95 * 100.0, 4) + "%");
    f << "</table>\n";

    f << "<h2>Trade Statistics</h2><table>\n";
    row("Profit Factor", fmt(s.profit_factor, 4));
    row("Expectancy", "$" + fmt(s.expectancy, 2));
    row("Payoff Ratio", fmt(s.payoff_ratio, 4));
    row("Avg Win", "$" + fmt(s.avg_win, 2));
    row("Avg Loss", "$" + fmt(s.avg_loss, 2));
    row("Max Consecutive Wins", std::to_string(s.max_consecutive_wins));
    row("Max Consecutive Losses", std::to_string(s.max_consecutive_losses));
    f << "</table>\n";

    f << "<h2>Return Distribution</h2><table>\n";
    row("Skewness", fmt(s.skewness, 4));
    row("Excess Kurtosis", fmt(s.kurtosis, 4));
    row("Tail Ratio", fmt(s.tail_ratio, 4));
    f << "</table>\n";

    f << "<h2>Monte Carlo</h2><table>\n";
    row("Mean Final Equity", "$" + fmt(mc.mean_final_equity, 2));
    row("Median Final Equity", "$" + fmt(mc.median_final_equity, 2));
    row("5th Percentile", "$" + fmt(mc.percentile_5, 2));
    row("95th Percentile", "$" + fmt(mc.percentile_95, 2));
    row("Survival Rate", fmt(mc.survival_rate * 100.0, 2) + "%");
    row("Ruin Probability", fmt(mc.ruin_probability * 100.0, 2) + "%");
    f << "</table>\n";

    f << "<h2>Bayesian Optimizer</h2><table>\n";
    row("Evaluations", std::to_string(opt.total_evaluations));
    row("Best Objective", fmt(opt.best_objective, 4));
    for (const auto& kv : opt.best_params.values) {
        row("Best " + kv.first, fmt(kv.second, 4));
    }
    f << "</table>\n";

    f << "<h2>Walk-Forward Analysis</h2><table>\n";
    row("Windows", std::to_string(wfa.windows.size()));
    row("Consistency Score", fmt(wfa.consistency_score, 4));
    row("Avg OOS Sharpe", fmt(wfa.avg_oos_sharpe, 4));
    row("Consistent", wfa.is_consistent ? "YES" : "NO");
    f << "</table>\n";

    f << "<h2>Truth Engine Checks</h2><table>\n";
    auto pass_row = [&](const std::string& k, bool v) {
        f << "<tr><td>" << k << "</td><td class='" << (v ? "pass" : "fail") << "'>"
          << (v ? "PASS" : "FAIL") << "</td></tr>\n";
    };
    pass_row("Lookahead Clean", truth.lookahead_clean);
    pass_row("Slippage Robust", truth.slippage_robust);
    pass_row("Parameter Stable", truth.param_stable);
    pass_row("WFA Consistent", truth.wfa_consistent);
    pass_row("MC Survived", truth.mc_survived);
    pass_row("Regime Independent", truth.regime_independent);
    f << "</table>\n";

    if (!truth.flags.empty()) {
        f << "<h2>Flags Raised</h2><ul>\n";
        for (const auto& fl : truth.flags) f << "<li>" << fl << "</li>\n";
        f << "</ul>\n";
    }

    f << "</body></html>\n";
    Logger::log_info("Wrote HTML report to " + path);
    return true;
}

} // namespace qp
