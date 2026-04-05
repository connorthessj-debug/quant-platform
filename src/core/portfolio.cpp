#include "core/portfolio.h"

namespace qp {

Portfolio::Portfolio(double initial_capital)
    : initial_capital_(initial_capital)
    , cash_(initial_capital)
    , equity_(initial_capital) {}

void Portfolio::open_position(const Order& filled_order, double commission) {
    if (has_position_) return;

    position_.side = filled_order.side;
    position_.quantity = filled_order.quantity;
    position_.avg_price = filled_order.price;
    position_.unrealized_pnl = 0.0;
    has_position_ = true;

    cash_ -= filled_order.price * filled_order.quantity + commission;
    total_commission_ += commission;

    entry_order_ = filled_order;
    entry_commission_ = commission;
}

void Portfolio::close_position(const Order& filled_order, double commission) {
    if (!has_position_) return;

    double entry_price = position_.avg_price;
    double exit_price = filled_order.price;
    double quantity = position_.quantity;
    double pnl = 0.0;

    if (position_.side == Side::Buy) {
        pnl = (exit_price - entry_price) * quantity;
    } else {
        pnl = (entry_price - exit_price) * quantity;
    }

    double total_comm = commission + entry_commission_;
    pnl -= total_comm;

    Trade trade;
    trade.id = next_trade_id_++;
    trade.side = position_.side;
    trade.entry_price = entry_price;
    trade.exit_price = exit_price;
    trade.quantity = quantity;
    trade.pnl = pnl;
    trade.commission = total_comm;
    trade.entry_time = entry_order_.fill_time;
    trade.exit_time = filled_order.fill_time;
    trades_.push_back(trade);

    cash_ += exit_price * quantity - commission;
    total_commission_ += commission;
    has_position_ = false;
    position_ = Position{};
}

void Portfolio::update_equity(Timestamp time, double current_price) {
    double pos_value = 0.0;
    if (has_position_) {
        if (position_.side == Side::Buy) {
            pos_value = position_.quantity * current_price;
            position_.unrealized_pnl = (current_price - position_.avg_price) * position_.quantity;
        } else {
            pos_value = position_.quantity * (2.0 * position_.avg_price - current_price);
            position_.unrealized_pnl = (position_.avg_price - current_price) * position_.quantity;
        }
    }
    equity_ = cash_ + pos_value;
    equity_curve_.times.push_back(time);
    equity_curve_.equity.push_back(equity_);
}

void Portfolio::reset(double initial_capital) {
    initial_capital_ = initial_capital;
    cash_ = initial_capital;
    equity_ = initial_capital;
    total_commission_ = 0.0;
    has_position_ = false;
    position_ = Position{};
    next_trade_id_ = 1;
    entry_order_ = Order{};
    entry_commission_ = 0.0;
    equity_curve_ = EquityCurve{};
    trades_.clear();
}

} // namespace qp
