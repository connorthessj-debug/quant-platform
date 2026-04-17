#pragma once

#include "strategy/strategy_base.h"

namespace qp {

// MACD strategy: buy when MACD line crosses above signal line, sell on opposite.
// MACD = EMA(fast) - EMA(slow); Signal = EMA(MACD, signal_period).
class MacdStrategy : public IStrategy {
public:
    MacdStrategy();

    std::string name() const override { return "MACD"; }
    void configure(const StrategyParams& params) override;
    void on_bar(const Bar& bar, std::vector<Order>& orders) override;
    void reset() override;
    std::unique_ptr<IStrategy> clone() const override;

private:
    int fast_period_ = 12;
    int slow_period_ = 26;
    int signal_period_ = 9;
    double position_size_ = 100.0;

    double ema_fast_ = 0.0;
    double ema_slow_ = 0.0;
    double ema_signal_ = 0.0;
    bool initialized_ = false;
    int bar_count_ = 0;
    double prev_macd_minus_signal_ = 0.0;
    bool has_prev_cross_ = false;
    bool has_position_ = false;
    int next_order_id_ = 1;

    static double ema_update(double prev, double price, int period);
};

} // namespace qp
