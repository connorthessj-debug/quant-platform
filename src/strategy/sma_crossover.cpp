#include "strategy/sma_crossover.h"
#include <numeric>

namespace qp {

SmaCrossover::SmaCrossover() {}

void SmaCrossover::configure(const StrategyParams& params) {
    fast_period_ = static_cast<int>(params.get("fast_period", 10.0));
    slow_period_ = static_cast<int>(params.get("slow_period", 30.0));
    position_size_ = params.get("position_size", 100.0);
}

void SmaCrossover::on_bar(const Bar& bar, std::vector<Order>& orders) {
    fast_window_.push_back(bar.close);
    slow_window_.push_back(bar.close);

    if (static_cast<int>(fast_window_.size()) > fast_period_)
        fast_window_.pop_front();
    if (static_cast<int>(slow_window_.size()) > slow_period_)
        slow_window_.pop_front();

    if (static_cast<int>(slow_window_.size()) < slow_period_)
        return;

    double fast_sma = sma(fast_window_);
    double slow_sma = sma(slow_window_);

    // Check for crossover
    if (fast_sma > slow_sma && !has_position_) {
        Order order;
        order.id = next_order_id_++;
        order.side = Side::Buy;
        order.type = OrderType::Market;
        order.price = bar.close;
        order.quantity = position_size_;
        order.status = OrderStatus::Pending;
        order.created_time = bar.time;
        orders.push_back(order);
        has_position_ = true;
    } else if (fast_sma < slow_sma && has_position_) {
        Order order;
        order.id = next_order_id_++;
        order.side = Side::Sell;
        order.type = OrderType::Market;
        order.price = bar.close;
        order.quantity = position_size_;
        order.status = OrderStatus::Pending;
        order.created_time = bar.time;
        orders.push_back(order);
        has_position_ = false;
    }
}

void SmaCrossover::reset() {
    fast_window_.clear();
    slow_window_.clear();
    has_position_ = false;
    next_order_id_ = 1;
}

std::unique_ptr<IStrategy> SmaCrossover::clone() const {
    auto copy = std::make_unique<SmaCrossover>();
    copy->fast_period_ = fast_period_;
    copy->slow_period_ = slow_period_;
    copy->position_size_ = position_size_;
    return copy;
}

double SmaCrossover::sma(const std::deque<double>& window) const {
    if (window.empty()) return 0.0;
    double sum = std::accumulate(window.begin(), window.end(), 0.0);
    return sum / static_cast<double>(window.size());
}

} // namespace qp
