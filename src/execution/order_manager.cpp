#include "execution/order_manager.h"
#include <algorithm>

namespace qp {

int OrderManager::submit_order(Side side, OrderType type, double price, double quantity) {
    Order order;
    order.id = next_id_++;
    order.side = side;
    order.type = type;
    order.price = price;
    order.quantity = quantity;
    order.status = OrderStatus::Pending;
    order.created_time = 0;
    order.fill_time = 0;
    pending_.push_back(order);
    return order.id;
}

bool OrderManager::cancel_order(int order_id) {
    for (auto it = pending_.begin(); it != pending_.end(); ++it) {
        if (it->id == order_id) {
            it->status = OrderStatus::Cancelled;
            pending_.erase(it);
            return true;
        }
    }
    return false;
}

void OrderManager::process_fills(double current_price, Timestamp current_time) {
    auto it = pending_.begin();
    while (it != pending_.end()) {
        if (should_fill(*it, current_price)) {
            it->status = OrderStatus::Filled;
            it->fill_time = current_time;
            if (it->type == OrderType::Market) {
                it->price = current_price;
            }
            filled_.push_back(*it);
            if (fill_callback_) {
                fill_callback_(*it);
            }
            it = pending_.erase(it);
        } else {
            ++it;
        }
    }
}

void OrderManager::reset() {
    pending_.clear();
    filled_.clear();
    next_id_ = 1;
}

bool OrderManager::should_fill(const Order& order, double current_price) const {
    switch (order.type) {
        case OrderType::Market:
            return true;
        case OrderType::Limit:
            if (order.side == Side::Buy) return current_price <= order.price;
            else return current_price >= order.price;
        case OrderType::Stop:
            if (order.side == Side::Buy) return current_price >= order.price;
            else return current_price <= order.price;
    }
    return false;
}

} // namespace qp
