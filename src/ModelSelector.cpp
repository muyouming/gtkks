#include "ModelSelector.h"
#include "ApiManager.h"
#include <iostream>

ModelSelector::ModelSelector() : 
    Gtk::VBox(false, 0),
    table(5, 2, false) {
    
    // Get config singleton
    Config& configRef = Config::getInstance();
    config = &configRef;
    
    // Set up UI
    set_border_width(10);
    
    // Configure table
    table.set_col_spacings(10);
    table.set_row_spacings(10);
    
    // Add labels
    apiLabel.set_text("API:");
    apiLabel.set_alignment(0, 0.5);
    table.attach(apiLabel, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
    
    modelLabel.set_text("Model:");
    modelLabel.set_alignment(0, 0.5);
    table.attach(modelLabel, 0, 1, 1, 2, Gtk::FILL, Gtk::FILL);
    
    apiKeyLabel.set_text("API Key:");
    apiKeyLabel.set_alignment(0, 0.5);
    table.attach(apiKeyLabel, 0, 1, 2, 3, Gtk::FILL, Gtk::FILL);
    
    endpointLabel.set_text("Endpoint:");
    endpointLabel.set_alignment(0, 0.5);
    table.attach(endpointLabel, 0, 1, 3, 4, Gtk::FILL, Gtk::FILL);
    
    // Add ComboBoxes
    apiListStore = Gtk::ListStore::create(apiColumns);
    apiComboBox.set_model(apiListStore);
    apiComboBox.pack_start(apiColumns.name);
    table.attach(apiComboBox, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL);
    
    modelListStore = Gtk::ListStore::create(modelColumns);
    modelComboBox.set_model(modelListStore);
    modelComboBox.pack_start(modelColumns.name);
    table.attach(modelComboBox, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL);
    
    // Add entries
    table.attach(apiKeyEntry, 1, 2, 2, 3, Gtk::FILL | Gtk::EXPAND, Gtk::FILL);
    
    table.attach(endpointEntry, 1, 2, 3, 4, Gtk::FILL | Gtk::EXPAND, Gtk::FILL);
    
    // Add save button
    saveButton.set_label("Save");
    table.attach(saveButton, 1, 2, 4, 5, Gtk::FILL, Gtk::FILL, 0, 0);
    
    // Add table to box
    pack_start(table, true, true, 0);
    
    // Connect signals
    apiComboBox.signal_changed().connect(sigc::mem_fun(*this, &ModelSelector::onApiChanged));
    saveButton.signal_clicked().connect(sigc::mem_fun(*this, &ModelSelector::onSaveClicked));
    
    // Populate API ComboBox
    populateApiComboBox();
    
    // Show all
    show_all_children();
}

ModelSelector::~ModelSelector() {
}

std::pair<std::string, std::string> ModelSelector::getSelectedModel() const {
    return std::make_pair(apiName, modelName);
}

void ModelSelector::setSelectedModel(const std::string& newApiName, const std::string& newModelName) {
    apiName = newApiName;
    modelName = newModelName;
    
    // Update ComboBoxes
    for (auto iter = apiListStore->children().begin(); iter != apiListStore->children().end(); ++iter) {
        Gtk::TreeModel::Row row = *iter;
        if (row[apiColumns.name] == apiName) {
            apiComboBox.set_active(iter);
            break;
        }
    }
    
    // Model ComboBox will be updated by onApiChanged
}

sigc::signal<void> ModelSelector::signal_api_config_changed() {
    return m_signal_api_config_changed;
}

void ModelSelector::onApiChanged() {
    Gtk::TreeModel::iterator iter = apiComboBox.get_active();
    if (iter) {
        Gtk::TreeModel::Row apiRow = *iter;
        Glib::ustring ustr = apiRow[apiColumns.name];
        apiName = ustr.raw();
        
        // Update model combo box with models for this API
        populateModelComboBox(apiName);
        
        // Update API key entry
        apiKeyEntry.set_text(config->getApiKey(apiName));
        
        // Update endpoint entry
        endpointEntry.set_text(config->getEndpoint(apiName));
    }
}

void ModelSelector::onSaveClicked() {
    // Get selected API
    Gtk::TreeModel::iterator apiIter = apiComboBox.get_active();
    if (!apiIter) {
        return;
    }
    
    // Get selected model
    Gtk::TreeModel::iterator modelIter = modelComboBox.get_active();
    if (!modelIter) {
        return;
    }
    
    // Get values
    Gtk::TreeModel::Row apiRow = *apiIter;
    Gtk::TreeModel::Row modelRow = *modelIter;
    
    Glib::ustring apiUstr = apiRow[apiColumns.name];
    Glib::ustring modelUstr = modelRow[modelColumns.name];
    
    apiName = apiUstr.raw();
    modelName = modelUstr.raw();
    
    // Save API key and endpoint
    config->setApiKey(apiName, apiKeyEntry.get_text());
    config->setEndpoint(apiName, endpointEntry.get_text());
    
    // Save selected model
    config->setLastUsedModel(apiName, modelName);
    
    // Save config
    config->save();
    
    // Emit signal
    m_signal_api_config_changed.emit();
}

void ModelSelector::populateApiComboBox() {
    // Clear list store
    apiListStore->clear();
    
    // Add all available APIs
    auto& apiManager = ApiManager::getInstance();
    for (const auto& apiName : apiManager.getAvailableApiNames()) {
        Gtk::TreeModel::Row row = *(apiListStore->append());
        row[apiColumns.name] = Glib::ustring(apiName);
    }
    
    // Select first item
    if (!apiListStore->children().empty()) {
        apiComboBox.set_active(apiListStore->children().begin());
    }
}

void ModelSelector::populateModelComboBox(const std::string& apiName) {
    // Clear list store
    modelListStore->clear();
    
    // Get available models for this API
    auto& apiManager = ApiManager::getInstance();
    auto api = apiManager.getApi(apiName);
    
    if (api) {
        for (const auto& modelName : api->getAvailableModels()) {
            Gtk::TreeModel::Row row = *(modelListStore->append());
            row[modelColumns.name] = Glib::ustring(modelName);
            
            // If this is the model we want to select
            if (modelName == this->modelName) {
                modelComboBox.set_active(modelListStore->children().size() - 1);
            }
        }
        
        // If no model was selected, select the first one
        if (!modelComboBox.get_active() && !modelListStore->children().empty()) {
            modelComboBox.set_active(modelListStore->children().begin());
            
            // Update model name
            auto iter = modelComboBox.get_active();
            if (iter) {
                Gtk::TreeModel::Row modelRow = *iter;
                Glib::ustring modelUstr = modelRow[modelColumns.name];
                this->modelName = modelUstr.raw();
            }
        }
    }
} 