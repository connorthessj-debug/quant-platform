#include "gui/gui_app.h"
#include "common/logging.h"
#include "io/csv_reader.h"
#include "strategy/sma_crossover.h"
#include "metrics/metrics_engine.h"
#include "montecarlo/monte_carlo.h"
#include "optimization/optimizer.h"
#include "optimization/param_space.h"
#include "walkforward/walk_forward.h"
#include "truth/truth_engine.h"
#include <sstream>
#include <iomanip>
#include <iostream>

#ifdef _WIN32

namespace qp {

static const int ID_BTN_LOAD = 101;
static const int ID_BTN_RUN = 102;
static const int ID_OUTPUT = 103;
static const int ID_STATUS = 104;

struct GuiApp::WinData {
    HWND hwnd_main = nullptr;
    HWND hwnd_output = nullptr;
    HWND hwnd_status = nullptr;
    HWND hwnd_btn_load = nullptr;
    HWND hwnd_btn_run = nullptr;
    GuiApp* app = nullptr;
};

GuiApp::WinData* GuiApp::g_win_data_ = nullptr;

GuiApp::GuiApp() : win_data_(std::make_unique<WinData>()) {
    win_data_->app = this;
    g_win_data_ = win_data_.get();
}

GuiApp::~GuiApp() {
    g_win_data_ = nullptr;
}

LRESULT CALLBACK GuiApp::window_proc(HWND hwnd, UINT msg,
                                      WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_COMMAND:
            if (g_win_data_ && g_win_data_->app) {
                if (LOWORD(wparam) == ID_BTN_LOAD) {
                    g_win_data_->app->load_data(g_win_data_->app->app_config_.data_path);
                } else if (LOWORD(wparam) == ID_BTN_RUN) {
                    g_win_data_->app->run_backtest();
                    g_win_data_->app->display_results();
                }
            }
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wparam, lparam);
}

void GuiApp::create_window() {
    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = window_proc;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.lpszClassName = "QuantPlatformClass";
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursorA(nullptr, IDC_ARROW);
    RegisterClassExA(&wc);

    win_data_->hwnd_main = CreateWindowExA(
        0, "QuantPlatformClass", "Quant Platform",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        nullptr, nullptr, GetModuleHandleA(nullptr), nullptr);

    HWND parent = win_data_->hwnd_main;

    win_data_->hwnd_btn_load = CreateWindowExA(
        0, "BUTTON", "Load Data",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 10, 120, 30, parent,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_BTN_LOAD)),
        GetModuleHandleA(nullptr), nullptr);

    win_data_->hwnd_btn_run = CreateWindowExA(
        0, "BUTTON", "Run Backtest",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        140, 10, 120, 30, parent,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_BTN_RUN)),
        GetModuleHandleA(nullptr), nullptr);

    win_data_->hwnd_output = CreateWindowExA(
        WS_EX_CLIENTEDGE, "EDIT", "",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
        10, 50, 760, 470, parent,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_OUTPUT)),
        GetModuleHandleA(nullptr), nullptr);

    win_data_->hwnd_status = CreateWindowExA(
        0, "STATIC", "Ready",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        10, 530, 760, 20, parent,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_STATUS)),
        GetModuleHandleA(nullptr), nullptr);
}

void GuiApp::append_output(const std::string& text) {
    if (!win_data_->hwnd_output) return;
    int len = GetWindowTextLengthA(win_data_->hwnd_output);
    SendMessageA(win_data_->hwnd_output, EM_SETSEL, len, len);
    std::string with_newline = text + "\r\n";
    SendMessageA(win_data_->hwnd_output, EM_REPLACESEL, FALSE,
                 reinterpret_cast<LPARAM>(with_newline.c_str()));
}

