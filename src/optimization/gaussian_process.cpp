#include "optimization/gaussian_process.h"
#include <cmath>
#include <numeric>
#include <stdexcept>

namespace qp {

GaussianProcess::GaussianProcess(double length_scale,
                                  double signal_variance,
                                  double noise_variance)
    : length_scale_(length_scale)
    , signal_variance_(signal_variance)
    , noise_variance_(noise_variance) {}

double GaussianProcess::kernel(const std::vector<double>& a,
                                const std::vector<double>& b) const {
    double sq_dist = 0.0;
    size_t n = a.size() < b.size() ? a.size() : b.size();
    for (size_t i = 0; i < n; ++i) {
        double d = a[i] - b[i];
        sq_dist += d * d;
    }
    return signal_variance_ * std::exp(-sq_dist / (2.0 * length_scale_ * length_scale_));
}

bool GaussianProcess::cholesky(const std::vector<std::vector<double>>& A,
                                std::vector<std::vector<double>>& L) {
    size_t n = A.size();
    L.assign(n, std::vector<double>(n, 0.0));

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j <= i; ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < j; ++k) {
                sum += L[i][k] * L[j][k];
            }
            if (i == j) {
                double diag = A[i][i] - sum;
                if (diag <= 0.0) return false; // Not positive definite
                L[i][j] = std::sqrt(diag);
            } else {
                if (L[j][j] == 0.0) return false;
                L[i][j] = (A[i][j] - sum) / L[j][j];
            }
        }
    }
    return true;
}

std::vector<double> GaussianProcess::solve_lower(const std::vector<std::vector<double>>& L,
                                                   const std::vector<double>& b) {
    size_t n = L.size();
    std::vector<double> x(n, 0.0);
    for (size_t i = 0; i < n; ++i) {
        double sum = b[i];
        for (size_t j = 0; j < i; ++j) sum -= L[i][j] * x[j];
        x[i] = sum / L[i][i];
    }
    return x;
}

std::vector<double> GaussianProcess::solve_upper_transpose(const std::vector<std::vector<double>>& L,
                                                             const std::vector<double>& b) {
    size_t n = L.size();
    std::vector<double> x(n, 0.0);
    for (size_t ii = n; ii > 0; --ii) {
        size_t i = ii - 1;
        double sum = b[i];
        for (size_t j = i + 1; j < n; ++j) sum -= L[j][i] * x[j];
        x[i] = sum / L[i][i];
    }
    return x;
}

void GaussianProcess::fit(const std::vector<std::vector<double>>& X,
                           const std::vector<double>& y) {
    fitted_ = false;
    X_ = X;
    y_ = y;
    size_t n = X.size();
    if (n == 0) return;

    // Center y (helps numerical stability of GP with non-zero mean).
    y_mean_ = std::accumulate(y_.begin(), y_.end(), 0.0) / static_cast<double>(n);
    std::vector<double> y_centered(n);
    for (size_t i = 0; i < n; ++i) y_centered[i] = y_[i] - y_mean_;

    // Build covariance matrix K + noise*I with jitter for stability.
    std::vector<std::vector<double>> K(n, std::vector<double>(n, 0.0));
    const double jitter = 1e-8;
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            K[i][j] = kernel(X[i], X[j]);
            if (i == j) K[i][j] += noise_variance_ + jitter;
        }
    }

    // Try Cholesky, increasing jitter if it fails.
    double extra_jitter = 0.0;
    for (int attempt = 0; attempt < 5; ++attempt) {
        if (extra_jitter > 0.0) {
            for (size_t i = 0; i < n; ++i) K[i][i] += extra_jitter;
        }
        if (cholesky(K, L_)) {
            fitted_ = true;
            break;
        }
        extra_jitter = (extra_jitter == 0.0) ? 1e-6 : extra_jitter * 10.0;
    }
    if (!fitted_) return;

    // alpha = K^-1 * y_centered, via L * L^T * alpha = y_centered.
    std::vector<double> z = solve_lower(L_, y_centered);
    alpha_ = solve_upper_transpose(L_, z);
}

GaussianProcess::Prediction GaussianProcess::predict(const std::vector<double>& x) const {
    Prediction p{y_mean_, signal_variance_};
    if (!fitted_ || X_.empty()) return p;

    size_t n = X_.size();
    std::vector<double> k_star(n);
    for (size_t i = 0; i < n; ++i) k_star[i] = kernel(X_[i], x);

    // Mean: y_mean + k_star^T * alpha
    double mean = y_mean_;
    for (size_t i = 0; i < n; ++i) mean += k_star[i] * alpha_[i];
    p.mean = mean;

    // Variance: k(x,x) - k_star^T * K^-1 * k_star
    // Solve L * v = k_star, then var = k(x,x) - v^T * v
    std::vector<double> v = solve_lower(L_, k_star);
    double v_sq = 0.0;
    for (double vi : v) v_sq += vi * vi;
    double var = kernel(x, x) - v_sq;
    if (var < 0.0) var = 0.0; // Numerical floor
    p.variance = var;

    return p;
}

} // namespace qp
