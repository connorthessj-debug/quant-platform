#include "optimization/bayesian_optimizer.h"
#include "optimization/gaussian_process.h"
#include "metrics/metrics_engine.h"
#include "common/logging.h"
#include <random>
#include <cmath>
#include <limits>
#include <algorithm>

namespace qp {

namespace {

constexpr double kPi = 3.14159265358979323846;

// Standard normal PDF and CDF for Expected Improvement.
double norm_pdf(double x) {
    return std::exp(-0.5 * x * x) / std::sqrt(2.0 * kPi);
}

double norm_cdf(double x) {
    return 0.5 * (1.0 + std::erf(x / std::sqrt(2.0)));
}

// Expected Improvement acquisition function (maximization).
// EI(x) = (mu - f_best - xi) * Phi(z) + sigma * phi(z), where z = (mu - f_best - xi) / sigma.
double expected_improvement(double mean, double variance, double f_best, double xi) {
    if (variance <= 1e-12) return 0.0;
    double sigma = std::sqrt(variance);
    double improvement = mean - f_best - xi;
    double z = improvement / sigma;
    return improvement * norm_cdf(z) + sigma * norm_pdf(z);
}

// Map normalized value in [0,1] to the actual parameter value, snapped to step.
double denormalize(const ParamRange& r, double u) {
    double v = r.min_val + u * (r.max_val - r.min_val);
    if (r.step > 0.0) {
        double steps = std::round((v - r.min_val) / r.step);
        v = r.min_val + steps * r.step;
    }
    if (v < r.min_val) v = r.min_val;
    if (v > r.max_val) v = r.max_val;
    return v;
}

double normalize(const ParamRange& r, double v) {
    if (r.max_val <= r.min_val) return 0.0;
    return (v - r.min_val) / (r.max_val - r.min_val);
}

StrategyParams make_params(const ParamSpace& space, const std::vector<double>& u) {
    StrategyParams p;
    for (size_t i = 0; i < space.ranges.size(); ++i) {
        p.set(space.ranges[i].name, denormalize(space.ranges[i], u[i]));
    }
    return p;
}

} // namespace

OptResult BayesianOptimizer::run(IStrategy& strategy,
                                   const std::vector<Bar>& bars,
                                   const ParamSpace& param_space,
                                   const BacktestConfig& bt_config,
                                   const std::string& objective,
                                   const BayesianOptConfig& bo_config) {
    OptResult result;
    size_t dim = param_space.ranges.size();
    if (dim == 0) {
        Logger::log_warning("Bayesian optimizer: empty parameter space");
        return result;
    }

    Logger::log_info("Bayesian optimization: n_initial=" + std::to_string(bo_config.n_initial) +
                    " n_iterations=" + std::to_string(bo_config.n_iterations) +
                    " dim=" + std::to_string(dim));

    std::mt19937 rng(bo_config.seed);
    std::uniform_real_distribution<double> uniform(0.0, 1.0);

    BacktestEngine engine;
    std::vector<std::vector<double>> X_norm; // Normalized evaluated points
    std::vector<double> y;                   // Objective values
    double best_obj = -std::numeric_limits<double>::infinity();

    auto evaluate = [&](const std::vector<double>& u) -> double {
        StrategyParams params = make_params(param_space, u);
        auto clone = strategy.clone();
        clone->configure(params);
        BacktestResult bt = engine.run(*clone, bars, bt_config);
        BacktestSnapshot snap = MetricsEngine::compute(bt.trades, bt.equity_curve,
                                                         bt_config.initial_capital);
        double obj = Optimizer::evaluate_objective(snap, objective);

        // Sanitize NaN/Inf — GP cannot handle them.
        if (!std::isfinite(obj)) obj = -1e9;

        OptTrialResult trial;
        trial.params = params;
        trial.metrics = snap;
        trial.objective_value = obj;
        result.all_trials.push_back(trial);

        if (obj > best_obj) {
            best_obj = obj;
            result.best_params = params;
            result.best_metrics = snap;
            result.best_objective = obj;
        }

        X_norm.push_back(u);
        y.push_back(obj);
        return obj;
    };

    // Phase 1: Initial random sampling (Latin hypercube-style jitter would be nicer;
    // uniform random is good enough for low dimensions).
    for (int i = 0; i < bo_config.n_initial; ++i) {
        std::vector<double> u(dim);
        for (size_t d = 0; d < dim; ++d) u[d] = uniform(rng);
        evaluate(u);
    }

    // Phase 2: Bayesian iterations.
    for (int iter = 0; iter < bo_config.n_iterations; ++iter) {
        GaussianProcess gp(bo_config.length_scale,
                           bo_config.signal_variance,
                           bo_config.noise_variance);
        gp.fit(X_norm, y);

        if (!gp.is_fitted()) {
            Logger::log_warning("GP fit failed at iteration " + std::to_string(iter) +
                              " — falling back to random sample");
            std::vector<double> u(dim);
            for (size_t d = 0; d < dim; ++d) u[d] = uniform(rng);
            evaluate(u);
            continue;
        }

        // Maximize EI by random candidate sampling.
        std::vector<double> best_u(dim);
        double best_ei = -1.0;
        for (int c = 0; c < bo_config.n_candidates; ++c) {
            std::vector<double> u(dim);
            for (size_t d = 0; d < dim; ++d) u[d] = uniform(rng);
            auto pred = gp.predict(u);
            double ei = expected_improvement(pred.mean, pred.variance, best_obj, bo_config.xi);
            if (ei > best_ei) {
                best_ei = ei;
                best_u = u;
            }
        }

        // If EI is essentially zero everywhere, sample randomly to explore.
        if (best_ei < 1e-12) {
            for (size_t d = 0; d < dim; ++d) best_u[d] = uniform(rng);
        }

        evaluate(best_u);
    }

    result.total_evaluations = static_cast<int>(result.all_trials.size());

    Logger::log_info("Bayesian optimization complete: best " + objective + " = " +
                    std::to_string(best_obj) + " after " +
                    std::to_string(result.total_evaluations) + " evaluations");

    return result;
}

} // namespace qp
