#include "Config.h"
#include <json/json.h>
#include <fstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

Config::Config() {
    // Initialize default endpoints
    initDefaultEndpoints();
    
    // Load configuration from file
    load();
}

Config& Config::getInstance() {
    static Config instance;
    return instance;
}

std::string Config::getApiKey(const std::string& apiName) const {
    auto it = apiKeys.find(apiName);
    if (it != apiKeys.end()) {
        return it->second;
    }
    return "";
}

void Config::setApiKey(const std::string& apiName, const std::string& apiKey) {
    apiKeys[apiName] = apiKey;
}

std::string Config::getEndpoint(const std::string& apiName) const {
    auto it = endpoints.find(apiName);
    if (it != endpoints.end()) {
        return it->second;
    }
    return "";
}

void Config::setEndpoint(const std::string& apiName, const std::string& endpoint) {
    endpoints[apiName] = endpoint;
}

std::pair<std::string, std::string> Config::getLastUsedModel() const {
    return std::make_pair(lastUsedApi, lastUsedModel);
}

void Config::setLastUsedModel(const std::string& api, const std::string& model) {
    lastUsedApi = api;
    lastUsedModel = model;
}

std::string Config::getConfigPath() const {
    // Get home directory
    std::string homePath;
    
    #ifdef _WIN32
        homePath = std::getenv("USERPROFILE");
    #else
        homePath = std::getenv("HOME");
    #endif
    
    // Create config directory path
    fs::path configDir = fs::path(homePath) / ".config" / "gtkks";
    
    // Create config file path
    fs::path configFile = configDir / "config.json";
    
    return configFile.string();
}

void Config::save() {
    // Create JSON object
    Json::Value root;
    
    // Add API keys
    Json::Value apiKeysJson;
    for (const auto& [name, key] : apiKeys) {
        apiKeysJson[name] = key;
    }
    root["apiKeys"] = apiKeysJson;
    
    // Add endpoints
    Json::Value endpointsJson;
    for (const auto& [name, endpoint] : endpoints) {
        endpointsJson[name] = endpoint;
    }
    root["endpoints"] = endpointsJson;
    
    // Add last used model
    root["lastUsedApi"] = lastUsedApi;
    root["lastUsedModel"] = lastUsedModel;
    
    // Get config file path
    std::string configPath = getConfigPath();
    
    // Create directory if it doesn't exist
    fs::path configDir = fs::path(configPath).parent_path();
    if (!fs::exists(configDir)) {
        fs::create_directories(configDir);
    }
    
    // Write to file
    std::ofstream file(configPath);
    if (file.is_open()) {
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "  ";
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        writer->write(root, &file);
        file.close();
    } else {
        std::cerr << "Failed to save configuration to " << configPath << std::endl;
    }
}

void Config::load() {
    // Get config file path
    std::string configPath = getConfigPath();
    
    // Check if file exists
    if (!fs::exists(configPath)) {
        return;
    }
    
    // Read file
    std::ifstream file(configPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open configuration file: " << configPath << std::endl;
        return;
    }
    
    // Parse JSON
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errs;
    if (!Json::parseFromStream(builder, file, &root, &errs)) {
        std::cerr << "Failed to parse configuration: " << errs << std::endl;
        return;
    }
    
    // Load API keys
    if (root.isMember("apiKeys") && root["apiKeys"].isObject()) {
        const Json::Value& apiKeysJson = root["apiKeys"];
        for (const auto& name : apiKeysJson.getMemberNames()) {
            apiKeys[name] = apiKeysJson[name].asString();
        }
    }
    
    // Load endpoints
    if (root.isMember("endpoints") && root["endpoints"].isObject()) {
        const Json::Value& endpointsJson = root["endpoints"];
        for (const auto& name : endpointsJson.getMemberNames()) {
            endpoints[name] = endpointsJson[name].asString();
        }
    }
    
    // Load last used model
    if (root.isMember("lastUsedApi")) {
        lastUsedApi = root["lastUsedApi"].asString();
    }
    
    if (root.isMember("lastUsedModel")) {
        lastUsedModel = root["lastUsedModel"].asString();
    }
}

void Config::initDefaultEndpoints() {
    // Set default endpoints
    endpoints["Ollama"] = "http://localhost:11434";
    endpoints["OpenAI"] = "https://api.openai.com/v1";
    endpoints["Gemini"] = "https://generativelanguage.googleapis.com/v1";
    endpoints["Deepseek"] = "https://api.deepseek.com/v1";
    endpoints["OpenRouter"] = "https://openrouter.ai/api/v1";
} 