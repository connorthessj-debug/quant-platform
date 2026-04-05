#pragma once

#include "common/types.h"
#include <vector>
#include <functional>

namespace qp {

class OrderManager {
public:
    using FillCallback = std::function<void(const Order&)>;

    void set_fill_callback(FillCallback cb) { fill_callback_ = std::move(cb); }

    int submit_order(Side side, OrderType type, double price, double quantity);
    bool cancel_order(int order_id);
    void process_fills(double current_price, Timestamp current_time);

    const std::vector<Order>& pending_orders() const { return pending_; }
    const std::vector<Order>& filled_orders() const { return filled_; }

    void reset();

private:
    std::vector<Order> pending_;
    std::vector<Order> filled_;
    FillCallback fill_callback_;
    int next_id_ = 1;

    bool should_fill(const Order& order, double current_price) const;
};

} // namespace qp
