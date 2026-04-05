#pragma once

#include "strategy/strategy_base.h"
#include <deque>

namespace qp {

class SmaCrossover : public IStrategy {
public:
    SmaCrossover();

    std::string name() const override { return "SMA_Crossover"; }
    void configure(const StrategyParams& params) override;
    void on_bar(const Bar& bar, std::vector<Order>& orders) override;
    void reset() override;
    std::unique_ptr<IStrategy> clone() const override;

private:
    int fast_period_ = 10;
    int slow_period_ = 30;
    double position_size_ = 100.0;
    std::deque<double> fast_window_;
    std::deque<double> slow_window_;
    bool has_position_ = false;
    int next_order_id_ = 1;

    double sma(const std::deque<double>& window) const;
};

} // namespace qp
