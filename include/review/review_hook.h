#pragma once

#include <string>
#include <vector>

namespace qp {

struct ReviewFinding {
    std::string severity;
    std::string category;
    std::string file;
    std::string description;
    std::string suggested_fix;
};

class ReviewHook {
public:
    static std::vector<ReviewFinding> self_review();
    static std::string generate_review_prompt();
    static void write_review_report(const std::vector<ReviewFinding>& findings,
                                     const std::string& output_path);
};

} // namespace qp
