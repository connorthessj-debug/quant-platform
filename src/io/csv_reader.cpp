#include "io/csv_reader.h"
#include "common/time_utils.h"
#include "common/logging.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace qp {

std::vector<Bar> CsvReader::load_bars(const std::string& filepath) {
    std::vector<Bar> bars;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        Logger::log_error("Failed to open CSV file: " + filepath);
        return bars;
    }

    std::string line;
    // Skip header line
    if (!std::getline(file, line)) {
        Logger::log_error("CSV file is empty: " + filepath);
        return bars;
    }

    int line_num = 1;
    while (std::getline(file, line)) {
        line_num++;
        if (line.empty()) continue;

        // Remove trailing \r if present (Windows line endings)
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        std::istringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(ss, token, ',')) {
            // Trim whitespace
            auto start = token.find_first_not_of(" \t");
            auto end = token.find_last_not_of(" \t");
            if (start != std::string::npos) {
                tokens.push_back(token.substr(start, end - start + 1));
            } else {
                tokens.push_back("");
            }
        }

        if (tokens.size() < 6) {
            Logger::log_warning("Skipping malformed CSV line " + std::to_string(line_num));
            continue;
        }

        Bar bar;
        bar.time = parse_date(tokens[0]);
        try {
            bar.open = std::stod(tokens[1]);
            bar.high = std::stod(tokens[2]);
            bar.low = std::stod(tokens[3]);
            bar.close = std::stod(tokens[4]);
            bar.volume = std::stod(tokens[5]);
        } catch (const std::exception& e) {
            Logger::log_warning("Skipping line " + std::to_string(line_num) +
                              ": parse error: " + e.what());
            continue;
        }

        bars.push_back(bar);
    }

    Logger::log_info("Loaded " + std::to_string(bars.size()) + " bars from " + filepath);
    return bars;
}

} // namespace qp
