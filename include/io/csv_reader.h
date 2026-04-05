#pragma once

#include "common/types.h"
#include <string>
#include <vector>

namespace qp {

class CsvReader {
public:
    static std::vector<Bar> load_bars(const std::string& filepath);
};

} // namespace qp
