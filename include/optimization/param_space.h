#pragma once

#include "common/types.h"
#include <string>
#include <vector>

namespace qp {

struct ParamRange {
    std::string name;
    double min_val;
    double max_val;
    double step;
};

struct ParamSpace {
    std::vector<ParamRange> ranges;

    std::vector<StrategyParams> generate_combinations() const;
};

} // namespace qp
