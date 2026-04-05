#include "metrics/metrics_engine.h"
#include <cmath>
#include <algorithm>
#include <numeric>

namespace qp {

BacktestSnapshot MetricsEngine::compute(const std::vector<Trade>& trades,
                                          const EquityCurve& equity_curve,
                                          double initial_capital) {
    BacktestSnapshot snap;
    snap.total_trades = static_cast<int>(trades.size());

    // Always compute equity-curve-based metrics, even with zero trades
    std::vector<double> returns;
    const auto& eq = equity_curve.equity;
    if (eq.size() > 1) {
        for (size_t i = 1; i < eq.size(); ++i) {
            if (eq[i - 1] != 0.0) {
                returns.push_back((eq[i] - eq[i - 1]) / eq[i - 1]);
            }
        }
    }
    snap.sharpe_ratio = sharpe_ratio(returns);
    snap.sortino_ratio = sortino_ratio(returns);
    snap.max_drawdown = max_drawdown(eq);
    snap.max_drawdown_pct = max_drawdown_pct(eq);

    if (trades.empty()) return snap;

    double total_pnl = 0.0;
    double gross_profit = 0.0;
    double gross_loss = 0.0;
    int winners = 0;
    int losers = 0;

    for (const auto& t : trades) {
        total_pnl += t.pnl;
        if (t.pnl > 0.0) {
            gross_profit += t.pnl;
            winners++;
        } else {
            gross_loss += std::abs(t.pnl);
            losers++;
        }
    }

    snap.total_pnl = total_pnl;
    snap.winning_trades = winners;
    snap.losing_trades = losers;
    snap.win_rate = static_cast<double>(winners) / snap.total_trades;
    snap.profit_factor = (gross_loss > 0.0) ? gross_profit / gross_loss : 0.0;
    snap.expectancy = total_pnl / snap.total_trades;

    return snap;
}

double MetricsEngine::sharpe_ratio(const std::vector<double>& returns) {
    if (returns.size() < 2) return 0.0;
    double mean = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
    double sq_sum = 0.0;
    for (double r : returns) {
        sq_sum += (r - mean) * (r - mean);
    }
    double stddev = std::sqrt(sq_sum / (returns.size() - 1));
    if (stddev == 0.0) return 0.0;
    return (mean / stddev) * std::sqrt(252.0);
}

double MetricsEngine::sortino_ratio(const std::vector<double>& returns) {
    if (returns.size() < 2) return 0.0;
    double mean = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
    double downside_sq_sum = 0.0;
    int count = 0;
    for (double r : returns) {
        if (r < 0.0) {
            downside_sq_sum += r * r;
            count++;
        }
    }
    if (count == 0) return 0.0;
    double downside_dev = std::sqrt(downside_sq_sum / count);
    if (downside_dev == 0.0) return 0.0;
    return (mean / downside_dev) * std::sqrt(252.0);
}

double MetricsEngine::max_drawdown(const std::vector<double>& equity) {
    if (equity.empty()) return 0.0;
    double peak = equity[0];
    double max_dd = 0.0;
    for (double e : equity) {
        if (e > peak) peak = e;
        double dd = peak - e;
        if (dd > max_dd) max_dd = dd;
    }
    return max_dd;
}

double MetricsEngine::max_drawdown_pct(const std::vector<double>& equity) {
    if (equity.empty()) return 0.0;
    double peak = equity[0];
    double max_dd_pct = 0.0;
    for (double e : equity) {
        if (e > peak) peak = e;
        if (peak > 0.0) {
            double dd_pct = (peak - e) / peak * 100.0;
            if (dd_pct > max_dd_pct) max_dd_pct = dd_pct;
        }
    }
    return max_dd_pct;
}

double MetricsEngine::profit_factor(const std::vector<Trade>& trades) {
    double gross_profit = 0.0;
    double gross_loss = 0.0;
    for (const auto& t : trades) {
        if (t.pnl > 0.0) gross_profit += t.pnl;
        else gross_loss += std::abs(t.pnl);
    }
    return (gross_loss > 0.0) ? gross_profit / gross_loss : 0.0;
}

double MetricsEngine::win_rate(const std::vector<Trade>& trades) {
    if (trades.empty()) return 0.0;
    int winners = 0;
    for (const auto& t : trades) {
        if (t.pnl > 0.0) winners++;
    }
    return static_cast<double>(winners) / trades.size();
}

double MetricsEngine::expectancy(const std::vector<Trade>& trades) {
    if (trades.empty()) return 0.0;
    double total = 0.0;
    for (const auto& t : trades) total += t.pnl;
    return total / trades.size();
}

} // namespace qp
