#pragma once

#include "common/types.h"
#include <vector>

namespace qp {

struct MCConfig {
    int iterations = 1000;
    double slippage_perturbation_bps = 10.0;
    double initial_capital = 100000.0;
};

struct MCResult {
    double mean_final_equity = 0.0;
    double median_final_equity = 0.0;
    double percentile_5 = 0.0;
    double percentile_95 = 0.0;
    double worst_case_equity = 0.0;
    double survival_rate = 0.0;
    double ruin_probability = 0.0;
    std::vector<double> final_equities;
};

class MonteCarlo {
public:
    static MCResult run(const std::vector<Trade>& trades,
                        const MCConfig& config);
};

} // namespace qp
