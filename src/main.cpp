#include "application.hpp"
#include "util/logging.hpp"
#include <exception>
#include <iostream>

int main(int, char**)
{
    try {
        initialize_logging();
        log_info("=== Application Starting ===");

        // For safety, scope application so app is shutdown before logging is terminated
        {
            Application app("OhPossum Gimbal Client", 1280, 720);
            app.init();
            app.loop();
        }

        log_info("=== Application Shutting Down ===");
        shutdown_logging();
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error:  " << e.what() << std::endl;
        log_critical("Fatal error: {}", e.what());
        shutdown_logging();
        return 1;
    }
}
