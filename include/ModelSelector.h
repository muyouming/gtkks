#pragma once

#include <gtkmm.h>
#include <string>
#include <utility>
#include "Config.h"

class ModelSelector : public Gtk::VBox {
public:
    ModelSelector();
    virtual ~ModelSelector();
    
    // Get the selected model
    std::pair<std::string, std::string> getSelectedModel() const;
    
    // Set the selected model
    void setSelectedModel(const std::string& apiName, const std::string& modelName);
    
    // Signal for API configuration changed
    sigc::signal<void> signal_api_config_changed();
    
private:
    // Configuration
    Config* config;
    
    // Selected API and model
    std::string apiName;
    std::string modelName;
    
    // UI components
    Gtk::Table table;
    
    // Labels
    Gtk::Label apiLabel;
    Gtk::Label modelLabel;
    Gtk::Label apiKeyLabel;
    Gtk::Label endpointLabel;
    
    // ComboBoxes
    Gtk::ComboBox apiComboBox;
    Gtk::ComboBox modelComboBox;
    
    // Entries
    Gtk::Entry apiKeyEntry;
    Gtk::Entry endpointEntry;
    
    // Buttons
    Gtk::Button saveButton;
    
    // Models for ComboBoxes
    class ApiColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        ApiColumns() { add(name); }
        Gtk::TreeModelColumn<Glib::ustring> name;
    };
    
    class ModelColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        ModelColumns() { add(name); }
        Gtk::TreeModelColumn<Glib::ustring> name;
    };
    
    ApiColumns apiColumns;
    ModelColumns modelColumns;
    
    Glib::RefPtr<Gtk::ListStore> apiListStore;
    Glib::RefPtr<Gtk::ListStore> modelListStore;
    
    // Signal handlers
    void onApiChanged();
    void onSaveClicked();
    
    // Helper methods
    void populateApiComboBox();
    void populateModelComboBox(const std::string& apiName);
    
    // Signals
    sigc::signal<void> m_signal_api_config_changed;
}; 