int GuiApp::run(int argc, char* argv[]) {
    (void)argc; (void)argv;

    app_config_ = AppConfig::load("config/app_config.json");
    opt_config_ = OptimizerConfig::load("config/optimizer_config.json");
    truth_config_ = TruthEngineConfig::load("config/truth_engine_config.json");

    if (app_config_.strategy_name == "SMA_Crossover") {
        strategy_ = std::make_unique<SmaCrossover>();
    } else {
        strategy_ = std::make_unique<SmaCrossover>();
    }

    create_window();
    append_output("Quant Platform v1.0");
    append_output("Click 'Load Data' to load market data.");
    append_output("Click 'Run Backtest' to execute strategy.");

    SetWindowTextA(win_data_->hwnd_status, "Ready - config loaded");

    MSG msg;
    while (GetMessageA(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return 0;
}

} // namespace qp

#else
// Non-Windows: console fallback

namespace qp {

GuiApp::GuiApp() {}
GuiApp::~GuiApp() {}

int GuiApp::run(int argc, char* argv[]) {
    run_console(argc, argv);
    return 0;
}

void GuiApp::run_console(int argc, char* argv[]) {
    (void)argc; (void)argv;

    std::cout << "=== Quant Platform v1.0 (Console Mode) ===" << std::endl;

    app_config_ = AppConfig::load("config/app_config.json");
    opt_config_ = OptimizerConfig::load("config/optimizer_config.json");
    truth_config_ = TruthEngineConfig::load("config/truth_engine_config.json");

    if (app_config_.strategy_name == "SMA_Crossover") {
        strategy_ = std::make_unique<SmaCrossover>();
    } else {
        strategy_ = std::make_unique<SmaCrossover>();
    }

    load_data(app_config_.data_path);
    if (!data_loaded_) {
        std::cerr << "Failed to load data. Exiting." << std::endl;
        return;
    }

    run_backtest();
    display_results();
}

} // namespace qp

#endif

// Shared implementation (both platforms)
namespace qp {

void GuiApp::load_data(const std::string& path) {
    bars_ = CsvReader::load_bars(path);
    data_loaded_ = !bars_.empty();

    std::string msg = data_loaded_
        ? "Loaded " + std::to_string(bars_.size()) + " bars from " + path
        : "Failed to load data from " + path;

#ifdef _WIN32
    append_output(msg);
    SetWindowTextA(win_data_->hwnd_status, msg.c_str());
#else
    std::cout << msg << std::endl;
#endif
}

void GuiApp::run_backtest() {
    if (!data_loaded_ || !strategy_) {
#ifdef _WIN32
        append_output("Error: load data first");
#else
        std::cerr << "Error: load data first" << std::endl;
#endif
        return;
    }

    BacktestConfig bt_config;
    bt_config.initial_capital = app_config_.initial_capital;
    bt_config.commission_per_trade = app_config_.commission_per_trade;
    bt_config.slippage_bps = app_config_.slippage_bps;

    StrategyParams params;
    params.set("fast_period", 10.0);
    params.set("slow_period", 30.0);
    params.set("position_size", 100.0);
    strategy_->configure(params);

    BacktestEngine engine;
    backtest_result_ = engine.run(*strategy_, bars_, bt_config);
    snapshot_ = MetricsEngine::compute(backtest_result_.trades,
                                        backtest_result_.equity_curve,
                                        app_config_.initial_capital);
    // --- Monte Carlo ---
    MCConfig mc_config;
    mc_config.iterations = truth_config_.mc_iterations;
    mc_config.slippage_perturbation_bps = truth_config_.slippage_perturbation_bps;
    mc_config.initial_capital = app_config_.initial_capital;
    mc_result_ = MonteCarlo::run(backtest_result_.trades, mc_config);

    // --- Optimizer ---
    ParamSpace param_space;
    param_space.ranges.push_back({"fast_period", 5.0, 20.0, 1.0});
    param_space.ranges.push_back({"slow_period", 20.0, 50.0, 5.0});
    auto opt_clone = strategy_->clone();
    opt_result_ = Optimizer::run(*opt_clone, bars_, param_space, bt_config,
                                  opt_config_.objective, opt_config_.max_iterations);

    // --- Walk-Forward Analysis ---
    WFAConfig wfa_config;
    auto wfa_clone = strategy_->clone();
    wfa_result_ = WalkForward::run(*wfa_clone, bars_, param_space, bt_config,
                                    wfa_config,
                                    [](IStrategy& s, const std::vector<Bar>& b,
                                       const ParamSpace& ps, const BacktestConfig& bc,
                                       const std::string& obj) {
                                        return Optimizer::find_best(s, b, ps, bc, obj);
                                    });

    // --- Truth Engine ---
    TruthEngine truth_engine;
    truth_report_ = truth_engine.validate(*strategy_, bars_, bt_config,
                                           backtest_result_, snapshot_,
                                           mc_result_, wfa_result_, opt_result_,
                                           truth_config_);

    backtest_run_ = true;

#ifdef _WIN32
    SetWindowTextA(win_data_->hwnd_status, "Analysis complete");
#endif
}

void GuiApp::display_results() {
    if (!backtest_run_) return;

    std::string output = format_snapshot(snapshot_);

    // Monte Carlo summary
    std::ostringstream mc_ss;
    mc_ss << std::fixed << std::setprecision(2);
    mc_ss << "\r\n--- Monte Carlo Results ---\r\n";
    mc_ss << "Mean Final Equity: $" << mc_result_.mean_final_equity << "\r\n";
    mc_ss << "Survival Rate: " << (mc_result_.survival_rate * 100.0) << "%\r\n";
    mc_ss << "Ruin Probability: " << (mc_result_.ruin_probability * 100.0) << "%\r\n";
    mc_ss << "5th Percentile: $" << mc_result_.percentile_5 << "\r\n";
    mc_ss << "95th Percentile: $" << mc_result_.percentile_95 << "\r\n";
    output += mc_ss.str();

    // Optimizer summary
    std::ostringstream opt_ss;
    opt_ss << std::fixed << std::setprecision(4);
    opt_ss << "\r\n--- Optimizer Results ---\r\n";
    opt_ss << "Evaluations: " << opt_result_.total_evaluations << "\r\n";
    opt_ss << "Best Objective: " << opt_result_.best_objective << "\r\n";
    for (const auto& kv : opt_result_.best_params.values) {
        opt_ss << "  " << kv.first << " = " << kv.second << "\r\n";
    }
    output += opt_ss.str();

    // Walk-Forward summary
    std::ostringstream wfa_ss;
    wfa_ss << std::fixed << std::setprecision(4);
    wfa_ss << "\r\n--- Walk-Forward Analysis ---\r\n";
    wfa_ss << "Consistency Score: " << wfa_result_.consistency_score << "\r\n";
    wfa_ss << "Avg OOS Sharpe: " << wfa_result_.avg_oos_sharpe << "\r\n";
    wfa_ss << "Consistent: " << (wfa_result_.is_consistent ? "YES" : "NO") << "\r\n";
    output += wfa_ss.str();

    // Truth Engine report
    std::ostringstream truth_ss;
    truth_ss << std::fixed << std::setprecision(2);
    truth_ss << "\r\n--- Truth Engine Report ---\r\n";
    truth_ss << "Score: " << truth_report_.score << "\r\n";
    truth_ss << "Passed: " << (truth_report_.passed ? "YES" : "NO") << "\r\n";
    for (const auto& flag : truth_report_.flags) {
        truth_ss << "  Flag: " << flag << "\r\n";
    }
    output += truth_ss.str();

#ifdef _WIN32
    append_output(output);
#else
    std::cout << output << std::endl;
#endif
}

std::string GuiApp::format_snapshot(const BacktestSnapshot& snap) const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4);
    ss << "--- Backtest Results ---\r\n";
    ss << "Total Trades: " << snap.total_trades << "\r\n";
    ss << "Winning: " << snap.winning_trades << "  Losing: " << snap.losing_trades << "\r\n";
    ss << "Win Rate: " << std::setprecision(2) << (snap.win_rate * 100.0) << "%\r\n";
    ss << "Total PnL: $" << std::setprecision(2) << snap.total_pnl << "\r\n";
    ss << "Sharpe Ratio: " << std::setprecision(4) << snap.sharpe_ratio << "\r\n";
    ss << "Sortino Ratio: " << snap.sortino_ratio << "\r\n";
    ss << "Max Drawdown: $" << std::setprecision(2) << snap.max_drawdown << "\r\n";
    ss << "Max Drawdown %: " << snap.max_drawdown_pct << "%\r\n";
    ss << "Profit Factor: " << std::setprecision(4) << snap.profit_factor << "\r\n";
    ss << "Expectancy: $" << std::setprecision(2) << snap.expectancy << "\r\n";
    ss << "Final Equity: $" << backtest_result_.final_equity << "\r\n";
    ss << "Total Commission: $" << backtest_result_.total_commission << "\r\n";
    return ss.str();
}

} // namespace qp
