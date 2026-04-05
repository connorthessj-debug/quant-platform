#pragma once

#include <string>
#include <cstdint>
#include <ctime>
#include <sstream>
#include <iomanip>

namespace qp {

using Timestamp = int64_t;

inline Timestamp parse_date(const std::string& date_str) {
    std::tm tm = {};
    std::istringstream ss(date_str);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    if (ss.fail()) {
        ss.clear();
        ss.str(date_str);
        ss >> std::get_time(&tm, "%Y/%m/%d");
    }
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    tm.tm_isdst = -1;
    return static_cast<Timestamp>(std::mktime(&tm));
}

inline Timestamp parse_datetime(const std::string& dt_str) {
    std::tm tm = {};
    std::istringstream ss(dt_str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (ss.fail()) {
        return parse_date(dt_str);
    }
    tm.tm_isdst = -1;
    return static_cast<Timestamp>(std::mktime(&tm));
}

inline std::string timestamp_to_string(Timestamp ts) {
    std::time_t t = static_cast<std::time_t>(ts);
    std::tm* tm = std::localtime(&t);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm);
    return std::string(buf);
}

inline std::string timestamp_to_date(Timestamp ts) {
    std::time_t t = static_cast<std::time_t>(ts);
    std::tm* tm = std::localtime(&t);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", tm);
    return std::string(buf);
}

} // namespace qp
