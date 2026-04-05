#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <utility>

namespace qp {

using Timestamp = int64_t;

enum class Side { Buy, Sell };
enum class OrderType { Market, Limit, Stop };
enum class OrderStatus { Pending, Filled, Cancelled, Rejected };

struct Bar {
    Timestamp time = 0;
    double open = 0.0;
    double high = 0.0;
    double low = 0.0;
    double close = 0.0;
    double volume = 0.0;
};

struct Order {
    int id = 0;
    Side side = Side::Buy;
    OrderType type = OrderType::Market;
    double price = 0.0;
    double quantity = 0.0;
    OrderStatus status = OrderStatus::Pending;
    Timestamp created_time = 0;
    Timestamp fill_time = 0;
};

struct Trade {
    int id = 0;
    Side side = Side::Buy;
    double entry_price = 0.0;
    double exit_price = 0.0;
    double quantity = 0.0;
    double pnl = 0.0;
    double commission = 0.0;
    Timestamp entry_time = 0;
    Timestamp exit_time = 0;
};

struct Position {
    Side side = Side::Buy;
    double quantity = 0.0;
    double avg_price = 0.0;
    double unrealized_pnl = 0.0;
};

struct EquityCurve {
    std::vector<Timestamp> times;
    std::vector<double> equity;
};

struct StrategyParams {
    std::string name;
    std::vector<std::pair<std::string, double>> values;

    double get(const std::string& key, double default_val) const {
        for (const auto& kv : values) {
            if (kv.first == key) return kv.second;
        }
        return default_val;
    }

    void set(const std::string& key, double val) {
        for (auto& kv : values) {
            if (kv.first == key) { kv.second = val; return; }
        }
        values.emplace_back(key, val);
    }
};

} // namespace qp
