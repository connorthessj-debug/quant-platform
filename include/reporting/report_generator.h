#pragma once

#include "common/types.h"
#include "core/backtest_engine.h"
#include "metrics/backtest_snapshot.h"
#include "montecarlo/monte_carlo.h"
#include "walkforward/walk_forward.h"
#include "optimization/optimizer.h"
#include "truth/truth_engine.h"
#include <string>
#include <vector>

namespace qp {

class ReportGenerator {
public:
    // Write trade-by-trade log in CSV form.
    static bool write_trade_log(const std::vector<Trade>& trades,
                                 const std::string& path);

    // Write equity curve as CSV (timestamp, equity).
    static bool write_equity_curve(const EquityCurve& curve,
                                    const std::string& path);

    // Write a human-readable text report summarizing all analysis phases.
    static bool write_text_report(const BacktestResult& bt_result,
                                   const BacktestSnapshot& snapshot,
                                   const MCResult& mc_result,
                                   const OptResult& opt_result,
                                   const WFAResult& wfa_result,
                                   const TruthReport& truth_report,
                                   const std::string& strategy_name,
                                   const std::string& path);

    // Write a self-contained HTML report.
    static bool write_html_report(const BacktestResult& bt_result,
                                   const BacktestSnapshot& snapshot,
                                   const MCResult& mc_result,
                                   const OptResult& opt_result,
                                   const WFAResult& wfa_result,
                                   const TruthReport& truth_report,
                                   const std::string& strategy_name,
                                   const std::string& path);
};

} // namespace qp
