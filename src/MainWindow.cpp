#include "MainWindow.h"
#include "ApiManager.h"
#include "Config.h"
#include <iostream>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/aboutdialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/table.h>
#include <gtkmm/label.h>
#include <gtkmm/dialog.h>
#include <gtkmm/stock.h>

MainWindow::MainWindow()
    : Gtk::Window(),
      settingsDialog("Settings", *this, true),
      aboutDialog() {
    
    // Set up window
    set_title("GTKKS - GTK LLM Client");
    set_default_size(800, 600);
    set_border_width(10);
    
    // Create menu bar
    menuBar.items().push_back(fileMenuItem);
    menuBar.items().push_back(helpMenuItem);
    
    fileMenuItem.set_label("_File");
    fileMenuItem.set_use_underline(true);
    fileMenuItem.set_submenu(fileMenu);
    
    helpMenuItem.set_label("_Help");
    helpMenuItem.set_use_underline(true);
    helpMenuItem.set_submenu(helpMenu);
    
    // Add items to file menu
    fileMenu.items().push_back(settingsMenuItem);
    fileMenu.items().push_back(saveMenuItem);
    fileMenu.items().push_back(loadMenuItem);
    fileMenu.items().push_back(Gtk::Menu_Helpers::SeparatorElem());
    fileMenu.items().push_back(quitMenuItem);
    
    // Add items to help menu
    helpMenu.items().push_back(aboutMenuItem);
    
    // Set up menu items
    settingsMenuItem.set_label("_Settings");
    settingsMenuItem.set_use_underline(true);
    saveMenuItem.set_label("_Save Chat");
    saveMenuItem.set_use_underline(true);
    loadMenuItem.set_label("_Load Chat");
    loadMenuItem.set_use_underline(true);
    quitMenuItem.set_label("_Quit");
    quitMenuItem.set_use_underline(true);
    aboutMenuItem.set_label("_About");
    aboutMenuItem.set_use_underline(true);
    
    // Set up chat view
    chatView.set_border_width(10);
    
    // Add widgets to main box
    mainBox.pack_start(menuBar, Gtk::PACK_SHRINK);
    mainBox.pack_start(chatView);
    
    // Set up settings dialog
    settingsDialog.set_default_size(400, 300);
    settingsDialog.set_title("Settings");
    settingsDialog.add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
    settingsDialog.get_vbox()->add(modelSelector);
    settingsDialog.get_vbox()->set_spacing(10);
    settingsDialog.get_vbox()->set_border_width(10);
    
    // Set up about dialog
    aboutDialog.set_name("GTKKS");
    aboutDialog.set_version("1.0.0");
    aboutDialog.set_copyright("Copyright © 2023");
    aboutDialog.set_comments("A GTK client for LLM services");
    aboutDialog.set_website("https://github.com/yourusername/gtkks");
    aboutDialog.set_website_label("GitHub Repository");
    
    // Add main box to window
    add(mainBox);
    
    // Connect signals
    settingsMenuItem.signal_activate().connect(sigc::mem_fun(*this, &MainWindow::onSettingsClicked));
    saveMenuItem.signal_activate().connect(sigc::mem_fun(*this, &MainWindow::onSaveClicked));
    loadMenuItem.signal_activate().connect(sigc::mem_fun(*this, &MainWindow::onLoadClicked));
    aboutMenuItem.signal_activate().connect(sigc::mem_fun(*this, &MainWindow::onAboutClicked));
    quitMenuItem.signal_activate().connect(sigc::mem_fun(*this, &MainWindow::onQuitClicked));
    modelSelector.signal_api_config_changed().connect(sigc::mem_fun(*this, &MainWindow::onApiConfigChanged));
    
    // Load last used model
    loadLastUsedModel();
    
    // Show all widgets
    show_all_children();
}

MainWindow::~MainWindow() {
}

void MainWindow::onSettingsClicked() {
    // Show settings dialog
    settingsDialog.show_all();
    int response = settingsDialog.run();
    settingsDialog.hide();
    
    // Update chat view with the selected model when dialog is closed
    if (response == Gtk::RESPONSE_CLOSE) {
        updateChatView();
    }
}

void MainWindow::onSaveClicked() {
    // Create file chooser dialog
    Gtk::FileChooserDialog dialog("Save Chat", Gtk::FILE_CHOOSER_ACTION_SAVE);
    dialog.set_transient_for(*this);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);
    
    // Add filters
    Gtk::FileFilter filter_json;
    filter_json.set_name("JSON files");
    filter_json.add_pattern("*.json");
    dialog.add_filter(filter_json);
    
    Gtk::FileFilter filter_any;
    filter_any.set_name("Any files");
    filter_any.add_pattern("*");
    dialog.add_filter(filter_any);
    
    // Set default filename
    dialog.set_current_name("chat.json");
    
    // Show dialog
    int result = dialog.run();
    
    // Handle result
    if (result == Gtk::RESPONSE_OK) {
        std::string filename = dialog.get_filename();
        chatView.saveChat(filename);
    }
}

void MainWindow::onLoadClicked() {
    // Create file chooser dialog
    Gtk::FileChooserDialog dialog("Load Chat", Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.set_transient_for(*this);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);
    
    // Add filters
    Gtk::FileFilter filter_json;
    filter_json.set_name("JSON files");
    filter_json.add_pattern("*.json");
    dialog.add_filter(filter_json);
    
    Gtk::FileFilter filter_any;
    filter_any.set_name("Any files");
    filter_any.add_pattern("*");
    dialog.add_filter(filter_any);
    
    // Show dialog
    int result = dialog.run();
    
    // Handle result
    if (result == Gtk::RESPONSE_OK) {
        std::string filename = dialog.get_filename();
        chatView.loadChat(filename);
    }
}

void MainWindow::onAboutClicked() {
    // Show about dialog
    aboutDialog.show();
    aboutDialog.run();
    aboutDialog.hide();
}

void MainWindow::onApiConfigChanged() {
    // Update chat view with new API configuration
    auto [apiName, modelName] = modelSelector.getSelectedModel();
    
    // Save as last used model
    ApiManager::getInstance().setLastUsedModel(apiName, modelName);
    
    // Update chat view
    updateChatView();
}

void MainWindow::loadLastUsedModel() {
    // Get last used model
    auto [apiName, modelName] = ApiManager::getInstance().getLastUsedModel();
    
    // Set selected model
    if (!apiName.empty() && !modelName.empty()) {
        modelSelector.setSelectedModel(apiName, modelName);
    }
    
    // Update chat view
    updateChatView();
}

void MainWindow::updateChatView() {
    // Get selected model
    auto [apiName, modelName] = modelSelector.getSelectedModel();
    
    // Get API
    auto api = ApiManager::getInstance().getApi(apiName);
    
    // Set API and model for chat view
    if (api && !modelName.empty()) {
        chatView.setApi(api, modelName);
    }
}

void MainWindow::onQuitClicked() {
    hide();
} 