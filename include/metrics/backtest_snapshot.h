#pragma once

namespace qp {

struct BacktestSnapshot {
    double sharpe_ratio = 0.0;
    double sortino_ratio = 0.0;
    double max_drawdown = 0.0;
    double max_drawdown_pct = 0.0;
    double profit_factor = 0.0;
    double win_rate = 0.0;
    double expectancy = 0.0;
    double total_pnl = 0.0;
    int total_trades = 0;
    int winning_trades = 0;
    int losing_trades = 0;
};

} // namespace qp
