#pragma once

#include "common/types.h"
#include "metrics/backtest_snapshot.h"
#include <vector>

namespace qp {

class MetricsEngine {
public:
    static BacktestSnapshot compute(const std::vector<Trade>& trades,
                                     const EquityCurve& equity_curve,
                                     double initial_capital);

    static double sharpe_ratio(const std::vector<double>& returns);
    static double sortino_ratio(const std::vector<double>& returns);
    static double max_drawdown(const std::vector<double>& equity);
    static double max_drawdown_pct(const std::vector<double>& equity);
    static double profit_factor(const std::vector<Trade>& trades);
    static double win_rate(const std::vector<Trade>& trades);
    static double expectancy(const std::vector<Trade>& trades);
};

} // namespace qp
