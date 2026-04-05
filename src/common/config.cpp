#include "common/config.h"
#include "common/logging.h"
#include <fstream>
#include <sstream>

namespace qp {

nlohmann::json load_json_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        Logger::log_error("Failed to open config file: " + path);
        return nlohmann::json::object();
    }
    std::stringstream ss;
    ss << file.rdbuf();
    try {
        return nlohmann::json::parse(ss.str());
    } catch (const nlohmann::json::parse_error& e) {
        Logger::log_error("JSON parse error in " + path + ": " + e.what());
        return nlohmann::json::object();
    }
}

AppConfig AppConfig::load(const std::string& path) {
    AppConfig cfg;
    auto j = load_json_file(path);
    if (j.contains("initial_capital")) cfg.initial_capital = j["initial_capital"].get<double>();
    if (j.contains("commission_per_trade")) cfg.commission_per_trade = j["commission_per_trade"].get<double>();
    if (j.contains("slippage_bps")) cfg.slippage_bps = j["slippage_bps"].get<double>();
    if (j.contains("data_path")) cfg.data_path = j["data_path"].get<std::string>();
    if (j.contains("strategy_name")) cfg.strategy_name = j["strategy_name"].get<std::string>();
    return cfg;
}

OptimizerConfig OptimizerConfig::load(const std::string& path) {
    OptimizerConfig cfg;
    auto j = load_json_file(path);
    if (j.contains("max_iterations")) cfg.max_iterations = j["max_iterations"].get<int>();
    if (j.contains("objective")) cfg.objective = j["objective"].get<std::string>();
    if (j.contains("exhaustive")) cfg.exhaustive = j["exhaustive"].get<bool>();
    return cfg;
}

TruthEngineConfig TruthEngineConfig::load(const std::string& path) {
    TruthEngineConfig cfg;
    auto j = load_json_file(path);
    if (j.contains("min_sharpe")) cfg.min_sharpe = j["min_sharpe"].get<double>();
    if (j.contains("max_drawdown_pct")) cfg.max_drawdown_pct = j["max_drawdown_pct"].get<double>();
    if (j.contains("min_profit_factor")) cfg.min_profit_factor = j["min_profit_factor"].get<double>();
    if (j.contains("mc_survival_threshold")) cfg.mc_survival_threshold = j["mc_survival_threshold"].get<double>();
    if (j.contains("wfa_consistency_threshold")) cfg.wfa_consistency_threshold = j["wfa_consistency_threshold"].get<double>();
    if (j.contains("mc_iterations")) cfg.mc_iterations = j["mc_iterations"].get<int>();
    if (j.contains("slippage_perturbation_bps")) cfg.slippage_perturbation_bps = j["slippage_perturbation_bps"].get<double>();
    return cfg;
}

ExecutionConfig ExecutionConfig::load(const std::string& path) {
    ExecutionConfig cfg;
    auto j = load_json_file(path);
    if (j.contains("max_position_size")) cfg.max_position_size = j["max_position_size"].get<double>();
    if (j.contains("max_order_value")) cfg.max_order_value = j["max_order_value"].get<double>();
    if (j.contains("max_daily_loss")) cfg.max_daily_loss = j["max_daily_loss"].get<double>();
    if (j.contains("paper_mode")) cfg.paper_mode = j["paper_mode"].get<bool>();
    return cfg;
}

} // namespace qp
