#pragma once

#include "common/types.h"
#include "execution/order_manager.h"
#include "execution/risk_manager.h"
#include "core/portfolio.h"
#include "core/commission_model.h"
#include <vector>

namespace qp {

class PaperBroker {
public:
    PaperBroker(double initial_capital, double commission_per_trade, double slippage_bps);

    bool submit_order(Side side, OrderType type, double price, double quantity);
    void on_bar(const Bar& bar);

    const Portfolio& portfolio() const { return portfolio_; }
    const OrderManager& order_manager() const { return order_manager_; }
    const RiskManager& risk_manager() const { return risk_manager_; }

    void reset();

private:
    Portfolio portfolio_;
    CommissionModel commission_model_;
    OrderManager order_manager_;
    RiskManager risk_manager_;

    void on_fill(const Order& order);
};

} // namespace qp
