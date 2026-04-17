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
    static double omega_ratio(const std::vector<double>& returns, double threshold = 0.0);
    static double max_drawdown(const std::vector<double>& equity);
    static double max_drawdown_pct(const std::vector<double>& equity);
    static double ulcer_index(const std::vector<double>& equity);
    static double cagr(const std::vector<double>& equity, const std::vector<Timestamp>& times,
                       double initial_capital);
    static double profit_factor(const std::vector<Trade>& trades);
    static double win_rate(const std::vector<Trade>& trades);
    static double expectancy(const std::vector<Trade>& trades);
    static double skewness(const std::vector<double>& returns);
    static double kurtosis(const std::vector<double>& returns);
    static double percentile(std::vector<double> values, double p);
};

} // namespace qp
