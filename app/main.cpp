#include "common/logging.h"
#include "common/config.h"
#include "io/csv_reader.h"
#include "strategy/strategy_base.h"
#include "strategy/sma_crossover.h"
#include "strategy/rsi_strategy.h"
#include "strategy/macd_strategy.h"
#include "core/backtest_engine.h"
#include "metrics/metrics_engine.h"
#include "montecarlo/monte_carlo.h"
#include "optimization/optimizer.h"
#include "optimization/param_space.h"
#include "optimization/bayesian_optimizer.h"
#include "walkforward/walk_forward.h"
#include "truth/truth_engine.h"
#include "reporting/report_generator.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <memory>

namespace {

std::unique_ptr<qp::IStrategy> create_strategy(const std::string& name) {
    if (name == "SMA_Crossover") return std::make_unique<qp::SmaCrossover>();
    if (name == "RSI") return std::make_unique<qp::RsiStrategy>();
    if (name == "MACD") return std::make_unique<qp::MacdStrategy>();
    qp::Logger::log_warning("Unknown strategy '" + name + "', using SMA_Crossover");
    return std::make_unique<qp::SmaCrossover>();
}

qp::ParamSpace default_param_space(const std::string& name) {
    qp::ParamSpace space;
    if (name == "RSI") {
        space.ranges.push_back({"period", 5.0, 30.0, 1.0});
        space.ranges.push_back({"oversold", 20.0, 40.0, 2.0});
        space.ranges.push_back({"overbought", 60.0, 80.0, 2.0});
    } else if (name == "MACD") {
        space.ranges.push_back({"fast_period", 5.0, 20.0, 1.0});
        space.ranges.push_back({"slow_period", 20.0, 40.0, 2.0});
        space.ranges.push_back({"signal_period", 5.0, 15.0, 1.0});
    } else {
        space.ranges.push_back({"fast_period", 5.0, 20.0, 1.0});
        space.ranges.push_back({"slow_period", 20.0, 50.0, 2.0});
    }
    return space;
}

qp::StrategyParams default_params(const std::string& name) {
    qp::StrategyParams p;
    if (name == "RSI") {
        p.set("period", 14.0);
        p.set("oversold", 30.0);
        p.set("overbought", 70.0);
    } else if (name == "MACD") {
        p.set("fast_period", 12.0);
        p.set("slow_period", 26.0);
        p.set("signal_period", 9.0);
    } else {
        p.set("fast_period", 10.0);
        p.set("slow_period", 30.0);
    }
    p.set("position_size", 100.0);
    return p;
}

std::string fmt(double v, int prec = 4) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(prec) << v;
    return ss.str();
}

} // namespace

