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
    Gtk::HeaderBar headerBar;
    Gtk::Box mainBox;
    
    // Menu
    Gtk::MenuButton menuButton;
    Gtk::Popover menuPopover;
    Gtk::Box menuBox;
    Gtk::Button settingsButton;
    Gtk::Button saveButton;
    Gtk::Button loadButton;
    Gtk::Button aboutButton;
    
    // Dialogs
    Gtk::Dialog settingsDialog;
    Gtk::AboutDialog aboutDialog;
    
    // Main components
    ModelSelector modelSelector;
    ChatView chatView;
    
    // Signal handlers
    void onMenuButtonClicked();
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