#pragma once

#include "common/types.h"
#include "core/portfolio.h"
#include "core/commission_model.h"
#include "strategy/strategy_base.h"
#include <vector>

namespace qp {

struct BacktestConfig {
    double initial_capital = 100000.0;
    double commission_per_trade = 1.0;
    double slippage_bps = 5.0;
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
