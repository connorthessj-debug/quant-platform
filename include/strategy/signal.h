#pragma once

#include "common/types.h"
#include <string>

namespace qp {

enum class SignalType { None, Buy, Sell };

struct Signal {
    SignalType type = SignalType::None;
    double strength = 0.0;
    Timestamp time = 0;
    std::string source;
};

} // namespace qp
