#pragma once

#include "common/types.h"
#include "core/portfolio.h"
#include "core/commission_model.h"
#include "strategy/strategy_base.h"
#include <vector>

namespace qp {

enum class PositionSizing {
    Fixed,              // Use strategy position_size literally
    FixedFractional,    // risk_per_trade_pct * equity / stop_distance
    Kelly               // Kelly fraction * equity / price
};

struct BacktestConfig {
    double initial_capital = 100000.0;
    double commission_per_trade = 1.0;
    double slippage_bps = 5.0;

    // Risk controls (0.0 = disabled).
    double stop_loss_pct = 0.0;       // e.g. 2.0 = close if down 2% from entry
    double take_profit_pct = 0.0;     // e.g. 4.0 = close if up 4% from entry

    // Position sizing.
    PositionSizing position_sizing = PositionSizing::Fixed;
    double risk_per_trade_pct = 1.0;  // For FixedFractional: % of equity to risk
    double kelly_fraction = 0.5;      // Fractional Kelly (0.5 = half-Kelly)
    double assumed_win_rate = 0.5;    // For Kelly when no history
    double assumed_payoff_ratio = 1.5;
};

struct BacktestResult {
    std::vector<Trade> trades;
    EquityCurve equity_curve;
    double final_equity = 0.0;
    double total_commission = 0.0;
    int total_bars = 0;
};

class BacktestEngine {
public:
    BacktestResult run(IStrategy& strategy,
                       const std::vector<Bar>& bars,
                       const BacktestConfig& config);
};

} // namespace qp
