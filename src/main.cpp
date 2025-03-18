#include <gtkmm.h>
#include "MainWindow.h"
#include "Config.h"
#include <iostream>

int main(int argc, char *argv[]) {
    // Initialize GTK
    Gtk::Main kit(argc, argv);
    
    // Load configuration
    Config& config = Config::getInstance();
    
    // Create main window
    MainWindow window;
    window.show_all();
    
    // Run application
    Gtk::Main::run(window);
    
    return 0;
} 