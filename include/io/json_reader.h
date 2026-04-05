#pragma once

#include "common/config.h"
#include <string>

namespace qp {

class JsonReader {
public:
    static AppConfig load_app_config(const std::string& path);
    static OptimizerConfig load_optimizer_config(const std::string& path);
    static TruthEngineConfig load_truth_config(const std::string& path);
    static ExecutionConfig load_execution_config(const std::string& path);
};

} // namespace qp
