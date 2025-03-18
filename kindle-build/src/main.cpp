#include <gtkmm.h>
#include "MainWindow.h"
#include "Config.h"
#include <iostream>

int main(int argc, char *argv[]) {
    // Initialize GTK
    auto app = Gtk::Application::create(argc, argv, "org.gtkmm.gtkks");
    
    // Load configuration
    Config& config = Config::getInstance();
    
    // Create main window
    MainWindow window;
    
    // Run application
    return app->run(window);
} 