#include "core/commission_model.h"
#include <cmath>

namespace qp {

CommissionModel::CommissionModel(double per_trade_fee, double slippage_bps)
    : per_trade_fee_(per_trade_fee)
    , slippage_bps_(slippage_bps) {}

double CommissionModel::calculate_commission(double quantity) const {
    (void)quantity;
    return per_trade_fee_;
}

double CommissionModel::apply_slippage(double price, Side side) const {
    double slip = price * (slippage_bps_ / 10000.0);
    if (side == Side::Buy) {
        return price + slip;
    } else {
        return price - slip;
    }
}

} // namespace qp
