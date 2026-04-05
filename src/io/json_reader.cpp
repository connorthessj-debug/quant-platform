#include "io/json_reader.h"

namespace qp {

AppConfig JsonReader::load_app_config(const std::string& path) {
    return AppConfig::load(path);
}

OptimizerConfig JsonReader::load_optimizer_config(const std::string& path) {
    return OptimizerConfig::load(path);
}

TruthEngineConfig JsonReader::load_truth_config(const std::string& path) {
    return TruthEngineConfig::load(path);
}

ExecutionConfig JsonReader::load_execution_config(const std::string& path) {
    return ExecutionConfig::load(path);
}

} // namespace qp
