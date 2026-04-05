# Quant Platform

A Windows-native C++17 quantitative trading platform with backtesting, optimization, Monte Carlo analysis, walk-forward analysis, truth validation, and paper trading.

## Features

- **Backtesting Engine** — OHLCV ingestion, trade simulation, commission/slippage modeling, equity curve
- **Strategy Framework** — Pluggable strategies with parameterized configs (SMA Crossover included)
- **Metrics Engine** — Sharpe, Sortino, drawdown, profit factor, win rate, expectancy
- **Monte Carlo Engine** — Trade reshuffling, slippage perturbation, distribution analysis
- **Walk-Forward Analysis** — Rolling windows, in-sample/out-of-sample split, parameter tracking
- **Optimization Engine** — Exhaustive parameter search, objective-driven scoring
- **Truth Engine** — Lookahead bias detection, slippage fragility, parameter stability, WFA consistency, MC survival, regime dependence
- **Paper Trading** — Order manager, risk checks, broker abstraction
- **Desktop GUI** — Win32 native window (Windows) / console mode (Linux)

## Build Requirements

- CMake 3.16+
- C++17 compiler (MSVC 2019+ or MinGW-w64)
- Windows 10/11 (for GUI mode)

## Building

### MSVC (Visual Studio)

```bat
scripts\build_windows_msvc.bat
```

Or manually:

```bat
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### MinGW-w64

```bat
scripts\build_windows_mingw.bat
```

### Linux (console mode only)

```bash
mkdir build && cd build
cmake ..
cmake --build .
./QuantPlatform
```

## Running

1. Ensure `config/` and `data/` directories are alongside the executable
2. Run `QuantPlatform.exe`
3. Click **Load Data** to load OHLCV data
4. Click **Run Backtest** to execute the SMA Crossover strategy
5. Results display in the output panel

## Project Structure

```
include/          — Header files organized by subsystem
src/              — Implementation files
app/main.cpp      — Entry point
config/           — JSON configuration files
data/sample/      — Sample OHLCV data (500 daily bars)
scripts/          — Windows build scripts
installer/        — Inno Setup and NSIS installer scripts
```

## Configuration

Edit JSON files in `config/`:

- `app_config.json` — Capital, commissions, data path, strategy
- `optimizer_config.json` — Max iterations, objective function
- `truth_engine_config.json` — Validation thresholds
- `execution_config.json` — Position limits, risk parameters

## Installer

Build an installer using Inno Setup or NSIS:

```bat
iscc installer\setup.iss
```

or

```bat
makensis installer\setup.nsi
```

## Dependencies

- **nlohmann/json** — Single-header JSON library (vendored in `include/third_party/`)
- **Win32 API** — Windows GUI (no Qt dependency)
- **C++ Standard Library** — Everything else
