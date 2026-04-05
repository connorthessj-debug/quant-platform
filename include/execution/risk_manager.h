#pragma once

#include "common/types.h"
#include "common/config.h"
#include <string>

namespace qp {

struct RiskCheckResult {
    bool approved = true;
    std::string rejection_reason;
};

class RiskManager {
public:
    void configure(const ExecutionConfig& config);

    RiskCheckResult check_order(Side side, double price, double quantity,
                                 double current_equity, double daily_pnl) const;

    double max_position_size() const { return max_position_size_; }
    double max_order_value() const { return max_order_value_; }
    double max_daily_loss() const { return max_daily_loss_; }

private:
    double max_position_size_ = 1000.0;
    double max_order_value_ = 100000.0;
    double max_daily_loss_ = 5000.0;
};

} // namespace qp
