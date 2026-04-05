#pragma once

#include <string>
#include "third_party/json.hpp"

namespace qp {

struct AppConfig {
    double initial_capital = 100000.0;
    double commission_per_trade = 1.0;
    double slippage_bps = 5.0;
    std::string data_path = "data/sample/sample_ohlcv.csv";
    std::string strategy_name = "SMA_Crossover";

    static AppConfig load(const std::string& path);
};

struct OptimizerConfig {
    int max_iterations = 100;
    std::string objective = "sharpe";
    bool exhaustive = true;

    static OptimizerConfig load(const std::string& path);
};

struct TruthEngineConfig {
    double min_sharpe = 0.5;
    double max_drawdown_pct = 25.0;
    double min_profit_factor = 1.2;
    double mc_survival_threshold = 0.7;
    double wfa_consistency_threshold = 0.6;
    int mc_iterations = 1000;
    double slippage_perturbation_bps = 10.0;

    static TruthEngineConfig load(const std::string& path);
};

struct ExecutionConfig {
    double max_position_size = 1000.0;
    double max_order_value = 100000.0;
    double max_daily_loss = 5000.0;
    bool paper_mode = true;

    static ExecutionConfig load(const std::string& path);
};

nlohmann::json load_json_file(const std::string& path);

} // namespace qp
