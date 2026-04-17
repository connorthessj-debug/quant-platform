#include "strategy/macd_strategy.h"

namespace qp {

MacdStrategy::MacdStrategy() {}

void MacdStrategy::configure(const StrategyParams& params) {
    fast_period_ = static_cast<int>(params.get("fast_period", 12.0));
    slow_period_ = static_cast<int>(params.get("slow_period", 26.0));
    signal_period_ = static_cast<int>(params.get("signal_period", 9.0));
    position_size_ = params.get("position_size", 100.0);
}

double MacdStrategy::ema_update(double prev, double price, int period) {
    double alpha = 2.0 / (period + 1.0);
    return alpha * price + (1.0 - alpha) * prev;
}

void MacdStrategy::on_bar(const Bar& bar, std::vector<Order>& orders) {
    if (!initialized_) {
        ema_fast_ = bar.close;
        ema_slow_ = bar.close;
        ema_signal_ = 0.0;
        initialized_ = true;
    } else {
        ema_fast_ = ema_update(ema_fast_, bar.close, fast_period_);
        ema_slow_ = ema_update(ema_slow_, bar.close, slow_period_);
    }

    double macd = ema_fast_ - ema_slow_;

    // Seed signal EMA with first MACD value.
    if (bar_count_ == 0) {
        ema_signal_ = macd;
    } else {
        ema_signal_ = ema_update(ema_signal_, macd, signal_period_);
    }
    bar_count_++;

    // Need some warmup before trading.
    if (bar_count_ < slow_period_ + signal_period_) {
        prev_macd_minus_signal_ = macd - ema_signal_;
        has_prev_cross_ = true;
        return;
    }

    double cur_diff = macd - ema_signal_;

    if (has_prev_cross_) {
        bool cross_up = prev_macd_minus_signal_ <= 0.0 && cur_diff > 0.0;
        bool cross_down = prev_macd_minus_signal_ >= 0.0 && cur_diff < 0.0;

        if (cross_up && !has_position_) {
            Order o;
            o.id = next_order_id_++;
            o.side = Side::Buy;
            o.type = OrderType::Market;
            o.price = bar.close;
            o.quantity = position_size_;
            o.status = OrderStatus::Pending;
            o.created_time = bar.time;
            orders.push_back(o);
            has_position_ = true;
        } else if (cross_down && has_position_) {
            Order o;
            o.id = next_order_id_++;
            o.side = Side::Sell;
            o.type = OrderType::Market;
            o.price = bar.close;
            o.quantity = position_size_;
            o.status = OrderStatus::Pending;
            o.created_time = bar.time;
            orders.push_back(o);
            has_position_ = false;
        }
    }

    prev_macd_minus_signal_ = cur_diff;
    has_prev_cross_ = true;
}

void MacdStrategy::reset() {
    ema_fast_ = 0.0;
    ema_slow_ = 0.0;
    ema_signal_ = 0.0;
    initialized_ = false;
    bar_count_ = 0;
    prev_macd_minus_signal_ = 0.0;
    has_prev_cross_ = false;
    has_position_ = false;
    next_order_id_ = 1;
}

std::unique_ptr<IStrategy> MacdStrategy::clone() const {
    auto c = std::make_unique<MacdStrategy>();
    c->fast_period_ = fast_period_;
    c->slow_period_ = slow_period_;
    c->signal_period_ = signal_period_;
    c->position_size_ = position_size_;
    return c;
}

} // namespace qp
