#include "strategy/rsi_strategy.h"
#include <numeric>

namespace qp {

RsiStrategy::RsiStrategy() {}

void RsiStrategy::configure(const StrategyParams& params) {
    period_ = static_cast<int>(params.get("period", 14.0));
    oversold_ = params.get("oversold", 30.0);
    overbought_ = params.get("overbought", 70.0);
    position_size_ = params.get("position_size", 100.0);
}

double RsiStrategy::compute_rsi() const {
    if (gains_.empty() || losses_.empty()) return 50.0;
    double avg_gain = std::accumulate(gains_.begin(), gains_.end(), 0.0) / gains_.size();
    double avg_loss = std::accumulate(losses_.begin(), losses_.end(), 0.0) / losses_.size();
    if (avg_loss < 1e-12) return 100.0;
    double rs = avg_gain / avg_loss;
    return 100.0 - (100.0 / (1.0 + rs));
}

void RsiStrategy::on_bar(const Bar& bar, std::vector<Order>& orders) {
    if (!has_prev_) {
        prev_close_ = bar.close;
        has_prev_ = true;
        return;
    }

    double change = bar.close - prev_close_;
    double gain = change > 0.0 ? change : 0.0;
    double loss = change < 0.0 ? -change : 0.0;

    gains_.push_back(gain);
    losses_.push_back(loss);
    if (static_cast<int>(gains_.size()) > period_) gains_.pop_front();
    if (static_cast<int>(losses_.size()) > period_) losses_.pop_front();

    prev_close_ = bar.close;

    if (static_cast<int>(gains_.size()) < period_) return;

    double rsi = compute_rsi();

    if (rsi < oversold_ && !has_position_) {
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
    } else if (rsi > overbought_ && has_position_) {
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

void RsiStrategy::reset() {
    gains_.clear();
    losses_.clear();
    prev_close_ = 0.0;
    has_prev_ = false;
    has_position_ = false;
    next_order_id_ = 1;
}

std::unique_ptr<IStrategy> RsiStrategy::clone() const {
    auto c = std::make_unique<RsiStrategy>();
    c->period_ = period_;
    c->oversold_ = oversold_;
    c->overbought_ = overbought_;
    c->position_size_ = position_size_;
    return c;
}

} // namespace qp
