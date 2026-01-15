#include "application.hpp"
#include <iostream>

int main(int, char**)
{
    try {
        // Create application with window title and dimensions
        Application app("ImGui Viewer", 1280, 720);
        
        // Initialize the application
        app.init();
        
        // Start the main loop
        app.loop();
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
