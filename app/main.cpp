#include "gui/gui_app.h"
#include "common/logging.h"

int main(int argc, char* argv[]) {
    qp::Logger::log_info("Quant Platform starting...");

    qp::GuiApp app;
    int result = app.run(argc, argv);

    qp::Logger::log_info("Quant Platform exiting.");
    return result;
}
