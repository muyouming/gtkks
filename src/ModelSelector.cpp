#include "ModelSelector.h"
#include "ApiManager.h"
#include <iostream>

ModelSelector::ModelSelector() : 
    Gtk::Box(Gtk::ORIENTATION_VERTICAL, 0) {
    
    // Get config singleton
    Config& configRef = Config::getInstance();
    config = &configRef;
    
    // Set up UI
    set_border_width(10);
    
    // Create grid
    grid.set_column_spacing(10);
    grid.set_row_spacing(10);
    
    // Add labels
    apiLabel.set_text("API:");
    apiLabel.set_halign(Gtk::ALIGN_START);
    grid.attach(apiLabel, 0, 0, 1, 1);
    
    modelLabel.set_text("Model:");
    modelLabel.set_halign(Gtk::ALIGN_START);
    grid.attach(modelLabel, 0, 1, 1, 1);
    
    apiKeyLabel.set_text("API Key:");
    apiKeyLabel.set_halign(Gtk::ALIGN_START);
    grid.attach(apiKeyLabel, 0, 2, 1, 1);
    
    endpointLabel.set_text("Endpoint:");
    endpointLabel.set_halign(Gtk::ALIGN_START);
    grid.attach(endpointLabel, 0, 3, 1, 1);
    
    // Add ComboBoxes
    apiListStore = Gtk::ListStore::create(apiColumns);
    apiComboBox.set_model(apiListStore);
    apiComboBox.pack_start(apiColumns.name);
    grid.attach(apiComboBox, 1, 0, 1, 1);
    
    modelListStore = Gtk::ListStore::create(modelColumns);
    modelComboBox.set_model(modelListStore);
    modelComboBox.pack_start(modelColumns.name);
    grid.attach(modelComboBox, 1, 1, 1, 1);
    
    // Add entries
    apiKeyEntry.set_hexpand(true);
    grid.attach(apiKeyEntry, 1, 2, 1, 1);
    
    endpointEntry.set_hexpand(true);
    grid.attach(endpointEntry, 1, 3, 1, 1);
    
    // Add save button
    saveButton.set_label("Save");
    saveButton.set_halign(Gtk::ALIGN_END);
    grid.attach(saveButton, 1, 4, 1, 1);
    
    // Add grid to box
    pack_start(grid, true, true, 0);
    
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
    
    // Hide dialog
    hide();
    
    // Emit signal
    m_signal_api_config_changed.emit();
}

void ModelSelector::populateApiComboBox() {
    // Clear existing items
    apiListStore->clear();
    
    // Get API manager
    ApiManager& apiManager = ApiManager::getInstance();
    
    // Add APIs
    for (const auto& apiName : apiManager.getAvailableApis()) {
        Gtk::TreeModel::Row row = *(apiListStore->append());
        row[apiColumns.name] = apiName;
    }
    
    // Select first API
    if (apiListStore->children().size() > 0) {
        apiComboBox.set_active(0);
    }
}

void ModelSelector::populateModelComboBox(const std::string& apiName) {
    // Clear existing items
    modelListStore->clear();
    
    // Get API manager
    ApiManager& apiManager = ApiManager::getInstance();
    
    // Get API
    auto api = apiManager.getApi(apiName);
    if (!api) {
        return;
    }
    
    // Add models
    for (const auto& modelName : api->getAvailableModels()) {
        Gtk::TreeModel::Row row = *(modelListStore->append());
        row[modelColumns.name] = modelName;
    }
    
    // Select first model
    if (modelListStore->children().size() > 0) {
        modelComboBox.set_active(0);
    }
} 