#include "core/backtest_engine.h"
#include "common/logging.h"

namespace qp {

BacktestResult BacktestEngine::run(IStrategy& strategy,
                                    const std::vector<Bar>& bars,
                                    const BacktestConfig& config) {
    Portfolio portfolio(config.initial_capital);
    CommissionModel comm(config.commission_per_trade, config.slippage_bps);
    strategy.reset();

    Logger::log_info("Starting backtest with " + std::to_string(bars.size()) + " bars");

    for (size_t i = 0; i < bars.size(); ++i) {
        const Bar& bar = bars[i];

        std::vector<Order> orders;
        strategy.on_bar(bar, orders);

        for (auto& order : orders) {
            double fill_price = comm.apply_slippage(bar.close, order.side);
            order.price = fill_price;
            order.status = OrderStatus::Filled;
            order.fill_time = bar.time;
            double commission = comm.calculate_commission(order.quantity);

            if (order.side == Side::Buy && !portfolio.has_position()) {
                portfolio.open_position(order, commission);
            } else if (order.side == Side::Sell && portfolio.has_position()) {
                portfolio.close_position(order, commission);
            }
        }

        portfolio.update_equity(bar.time, bar.close);
    }

    // Close any remaining open position at last bar's close
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

    return result;
}

} // namespace qp
