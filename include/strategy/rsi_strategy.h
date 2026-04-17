#pragma once

#include "strategy/strategy_base.h"
#include <deque>

namespace qp {

// Relative Strength Index (RSI) mean-reversion strategy.
// Buys when RSI falls below oversold threshold; sells when RSI exceeds overbought.
class RsiStrategy : public IStrategy {
public:
    RsiStrategy();

    std::string name() const override { return "RSI"; }
    void configure(const StrategyParams& params) override;
    void on_bar(const Bar& bar, std::vector<Order>& orders) override;
    void reset() override;
    std::unique_ptr<IStrategy> clone() const override;

private:
    int period_ = 14;
    double oversold_ = 30.0;
    double overbought_ = 70.0;
    double position_size_ = 100.0;

    std::deque<double> gains_;
    std::deque<double> losses_;
    double prev_close_ = 0.0;
    bool has_prev_ = false;
    bool has_position_ = false;
    int next_order_id_ = 1;

    double compute_rsi() const;
};

} // namespace qp
