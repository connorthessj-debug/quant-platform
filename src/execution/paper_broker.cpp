#include "execution/paper_broker.h"
#include "common/logging.h"

namespace qp {

PaperBroker::PaperBroker(double initial_capital, double commission_per_trade, double slippage_bps)
    : portfolio_(initial_capital)
    , commission_model_(commission_per_trade, slippage_bps) {
    order_manager_.set_fill_callback([this](const Order& order) {
        on_fill(order);
    });
}

bool PaperBroker::submit_order(Side side, OrderType type, double price, double quantity) {
    RiskCheckResult check = risk_manager_.check_order(side, price, quantity,
                                                        portfolio_.equity(), 0.0);
    if (!check.approved) {
        Logger::log_warning("Order rejected by risk manager: " + check.rejection_reason);
        return false;
    }

    double slipped_price = commission_model_.apply_slippage(price, side);
    order_manager_.submit_order(side, type, slipped_price, quantity);
    return true;
}

void PaperBroker::on_bar(const Bar& bar) {
    order_manager_.process_fills(bar.close, bar.time);
    portfolio_.update_equity(bar.time, bar.close);
}

void PaperBroker::on_fill(const Order& order) {
    double commission = commission_model_.calculate_commission(order.quantity);
    if (order.side == Side::Buy && !portfolio_.has_position()) {
        portfolio_.open_position(order, commission);
        Logger::log_info("Paper broker: opened position at " + std::to_string(order.price));
    } else if (order.side == Side::Sell && portfolio_.has_position()) {
        portfolio_.close_position(order, commission);
        Logger::log_info("Paper broker: closed position at " + std::to_string(order.price));
    }
}

void PaperBroker::reset() {
    portfolio_.reset(portfolio_.cash() + portfolio_.equity());
    order_manager_.reset();
}

} // namespace qp
