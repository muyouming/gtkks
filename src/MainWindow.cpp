#include "MainWindow.h"
#include "ApiManager.h"
#include "Config.h"
#ifdef KINDLE
#include "KindleConfig.h"
#endif
#include <iostream>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/aboutdialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/dialog.h>
#include <gtkmm/stock.h>
#include <gio/gio.h>

MainWindow::MainWindow()
    : Gtk::Window(),
      mainBox(Gtk::ORIENTATION_VERTICAL, 0),
      settingsDialog("Settings", *this, true),
      aboutDialog() {
    
    // Set up window
    set_title("GTKKS - GTK LLM Client");
#ifdef KINDLE
    // Use Kindle screen dimensions
    set_default_size(KindleUtils::SCREEN_WIDTH, KindleUtils::SCREEN_HEIGHT);
    // Optimize for e-ink display
    Gdk::RGBA white;
    white.set_rgba(1.0, 1.0, 1.0, 1.0);
    override_background_color(white);
#else
    set_default_size(800, 600);
#endif
    set_border_width(0);
    
    // Set up header bar
    headerBar.set_title("GTKKS - GTK LLM Client");
    headerBar.set_show_close_button(true);
    
    // Create menu button
    menuButton.set_image_from_icon_name("open-menu-symbolic", Gtk::ICON_SIZE_BUTTON);
    
    // Create menu
    menuPopover.set_relative_to(menuButton);
    menuBox.set_orientation(Gtk::ORIENTATION_VERTICAL);
    menuBox.set_margin_start(10);
    menuBox.set_margin_end(10);
    menuBox.set_margin_top(10);
    menuBox.set_margin_bottom(10);
    menuBox.set_spacing(5);
    
    // Add menu items
    menuBox.add(settingsButton);
    menuBox.add(saveButton);
    menuBox.add(loadButton);
    menuBox.add(aboutButton);
    
    // Set up menu items
    settingsButton.set_label("Settings");
    settingsButton.set_halign(Gtk::ALIGN_START);
    saveButton.set_label("Save Chat");
    saveButton.set_halign(Gtk::ALIGN_START);
    loadButton.set_label("Load Chat");
    loadButton.set_halign(Gtk::ALIGN_START);
    aboutButton.set_label("About");
    aboutButton.set_halign(Gtk::ALIGN_START);
    
    // Add menu box to popover
    menuPopover.add(menuBox);
    menuButton.set_popover(menuPopover);
    
    // Add menu button to header bar
    headerBar.pack_end(menuButton);
    
    // Set up chat view
    chatView.set_margin_start(10);
    chatView.set_margin_end(10);
    chatView.set_margin_top(10);
    chatView.set_margin_bottom(10);
    chatView.set_vexpand(true);
    
#ifdef KINDLE
    // Set larger font for Kindle
    Pango::FontDescription font;
    font.set_family("Sans");
    font.set_size(KindleUtils::DEFAULT_FONT_SIZE * Pango::SCALE);
    chatView.override_font(font);
#endif
    
    // Add widgets to main box
    mainBox.pack_start(chatView, true, true, 0);
    
    // Set up settings dialog
    settingsDialog.set_default_size(400, 300);
    settingsDialog.set_title("Settings");
    settingsDialog.add_button("Close", Gtk::RESPONSE_CLOSE);
    settingsDialog.get_content_area()->add(modelSelector);
    settingsDialog.get_content_area()->set_spacing(10);
    settingsDialog.get_content_area()->set_margin_start(10);
    settingsDialog.get_content_area()->set_margin_end(10);
    settingsDialog.get_content_area()->set_margin_top(10);
    settingsDialog.get_content_area()->set_margin_bottom(10);
    
    // Set up about dialog
    aboutDialog.set_program_name("GTKKS");
    aboutDialog.set_version("1.0.0");
    aboutDialog.set_copyright("Copyright © 2023");
    aboutDialog.set_comments("A GTK client for LLM services");
    aboutDialog.set_license_type(Gtk::LICENSE_GPL_3_0);
    aboutDialog.set_website("https://github.com/yourusername/gtkks");
    aboutDialog.set_website_label("GitHub Repository");
    aboutDialog.set_authors({"Your Name"});
    
    // Set window titlebar
    set_titlebar(headerBar);
    
    // Add main box to window
    add(mainBox);
    
    // Connect signals
    settingsButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onSettingsClicked));
    saveButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onSaveClicked));
    loadButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onLoadClicked));
    aboutButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onAboutClicked));
    menuButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onMenuButtonClicked));
    modelSelector.signal_api_config_changed().connect(sigc::mem_fun(*this, &MainWindow::onApiConfigChanged));
    
    // Load last used model
    loadLastUsedModel();
    
    // Show all widgets
    show_all_children();
    menuBox.show_all();
    
#ifdef KINDLE
    // Initialize Kindle display
    KindleUtils::setHighQualityRefresh();
    KindleUtils::refreshDisplay();
#endif
}

MainWindow::~MainWindow() {
}

void MainWindow::onMenuButtonClicked() {
    menuPopover.show_all();
    menuPopover.popup();
#ifdef KINDLE
    KindleUtils::refreshDisplay();
#endif
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
    
#ifdef KINDLE
    KindleUtils::refreshDisplay();
#endif
}

void MainWindow::onSaveClicked() {
    // Create file chooser dialog
    Gtk::FileChooserDialog dialog("Save Chat", Gtk::FILE_CHOOSER_ACTION_SAVE);
    dialog.set_transient_for(*this);
    dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button("Save", Gtk::RESPONSE_OK);
    
    // Add filters
    auto filter_json = Gtk::FileFilter::create();
    filter_json->set_name("JSON files");
    filter_json->add_pattern("*.json");
    dialog.add_filter(filter_json);
    
    auto filter_any = Gtk::FileFilter::create();
    filter_any->set_name("Any files");
    filter_any->add_pattern("*");
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
    dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button("Open", Gtk::RESPONSE_OK);
    
    // Add filters
    auto filter_json = Gtk::FileFilter::create();
    filter_json->set_name("JSON files");
    filter_json->add_pattern("*.json");
    dialog.add_filter(filter_json);
    
    auto filter_any = Gtk::FileFilter::create();
    filter_any->set_name("Any files");
    filter_any->add_pattern("*");
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
    close();
} 