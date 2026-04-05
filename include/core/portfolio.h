#pragma once

#include "common/types.h"
#include <vector>

namespace qp {

class Portfolio {
public:
    explicit Portfolio(double initial_capital);

    void open_position(const Order& filled_order, double commission);
    void close_position(const Order& filled_order, double commission);
    void update_equity(Timestamp time, double current_price);

    bool has_position() const { return has_position_; }
    const Position& current_position() const { return position_; }
    double cash() const { return cash_; }
    double equity() const { return equity_; }
    const EquityCurve& equity_curve() const { return equity_curve_; }
    const std::vector<Trade>& trades() const { return trades_; }
    double total_commission() const { return total_commission_; }

    void reset(double initial_capital);

private:
    double initial_capital_;
    double cash_;
    double equity_;
    double total_commission_ = 0.0;
    bool has_position_ = false;
    Position position_;
    int next_trade_id_ = 1;
    Order entry_order_;
    double entry_commission_ = 0.0;
    EquityCurve equity_curve_;
    std::vector<Trade> trades_;
};

} // namespace qp
