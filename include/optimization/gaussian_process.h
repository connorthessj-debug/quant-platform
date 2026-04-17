#pragma once

#include <vector>

namespace qp {

// Gaussian Process regression with RBF (Squared Exponential) kernel.
// Used as surrogate model in Bayesian optimization.
class GaussianProcess {
public:
    GaussianProcess(double length_scale = 0.5,
                    double signal_variance = 1.0,
                    double noise_variance = 1e-6);

    // Fit GP to training data. X is n_samples x n_features.
    void fit(const std::vector<std::vector<double>>& X,
             const std::vector<double>& y);

    struct Prediction {
        double mean;
        double variance;
    };

    // Posterior predictive at point x.
    Prediction predict(const std::vector<double>& x) const;

    // RBF kernel: k(a,b) = signal_variance * exp(-||a-b||^2 / (2*length_scale^2))
    double kernel(const std::vector<double>& a, const std::vector<double>& b) const;

    bool is_fitted() const { return fitted_; }

private:
    double length_scale_;
    double signal_variance_;
    double noise_variance_;
    std::vector<std::vector<double>> X_;
    std::vector<double> y_;
    double y_mean_ = 0.0;

    // Cholesky factor L of (K + noise*I), lower-triangular.
    std::vector<std::vector<double>> L_;
    // alpha = K^-1 * (y - y_mean), precomputed for O(n) prediction.
    std::vector<double> alpha_;
    bool fitted_ = false;

    // Cholesky decomposition: returns L such that A = L * L^T.
    // Returns true on success (A must be symmetric positive definite).
    static bool cholesky(const std::vector<std::vector<double>>& A,
                         std::vector<std::vector<double>>& L);

    // Solve L * x = b by forward substitution.
    static std::vector<double> solve_lower(const std::vector<std::vector<double>>& L,
                                            const std::vector<double>& b);

    // Solve L^T * x = b by backward substitution.
    static std::vector<double> solve_upper_transpose(const std::vector<std::vector<double>>& L,
                                                      const std::vector<double>& b);
};

} // namespace qp
