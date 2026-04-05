#pragma once

#include "common/types.h"
#include <string>
#include <vector>
#include <memory>

namespace qp {

class IStrategy {
public:
    virtual ~IStrategy() = default;
    virtual std::string name() const = 0;
    virtual void configure(const StrategyParams& params) = 0;
    virtual void on_bar(const Bar& bar, std::vector<Order>& orders) = 0;
    virtual void reset() = 0;
    virtual std::unique_ptr<IStrategy> clone() const = 0;
};

} // namespace qp
