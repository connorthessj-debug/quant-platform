#pragma once

#include "common/types.h"

namespace qp {

class CommissionModel {
public:
    CommissionModel(double per_trade_fee, double slippage_bps);

    double calculate_commission(double quantity) const;
    double apply_slippage(double price, Side side) const;

    double per_trade_fee() const { return per_trade_fee_; }
    double slippage_bps() const { return slippage_bps_; }

private:
    double per_trade_fee_;
    double slippage_bps_;
};

} // namespace qp