int main(int argc, char* argv[]) {
    using namespace qp;

    std::cout << "================================================================" << std::endl;
    std::cout << "  Quant Platform v2.0 — Bayesian Backtesting Engine" << std::endl;
    std::cout << "================================================================" << std::endl;

    // --- Load configs ---
    AppConfig app_cfg = AppConfig::load("config/app_config.json");
    OptimizerConfig opt_cfg = OptimizerConfig::load("config/optimizer_config.json");
    TruthEngineConfig truth_cfg = TruthEngineConfig::load("config/truth_engine_config.json");

    // Allow command-line override: QuantPlatform [data_path] [strategy]
    if (argc >= 2) app_cfg.data_path = argv[1];
    if (argc >= 3) app_cfg.strategy_name = argv[2];

    std::cout << "Strategy:  " << app_cfg.strategy_name << std::endl;
    std::cout << "Data:      " << app_cfg.data_path << std::endl;
    std::cout << "Capital:   $" << fmt(app_cfg.initial_capital, 2) << std::endl;
    std::cout << std::endl;

    // --- Load data ---
    auto bars = CsvReader::load_bars(app_cfg.data_path);
    if (bars.empty()) {
        std::cerr << "ERROR: No data loaded from " << app_cfg.data_path << std::endl;
        return 1;
    }
    std::cout << "Loaded " << bars.size() << " bars." << std::endl;

    // --- Setup ---
    auto strategy = create_strategy(app_cfg.strategy_name);
    auto params = default_params(app_cfg.strategy_name);
    strategy->configure(params);

    BacktestConfig bt_cfg;
    bt_cfg.initial_capital = app_cfg.initial_capital;
    bt_cfg.commission_per_trade = app_cfg.commission_per_trade;
    bt_cfg.slippage_bps = app_cfg.slippage_bps;

    // --- 1. Backtest ---
    std::cout << "\n[1/6] Running backtest..." << std::endl;
    BacktestEngine engine;
    BacktestResult bt_result = engine.run(*strategy, bars, bt_cfg);
    BacktestSnapshot snap = MetricsEngine::compute(bt_result.trades, bt_result.equity_curve,
                                                     app_cfg.initial_capital);

    std::cout << "  Trades: " << snap.total_trades
              << "  PnL: $" << fmt(snap.total_pnl, 2)
              << "  Sharpe: " << fmt(snap.sharpe_ratio)
              << "  MaxDD: " << fmt(snap.max_drawdown_pct, 2) << "%" << std::endl;

    // --- 2. Monte Carlo ---
    std::cout << "\n[2/6] Running Monte Carlo (" << truth_cfg.mc_iterations << " iterations)..." << std::endl;
    MCConfig mc_cfg;
    mc_cfg.iterations = truth_cfg.mc_iterations;
    mc_cfg.slippage_perturbation_bps = truth_cfg.slippage_perturbation_bps;
    mc_cfg.initial_capital = app_cfg.initial_capital;
    MCResult mc_result = MonteCarlo::run(bt_result.trades, mc_cfg);

    std::cout << "  Survival: " << fmt(mc_result.survival_rate * 100.0, 2) << "%"
              << "  Mean equity: $" << fmt(mc_result.mean_final_equity, 2) << std::endl;

    // --- 3. Bayesian Optimization ---
    ParamSpace param_space = default_param_space(app_cfg.strategy_name);
    int n_bo = std::max(10, opt_cfg.max_iterations - 10);
    std::cout << "\n[3/6] Running Bayesian optimization (10 init + " << n_bo << " BO iterations)..." << std::endl;

    BayesianOptConfig bo_cfg;
    bo_cfg.n_initial = 10;
    bo_cfg.n_iterations = n_bo;
    auto opt_clone = strategy->clone();
    OptResult opt_result = BayesianOptimizer::run(*opt_clone, bars, param_space, bt_cfg,
                                                    opt_cfg.objective, bo_cfg);

    std::cout << "  Evaluations: " << opt_result.total_evaluations
              << "  Best " << opt_cfg.objective << ": " << fmt(opt_result.best_objective) << std::endl;
    std::cout << "  Best params:";
    for (const auto& kv : opt_result.best_params.values) {
        std::cout << " " << kv.first << "=" << fmt(kv.second, 1);
    }
    std::cout << std::endl;

    // --- 4. Walk-Forward Analysis ---
    std::cout << "\n[4/6] Running Walk-Forward Analysis (5 windows)..." << std::endl;
    WFAConfig wfa_cfg;
    auto wfa_clone = strategy->clone();
    WFAResult wfa_result = WalkForward::run(*wfa_clone, bars, param_space, bt_cfg,
                                              wfa_cfg,
                                              [](IStrategy& s, const std::vector<Bar>& b,
                                                 const ParamSpace& ps, const BacktestConfig& bc,
                                                 const std::string& obj) {
                                                  BayesianOptConfig bc2;
                                                  bc2.n_initial = 6;
                                                  bc2.n_iterations = 14;
                                                  return BayesianOptimizer::run(s, b, ps, bc, obj, bc2).best_params;
                                              });

    std::cout << "  Consistency: " << fmt(wfa_result.consistency_score)
              << "  Avg OOS Sharpe: " << fmt(wfa_result.avg_oos_sharpe)
              << "  " << (wfa_result.is_consistent ? "CONSISTENT" : "NOT CONSISTENT") << std::endl;

    // --- 5. Truth Engine ---
    std::cout << "\n[5/6] Running Truth Engine validation..." << std::endl;
    TruthEngine truth_engine;
    TruthReport truth_report = truth_engine.validate(*strategy, bars, bt_cfg,
                                                       bt_result, snap,
                                                       mc_result, wfa_result, opt_result,
                                                       truth_cfg);

    std::cout << "  Score: " << fmt(truth_report.score, 1) << "/100"
              << "  " << (truth_report.passed ? "PASSED" : "FAILED") << std::endl;
    for (const auto& flag : truth_report.flags) {
        std::cout << "  Flag: " << flag << std::endl;
    }

    // --- 6. Reports ---
    std::cout << "\n[6/6] Writing reports..." << std::endl;
    ReportGenerator::write_trade_log(bt_result.trades, "trade_log.csv");
    ReportGenerator::write_equity_curve(bt_result.equity_curve, "equity_curve.csv");
    ReportGenerator::write_text_report(bt_result, snap, mc_result, opt_result,
                                         wfa_result, truth_report,
                                         app_cfg.strategy_name, "report.txt");
    ReportGenerator::write_html_report(bt_result, snap, mc_result, opt_result,
                                         wfa_result, truth_report,
                                         app_cfg.strategy_name, "report.html");

    // --- Final Summary ---
    std::cout << "\n================================================================" << std::endl;
    std::cout << "  RESULTS SUMMARY" << std::endl;
    std::cout << "================================================================" << std::endl;
    std::cout << "Strategy:         " << app_cfg.strategy_name << std::endl;
    std::cout << "Trades:           " << snap.total_trades << " (" << snap.winning_trades << "W / " << snap.losing_trades << "L)" << std::endl;
    std::cout << "Total PnL:        $" << fmt(snap.total_pnl, 2) << std::endl;
    std::cout << "Total Return:     " << fmt(snap.total_return_pct, 2) << "%" << std::endl;
    std::cout << "CAGR:             " << fmt(snap.cagr * 100.0, 2) << "%" << std::endl;
    std::cout << "Sharpe:           " << fmt(snap.sharpe_ratio) << std::endl;
    std::cout << "Sortino:          " << fmt(snap.sortino_ratio) << std::endl;
    std::cout << "Calmar:           " << fmt(snap.calmar_ratio) << std::endl;
    std::cout << "Omega:            " << fmt(snap.omega_ratio) << std::endl;
    std::cout << "Max Drawdown:     " << fmt(snap.max_drawdown_pct, 2) << "%" << std::endl;
    std::cout << "Profit Factor:    " << fmt(snap.profit_factor) << std::endl;
    std::cout << "Win Rate:         " << fmt(snap.win_rate * 100.0, 2) << "%" << std::endl;
    std::cout << "Expectancy:       $" << fmt(snap.expectancy, 2) << std::endl;
    std::cout << "VaR 95%:          " << fmt(snap.value_at_risk_95 * 100.0, 4) << "%" << std::endl;
    std::cout << "Truth Score:      " << fmt(truth_report.score, 1) << "/100 — " << (truth_report.passed ? "PASSED" : "FAILED") << std::endl;
    std::cout << std::endl;
    std::cout << "Reports: trade_log.csv  equity_curve.csv  report.txt  report.html" << std::endl;
    std::cout << "================================================================" << std::endl;

    return 0;
}
