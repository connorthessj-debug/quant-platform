cmake_minimum_required(VERSION 3.16)
project(QuantPlatform VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Platform defines
if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)
endif()

# --- Static Libraries ---

# Common
add_library(qp_common STATIC
    src/common/types.cpp
    src/common/config.cpp
    src/common/logging.cpp
    src/common/time_utils.cpp
)
target_include_directories(qp_common PUBLIC include)

# IO
add_library(qp_io STATIC
    src/io/csv_reader.cpp
    src/io/json_reader.cpp
)
target_link_libraries(qp_io PUBLIC qp_common)

# Core
add_library(qp_core STATIC
    src/core/portfolio.cpp
    src/core/backtest_engine.cpp
    src/core/commission_model.cpp
)
target_link_libraries(qp_core PUBLIC qp_common qp_io)

# Strategy
add_library(qp_strategy STATIC
    src/strategy/sma_crossover.cpp
    src/strategy/rsi_strategy.cpp
    src/strategy/macd_strategy.cpp
    src/strategy/signal.cpp
)
target_link_libraries(qp_strategy PUBLIC qp_common)

# Metrics
add_library(qp_metrics STATIC
    src/metrics/metrics_engine.cpp
)
target_link_libraries(qp_metrics PUBLIC qp_common)

# Monte Carlo
add_library(qp_montecarlo STATIC
    src/montecarlo/monte_carlo.cpp
)
target_link_libraries(qp_montecarlo PUBLIC qp_common qp_metrics)

# Walk-Forward
add_library(qp_walkforward STATIC
    src/walkforward/walk_forward.cpp
)
target_link_libraries(qp_walkforward PUBLIC qp_common qp_core qp_metrics qp_strategy)

# Optimization
add_library(qp_optimization STATIC
    src/optimization/optimizer.cpp
    src/optimization/gaussian_process.cpp
    src/optimization/bayesian_optimizer.cpp
)
target_link_libraries(qp_optimization PUBLIC qp_common qp_core qp_metrics qp_strategy)

# Truth Engine
add_library(qp_truth STATIC
    src/truth/truth_engine.cpp
)
target_link_libraries(qp_truth PUBLIC qp_common qp_core qp_metrics qp_montecarlo qp_walkforward qp_optimization)

# Execution
add_library(qp_execution STATIC
    src/execution/order_manager.cpp
    src/execution/paper_broker.cpp
    src/execution/risk_manager.cpp
)
target_link_libraries(qp_execution PUBLIC qp_common qp_core)

# GUI
add_library(qp_gui STATIC
    src/gui/gui_app.cpp
)
target_link_libraries(qp_gui PUBLIC qp_common qp_core qp_metrics qp_strategy qp_io
                                     qp_montecarlo qp_optimization qp_walkforward qp_truth
                                     qp_reporting)

# Review
add_library(qp_review STATIC
    src/review/review_hook.cpp
)
target_link_libraries(qp_review PUBLIC qp_common)

# Reporting
add_library(qp_reporting STATIC
    src/reporting/report_generator.cpp
)
target_link_libraries(qp_reporting PUBLIC qp_common qp_core qp_metrics qp_montecarlo qp_walkforward qp_optimization qp_truth)

# --- Main Executable ---

add_executable(QuantPlatform app/main.cpp)
target_link_libraries(QuantPlatform PRIVATE
    qp_gui
    qp_core
    qp_strategy
    qp_metrics
    qp_montecarlo
    qp_walkforward
    qp_optimization
    qp_truth
    qp_execution
    qp_review
    qp_reporting
    qp_io
    qp_common
)

# Windows-specific linking
if(WIN32)
    target_link_libraries(QuantPlatform PRIVATE user32 gdi32 comctl32)
    if(MSVC)
        set_target_properties(QuantPlatform PROPERTIES
            LINK_FLAGS "/ENTRY:mainCRTStartup /SUBSYSTEM:WINDOWS"
        )
    elseif(MINGW)
        target_link_options(QuantPlatform PRIVATE -mwindows)
    endif()
endif()

# --- Install Rules ---

install(TARGETS QuantPlatform RUNTIME DESTINATION .)
install(DIRECTORY config/ DESTINATION config)
install(DIRECTORY data/ DESTINATION data)
