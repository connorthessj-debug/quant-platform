#include "execution/risk_manager.h"

namespace qp {

void RiskManager::configure(const ExecutionConfig& config) {
    max_position_size_ = config.max_position_size;
    max_order_value_ = config.max_order_value;
    max_daily_loss_ = config.max_daily_loss;
}

RiskCheckResult RiskManager::check_order(Side side, double price, double quantity,
                                           double current_equity, double daily_pnl) const {
    RiskCheckResult result;
    (void)side;

    if (quantity > max_position_size_) {
        result.approved = false;
        result.rejection_reason = "Position size " + std::to_string(quantity) +
                                  " exceeds max " + std::to_string(max_position_size_);
        return result;
    }

    double order_value = price * quantity;
    if (order_value > max_order_value_) {
        result.approved = false;
        result.rejection_reason = "Order value " + std::to_string(order_value) +
                                  " exceeds max " + std::to_string(max_order_value_);
        return result;
    }

    if (daily_pnl < -max_daily_loss_) {
        result.approved = false;
        result.rejection_reason = "Daily loss limit reached: " + std::to_string(daily_pnl);
        return result;
    }

    if (current_equity < order_value) {
        result.approved = false;
        result.rejection_reason = "Insufficient equity: " + std::to_string(current_equity) +
                                  " < " + std::to_string(order_value);
        return result;
    }

    result.approved = true;
    return result;
}

} // namespace qp
