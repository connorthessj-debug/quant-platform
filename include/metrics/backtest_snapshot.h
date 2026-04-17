#pragma once

namespace qp {

struct BacktestSnapshot {
    // Core metrics
    double sharpe_ratio = 0.0;
    double sortino_ratio = 0.0;
    double calmar_ratio = 0.0;
    double omega_ratio = 0.0;
    double max_drawdown = 0.0;
    double max_drawdown_pct = 0.0;
    double cagr = 0.0;
    double profit_factor = 0.0;
    double win_rate = 0.0;
    double expectancy = 0.0;
    double total_pnl = 0.0;
    double total_return_pct = 0.0;

    // Trade statistics
    int total_trades = 0;
    int winning_trades = 0;
    int losing_trades = 0;
    double avg_win = 0.0;
    double avg_loss = 0.0;
    double payoff_ratio = 0.0;          // avg_win / |avg_loss|
    int max_consecutive_wins = 0;
    int max_consecutive_losses = 0;

    // Distribution statistics
    double skewness = 0.0;              // 3rd standardized moment of returns
    double kurtosis = 0.0;              // Excess kurtosis (4th moment - 3)
    double tail_ratio = 0.0;            // p95(returns) / |p5(returns)|
    double value_at_risk_95 = 0.0;      // 5th percentile of returns
    double conditional_var_95 = 0.0;    // Expected return given worse than VaR
    double ulcer_index = 0.0;           // sqrt(mean(drawdown_pct^2))
    double recovery_factor = 0.0;       // total_pnl / max_drawdown
};

} // namespace qp
