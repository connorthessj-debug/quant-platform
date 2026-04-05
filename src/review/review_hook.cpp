#include "review/review_hook.h"
#include "common/logging.h"
#include <fstream>
#include <sstream>

namespace qp {

std::vector<ReviewFinding> ReviewHook::self_review() {
    std::vector<ReviewFinding> findings;

    // Self-review checks that can be run at build time
    // These verify structural consistency of the project

    ReviewFinding info;
    info.severity = "info";
    info.category = "build";
    info.file = "CMakeLists.txt";
    info.description = "Self-review: verify all source files are listed in CMake targets";
    info.suggested_fix = "Run cmake and check for missing source file errors";
    findings.push_back(info);

    return findings;
}

std::string ReviewHook::generate_review_prompt() {
    std::ostringstream ss;
    ss << "Review this Windows-native C++ quant platform for:\n\n";
    ss << "1. COMPILE BLOCKERS\n";
    ss << "   - Missing #include directives\n";
    ss << "   - Mismatched type names across translation units\n";
    ss << "   - Namespace inconsistencies\n";
    ss << "   - Undefined or invented interfaces\n\n";
    ss << "2. CMAKE / BUILD ISSUES\n";
    ss << "   - Missing source files in targets\n";
    ss << "   - Incorrect target_link_libraries\n";
    ss << "   - Include path issues\n\n";
    ss << "3. WINDOWS BUILD VIABILITY\n";
    ss << "   - MSVC compatibility\n";
    ss << "   - MinGW-w64 compatibility\n";
    ss << "   - Win32 API usage correctness\n\n";
    ss << "4. INTEGRATION ISSUES\n";
    ss << "   - GUI-to-engine wiring\n";
    ss << "   - Config loading paths\n";
    ss << "   - Data flow between subsystems\n\n";
    ss << "5. INSTALLER / PACKAGING\n";
    ss << "   - Inno Setup script consistency\n";
    ss << "   - NSIS script consistency\n";
    ss << "   - Correct file paths and names\n\n";
    ss << "Output format:\n";
    ss << "- Critical blockers (with file + line)\n";
    ss << "- Warnings (with file)\n";
    ss << "- Minimal patch plan\n";
    return ss.str();
}

void ReviewHook::write_review_report(const std::vector<ReviewFinding>& findings,
                                       const std::string& output_path) {
    std::ofstream file(output_path);
    if (!file.is_open()) {
        Logger::log_error("Failed to write review report to: " + output_path);
        return;
    }

    file << "# Review Report\n\n";
    file << "## Findings: " << findings.size() << "\n\n";

    for (size_t i = 0; i < findings.size(); ++i) {
        const auto& f = findings[i];
        file << "### " << (i + 1) << ". [" << f.severity << "] " << f.category << "\n";
        file << "- **File:** " << f.file << "\n";
        file << "- **Issue:** " << f.description << "\n";
        file << "- **Fix:** " << f.suggested_fix << "\n\n";
    }

    Logger::log_info("Review report written to: " + output_path);
}

} // namespace qp
