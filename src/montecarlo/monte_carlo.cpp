#include "montecarlo/monte_carlo.h"
#include "common/logging.h"
#include <algorithm>
#include <random>
#include <cmath>
#include <numeric>

namespace qp {

MCResult MonteCarlo::run(const std::vector<Trade>& trades, const MCConfig& config) {
    MCResult result;
    if (trades.empty()) {
        Logger::log_warning("Monte Carlo: no trades to simulate");
        return result;
    }

    if (config.iterations <= 0) {
        Logger::log_warning("Monte Carlo: iterations must be positive");
        return result;
    }

    Logger::log_info("Running Monte Carlo with " + std::to_string(config.iterations) +
                    " iterations on " + std::to_string(trades.size()) + " trades");

    std::mt19937 rng(42);
    std::vector<double> pnls;
    pnls.reserve(trades.size());
    for (const auto& t : trades) {
        pnls.push_back(t.pnl);
    }

    double perturbation_factor = config.slippage_perturbation_bps / 10000.0;
    std::normal_distribution<double> noise(0.0, perturbation_factor);

    result.final_equities.reserve(config.iterations);
    int survival_count = 0;
    double ruin_threshold = config.initial_capital * 0.5;

    for (int iter = 0; iter < config.iterations; ++iter) {
        // Shuffle trade order
        std::vector<double> shuffled = pnls;
        std::shuffle(shuffled.begin(), shuffled.end(), rng);

        // Apply slippage perturbation
        double equity = config.initial_capital;
        bool ruined = false;
        for (double& p : shuffled) {
            double perturbed = p * (1.0 + noise(rng));
            equity += perturbed;
            if (equity < ruin_threshold) {
                ruined = true;
            }
        }

        result.final_equities.push_back(equity);
        if (!ruined) survival_count++;
    }

    std::sort(result.final_equities.begin(), result.final_equities.end());

    int n = static_cast<int>(result.final_equities.size());
    result.mean_final_equity = std::accumulate(result.final_equities.begin(),
                                                result.final_equities.end(), 0.0) / n;
    result.median_final_equity = result.final_equities[n / 2];
    result.percentile_5 = result.final_equities[static_cast<int>(n * 0.05)];
    result.percentile_95 = result.final_equities[static_cast<int>(n * 0.95)];
    result.worst_case_equity = result.final_equities[0];
    result.survival_rate = static_cast<double>(survival_count) / config.iterations;
    result.ruin_probability = 1.0 - result.survival_rate;

    Logger::log_info("Monte Carlo complete: survival rate = " +
                    std::to_string(result.survival_rate * 100.0) + "%");

    return result;
}

} // namespace qp
