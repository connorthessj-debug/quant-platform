#pragma once

#include "common/types.h"
#include "common/config.h"
#include "core/backtest_engine.h"
#include "metrics/backtest_snapshot.h"
#include "montecarlo/monte_carlo.h"
#include "optimization/optimizer.h"
#include "walkforward/walk_forward.h"
#include "truth/truth_engine.h"
#include "strategy/strategy_base.h"
#include <string>
#include <vector>
#include <memory>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace qp {

class GuiApp {
public:
    GuiApp();
    ~GuiApp();

    int run(int argc, char* argv[]);

private:
    AppConfig app_config_;
    OptimizerConfig opt_config_;
    TruthEngineConfig truth_config_;
    std::vector<Bar> bars_;
    std::unique_ptr<IStrategy> strategy_;
    BacktestResult backtest_result_;
    BacktestSnapshot snapshot_;
    MCResult mc_result_;
    OptResult opt_result_;
    WFAResult wfa_result_;
    TruthReport truth_report_;
    bool data_loaded_ = false;
    bool backtest_run_ = false;

    void load_data(const std::string& path);
    void run_backtest();
    void display_results();
    std::string format_snapshot(const BacktestSnapshot& snap) const;

#ifdef _WIN32
    struct WinData;
    std::unique_ptr<WinData> win_data_;
    static WinData* g_win_data_;
    static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg,
                                        WPARAM wparam, LPARAM lparam);
    void create_window();
    void append_output(const std::string& text);
#else
    void run_console(int argc, char* argv[]);
#endif
};

} // namespace qp
