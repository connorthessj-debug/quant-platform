#pragma once

#include "common/types.h"
#include "common/config.h"
#include "core/backtest_engine.h"
#include "metrics/backtest_snapshot.h"
#include "strategy/strategy_base.h"
#include <string>
#include <vector>
#include <memory>

namespace qp {

class GuiApp {
public:
    GuiApp();
    ~GuiApp();

    int run(int argc, char* argv[]);

private:
    AppConfig app_config_;
    std::vector<Bar> bars_;
    std::unique_ptr<IStrategy> strategy_;
    BacktestResult backtest_result_;
    BacktestSnapshot snapshot_;
    bool data_loaded_ = false;
    bool backtest_run_ = false;

    void load_data(const std::string& path);
    void run_backtest();
    void display_results();
    std::string format_snapshot(const BacktestSnapshot& snap) const;

#ifdef _WIN32
    struct WinData;
    std::unique_ptr<WinData> win_data_;
    static long long __stdcall window_proc(void* hwnd, unsigned int msg,
                                            unsigned long long wparam,
                                            long long lparam);
    void create_window();
    void append_output(const std::string& text);
#else
    void run_console(int argc, char* argv[]);
#endif
};

} // namespace qp
