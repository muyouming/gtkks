#pragma once

#include <gtkmm.h>
#include "ChatView.h"
#include "ModelSelector.h"

class MainWindow : public Gtk::Window {
public:
    MainWindow();
    virtual ~MainWindow();
    
private:
    // UI components
    Gtk::VBox mainBox;
    
    // Menu
    Gtk::MenuBar menuBar;
    Gtk::Menu fileMenu;
    Gtk::Menu helpMenu;
    Gtk::MenuItem fileMenuItem;
    Gtk::MenuItem helpMenuItem;
    Gtk::MenuItem settingsMenuItem;
    Gtk::MenuItem saveMenuItem;
    Gtk::MenuItem loadMenuItem;
    Gtk::MenuItem aboutMenuItem;
    Gtk::MenuItem quitMenuItem;
    
    // Dialogs
    Gtk::Dialog settingsDialog;
    Gtk::AboutDialog aboutDialog;
    
    // Main components
    ModelSelector modelSelector;
    ChatView chatView;
    
    // Signal handlers
    void onSettingsClicked();
    void onSaveClicked();
    void onLoadClicked();
    void onAboutClicked();
    void onQuitClicked();
    void onApiConfigChanged();
    
    // Helper methods
    void loadLastUsedModel();
    void updateChatView();
}; 