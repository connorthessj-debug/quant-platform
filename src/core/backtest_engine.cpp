#include "core/backtest_engine.h"
#include "common/logging.h"
#include <cmath>
#include <algorithm>

namespace qp {

namespace {

double compute_position_size(const BacktestConfig& cfg,
                              double requested_quantity,
                              double entry_price,
                              double current_equity) {
    if (cfg.position_sizing == PositionSizing::Fixed) {
        return requested_quantity;
    }

    if (entry_price <= 0.0 || current_equity <= 0.0) return requested_quantity;

    if (cfg.position_sizing == PositionSizing::FixedFractional) {
        double risk_dollars = current_equity * (cfg.risk_per_trade_pct / 100.0);
        double stop_distance = entry_price * std::max(cfg.stop_loss_pct, 1.0) / 100.0;
        if (stop_distance <= 1e-9) return requested_quantity;
        return std::max(1.0, std::floor(risk_dollars / stop_distance));
    }

    if (cfg.position_sizing == PositionSizing::Kelly) {
        double p = cfg.assumed_win_rate;
        double b = cfg.assumed_payoff_ratio;
        if (b <= 0.0) return requested_quantity;
        // Kelly: f* = p - (1-p)/b
        double kelly = p - (1.0 - p) / b;
        if (kelly <= 0.0) return 1.0;
        double frac = kelly * cfg.kelly_fraction;
        double dollars = current_equity * frac;
        return std::max(1.0, std::floor(dollars / entry_price));
    }

    return requested_quantity;
}

} // namespace

BacktestResult BacktestEngine::run(IStrategy& strategy,
                                    const std::vector<Bar>& bars,
                                    const BacktestConfig& config) {
    Portfolio portfolio(config.initial_capital);
    CommissionModel comm(config.commission_per_trade, config.slippage_bps);
    strategy.reset();

    Logger::log_info("Starting backtest with " + std::to_string(bars.size()) + " bars");

    double entry_price_for_risk = 0.0;

    for (size_t i = 0; i < bars.size(); ++i) {
        const Bar& bar = bars[i];

        // --- Risk management: check stop-loss / take-profit first ---
        if (portfolio.has_position() &&
            (config.stop_loss_pct > 0.0 || config.take_profit_pct > 0.0)) {
            const Position& pos = portfolio.current_position();
            double pnl_pct = 0.0;
            if (pos.side == Side::Buy) {
                pnl_pct = (bar.close - pos.avg_price) / pos.avg_price * 100.0;
            } else {
                pnl_pct = (pos.avg_price - bar.close) / pos.avg_price * 100.0;
            }

            bool exit_on_sl = config.stop_loss_pct > 0.0 && pnl_pct <= -config.stop_loss_pct;
            bool exit_on_tp = config.take_profit_pct > 0.0 && pnl_pct >= config.take_profit_pct;

            if (exit_on_sl || exit_on_tp) {
                Order exit_order;
                exit_order.side = (pos.side == Side::Buy) ? Side::Sell : Side::Buy;
                exit_order.type = OrderType::Market;
                exit_order.price = comm.apply_slippage(bar.close, exit_order.side);
                exit_order.quantity = pos.quantity;
                exit_order.status = OrderStatus::Filled;
                exit_order.fill_time = bar.time;
                double commission = comm.calculate_commission(exit_order.quantity);
                portfolio.close_position(exit_order, commission);
                strategy.reset(); // Tell strategy we're flat again
            }
        }

        // --- Strategy signals ---
        std::vector<Order> orders;
        strategy.on_bar(bar, orders);

        for (auto& order : orders) {
            double fill_price = comm.apply_slippage(bar.close, order.side);
            order.price = fill_price;
            order.status = OrderStatus::Filled;
            order.fill_time = bar.time;

            // Apply position sizing on entry.
            if (order.side == Side::Buy && !portfolio.has_position()) {
                order.quantity = compute_position_size(config, order.quantity,
                                                         fill_price, portfolio.equity());
                double commission = comm.calculate_commission(order.quantity);
                portfolio.open_position(order, commission);
                entry_price_for_risk = fill_price;
            } else if (order.side == Side::Sell && portfolio.has_position()) {
                double commission = comm.calculate_commission(order.quantity);
                portfolio.close_position(order, commission);
            }
        }

        portfolio.update_equity(bar.time, bar.close);
    }

    // Close any open position at final bar.
    if (portfolio.has_position() && !bars.empty()) {
        const Bar& last_bar = bars.back();
        Order close_order;
        close_order.side = Side::Sell;
        close_order.type = OrderType::Market;
        close_order.price = comm.apply_slippage(last_bar.close, Side::Sell);
        close_order.quantity = portfolio.current_position().quantity;
        close_order.status = OrderStatus::Filled;
        close_order.fill_time = last_bar.time;
        double commission = comm.calculate_commission(close_order.quantity);
        portfolio.close_position(close_order, commission);
    }

    BacktestResult result;
    result.trades = portfolio.trades();
    result.equity_curve = portfolio.equity_curve();
    result.final_equity = portfolio.equity();
    result.total_commission = portfolio.total_commission();
    result.total_bars = static_cast<int>(bars.size());

    Logger::log_info("Backtest complete: " + std::to_string(result.trades.size()) +
                    " trades, final equity: " + std::to_string(result.final_equity));

    (void)entry_price_for_risk; // Retained for future trailing-stop support.
    return result;
}

} // namespace qp
