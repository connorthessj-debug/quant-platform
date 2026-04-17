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

    // Always compute equity-curve-based metrics, even with zero trades.
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
    snap.omega_ratio = omega_ratio(returns, 0.0);
    snap.max_drawdown = max_drawdown(eq);
    snap.max_drawdown_pct = max_drawdown_pct(eq);
    snap.ulcer_index = ulcer_index(eq);
    snap.skewness = skewness(returns);
    snap.kurtosis = kurtosis(returns);

    // Tail ratio & VaR from daily returns distribution.
    if (!returns.empty()) {
        double p5 = percentile(returns, 5.0);
        double p95 = percentile(returns, 95.0);
        snap.value_at_risk_95 = p5;
        snap.tail_ratio = (std::abs(p5) > 1e-12) ? p95 / std::abs(p5) : 0.0;

        // Conditional VaR: mean of returns worse than p5.
        double cvar_sum = 0.0;
        int cvar_count = 0;
        for (double r : returns) {
            if (r <= p5) { cvar_sum += r; cvar_count++; }
        }
        snap.conditional_var_95 = (cvar_count > 0) ? cvar_sum / cvar_count : 0.0;
    }

    // CAGR from equity curve + timestamps.
    snap.cagr = cagr(eq, equity_curve.times, initial_capital);

    // Calmar = CAGR / MaxDrawdown% (both in percent).
    if (snap.max_drawdown_pct > 1e-9) {
        snap.calmar_ratio = (snap.cagr * 100.0) / snap.max_drawdown_pct;
    }

    if (trades.empty()) return snap;

    double total_pnl = 0.0;
    double gross_profit = 0.0;
    double gross_loss = 0.0;
    int winners = 0;
    int losers = 0;
    double win_sum = 0.0;
    double loss_sum = 0.0;
    int cur_win_streak = 0;
    int cur_loss_streak = 0;
    int max_win_streak = 0;
    int max_loss_streak = 0;

    for (const auto& t : trades) {
        total_pnl += t.pnl;
        if (t.pnl > 0.0) {
            gross_profit += t.pnl;
            winners++;
            win_sum += t.pnl;
            cur_win_streak++;
            cur_loss_streak = 0;
            if (cur_win_streak > max_win_streak) max_win_streak = cur_win_streak;
        } else {
            gross_loss += std::abs(t.pnl);
            losers++;
            loss_sum += t.pnl;
            cur_loss_streak++;
            cur_win_streak = 0;
            if (cur_loss_streak > max_loss_streak) max_loss_streak = cur_loss_streak;
        }
    }

    snap.total_pnl = total_pnl;
    snap.winning_trades = winners;
    snap.losing_trades = losers;
    snap.win_rate = static_cast<double>(winners) / snap.total_trades;
    snap.profit_factor = (gross_loss > 0.0) ? gross_profit / gross_loss : 0.0;
    snap.expectancy = total_pnl / snap.total_trades;
    snap.avg_win = (winners > 0) ? win_sum / winners : 0.0;
    snap.avg_loss = (losers > 0) ? loss_sum / losers : 0.0;
    snap.payoff_ratio = (std::abs(snap.avg_loss) > 1e-12) ? snap.avg_win / std::abs(snap.avg_loss) : 0.0;
    snap.max_consecutive_wins = max_win_streak;
    snap.max_consecutive_losses = max_loss_streak;
    snap.total_return_pct = (initial_capital > 0.0) ? (total_pnl / initial_capital) * 100.0 : 0.0;
    snap.recovery_factor = (snap.max_drawdown > 1e-9) ? total_pnl / snap.max_drawdown : 0.0;

    return snap;
}

double MetricsEngine::sharpe_ratio(const std::vector<double>& returns) {
    if (returns.size() < 2) return 0.0;
    double mean = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
    double sq_sum = 0.0;
    for (double r : returns) sq_sum += (r - mean) * (r - mean);
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

double MetricsEngine::omega_ratio(const std::vector<double>& returns, double threshold) {
    double gain = 0.0;
    double loss = 0.0;
    for (double r : returns) {
        double diff = r - threshold;
        if (diff > 0.0) gain += diff;
        else loss += -diff;
    }
    return (loss > 1e-12) ? gain / loss : 0.0;
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

double MetricsEngine::ulcer_index(const std::vector<double>& equity) {
    if (equity.empty()) return 0.0;
    double peak = equity[0];
    double sum_sq = 0.0;
    for (double e : equity) {
        if (e > peak) peak = e;
        if (peak > 0.0) {
            double dd_pct = (peak - e) / peak * 100.0;
            sum_sq += dd_pct * dd_pct;
        }
    }
    return std::sqrt(sum_sq / equity.size());
}

double MetricsEngine::cagr(const std::vector<double>& equity,
                            const std::vector<Timestamp>& times,
                            double initial_capital) {
    if (equity.size() < 2 || times.size() != equity.size()) return 0.0;
    if (initial_capital <= 0.0) return 0.0;
    double final_equity = equity.back();
    if (final_equity <= 0.0) return -1.0;

    // Seconds -> years (365.25 days/year).
    double elapsed_sec = static_cast<double>(times.back() - times.front());
    double years = elapsed_sec / (365.25 * 86400.0);
    if (years < 1e-6) return 0.0;

    return std::pow(final_equity / initial_capital, 1.0 / years) - 1.0;
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
    for (const auto& t : trades) if (t.pnl > 0.0) winners++;
    return static_cast<double>(winners) / trades.size();
}

double MetricsEngine::expectancy(const std::vector<Trade>& trades) {
    if (trades.empty()) return 0.0;
    double total = 0.0;
    for (const auto& t : trades) total += t.pnl;
    return total / trades.size();
}

double MetricsEngine::skewness(const std::vector<double>& returns) {
    if (returns.size() < 3) return 0.0;
    double n = static_cast<double>(returns.size());
    double mean = std::accumulate(returns.begin(), returns.end(), 0.0) / n;
    double m2 = 0.0, m3 = 0.0;
    for (double r : returns) {
        double d = r - mean;
        m2 += d * d;
        m3 += d * d * d;
    }
    m2 /= n;
    m3 /= n;
    double std_dev = std::sqrt(m2);
    if (std_dev < 1e-12) return 0.0;
    return m3 / (std_dev * std_dev * std_dev);
}

double MetricsEngine::kurtosis(const std::vector<double>& returns) {
    if (returns.size() < 4) return 0.0;
    double n = static_cast<double>(returns.size());
    double mean = std::accumulate(returns.begin(), returns.end(), 0.0) / n;
    double m2 = 0.0, m4 = 0.0;
    for (double r : returns) {
        double d = r - mean;
        double d2 = d * d;
        m2 += d2;
        m4 += d2 * d2;
    }
    m2 /= n;
    m4 /= n;
    if (m2 < 1e-24) return 0.0;
    return (m4 / (m2 * m2)) - 3.0; // excess kurtosis
}

double MetricsEngine::percentile(std::vector<double> values, double p) {
    if (values.empty()) return 0.0;
    std::sort(values.begin(), values.end());
    double rank = (p / 100.0) * (values.size() - 1);
    size_t lo = static_cast<size_t>(std::floor(rank));
    size_t hi = static_cast<size_t>(std::ceil(rank));
    if (lo == hi) return values[lo];
    double frac = rank - lo;
    return values[lo] * (1.0 - frac) + values[hi] * frac;
}

} // namespace qp
