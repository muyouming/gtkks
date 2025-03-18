#include "Config.h"
#include "LLMApi.h" // For SimpleJson
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>

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
    SimpleJson root;
    
    // Add API keys
    SimpleJson apiKeysJson;
    for (const auto& [name, key] : apiKeys) {
        apiKeysJson.addToObject(name, SimpleJson(key));
    }
    root.addToObject("apiKeys", apiKeysJson);
    
    // Add endpoints
    SimpleJson endpointsJson;
    for (const auto& [name, endpoint] : endpoints) {
        endpointsJson.addToObject(name, SimpleJson(endpoint));
    }
    root.addToObject("endpoints", endpointsJson);
    
    // Add last used model
    root.addToObject("lastUsedApi", SimpleJson(lastUsedApi));
    root.addToObject("lastUsedModel", SimpleJson(lastUsedModel));
    
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
        file << root.toJsonString();
        file.close();
    } else {
        std::cerr << "Failed to save configuration to " << configPath << std::endl;
    }
}

// Simple JSON parsing function for our config file
SimpleJson parseJson(std::istream& input) {
    // We only need a simple parser for our config file
    // This is not a full JSON parser but works for our specific needs
    
    std::string line;
    std::string content;
    
    while (std::getline(input, line)) {
        content += line;
    }
    
    // Remove whitespace for simplicity
    content.erase(std::remove_if(content.begin(), content.end(), 
                  [](unsigned char c) { return std::isspace(c); }), content.end());
    
    // Parse the JSON
    if (content.empty() || content[0] != '{' || content.back() != '}') {
        return SimpleJson(); // Empty object
    }
    
    SimpleJson result;
    
    // Remove outer braces
    content = content.substr(1, content.length() - 2);
    
    // Split by commas, but be aware of nested objects and arrays
    size_t pos = 0;
    int bracketLevel = 0;
    int braceLevel = 0;
    std::string keyValuePair;
    
    for (size_t i = 0; i <= content.length(); i++) {
        if (i == content.length() || (content[i] == ',' && bracketLevel == 0 && braceLevel == 0)) {
            if (pos < i) {
                keyValuePair = content.substr(pos, i - pos);
                
                // Find the colon separator
                size_t colonPos = keyValuePair.find(':');
                if (colonPos != std::string::npos) {
                    std::string key = keyValuePair.substr(0, colonPos);
                    std::string value = keyValuePair.substr(colonPos + 1);
                    
                    // Remove quotes from key
                    if (key.front() == '"' && key.back() == '"') {
                        key = key.substr(1, key.length() - 2);
                    }
                    
                    // Parse value based on its type
                    if (value.front() == '"' && value.back() == '"') {
                        // String value
                        value = value.substr(1, value.length() - 2);
                        result.addToObject(key, SimpleJson(value));
                    } else if (value.front() == '{' && value.back() == '}') {
                        // Object value
                        std::istringstream objectStream(value);
                        result.addToObject(key, parseJson(objectStream));
                    } else if (value == "true") {
                        result.addToObject(key, SimpleJson(true));
                    } else if (value == "false") {
                        result.addToObject(key, SimpleJson(false));
                    } else if (value == "null") {
                        result.addToObject(key, SimpleJson());
                    } else {
                        // Try to parse as number
                        try {
                            double number = std::stod(value);
                            result.addToObject(key, SimpleJson(number));
                        } catch (...) {
                            // If all else fails, store as string
                            result.addToObject(key, SimpleJson(value));
                        }
                    }
                }
            }
            pos = i + 1;
        } else if (content[i] == '{') {
            braceLevel++;
        } else if (content[i] == '}') {
            braceLevel--;
        } else if (content[i] == '[') {
            bracketLevel++;
        } else if (content[i] == ']') {
            bracketLevel--;
        }
    }
    
    return result;
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
    SimpleJson root = parseJson(file);
    
    // Load API keys
    if (root.hasKey("apiKeys")) {
        const SimpleJson& apiKeysJson = root["apiKeys"];
        // In a full implementation, we would iterate through all keys
        // For simplicity, we'll just check for known API names
        const std::string apiNames[] = {
            "openai", "ollama", "gemini", "deepseek", "openrouter"
        };
        
        for (const auto& name : apiNames) {
            if (apiKeysJson.hasKey(name)) {
                apiKeys[name] = apiKeysJson[name].asString();
            }
        }
    }
    
    // Load endpoints
    if (root.hasKey("endpoints")) {
        const SimpleJson& endpointsJson = root["endpoints"];
        const std::string apiNames[] = {
            "openai", "ollama", "gemini", "deepseek", "openrouter"
        };
        
        for (const auto& name : apiNames) {
            if (endpointsJson.hasKey(name)) {
                endpoints[name] = endpointsJson[name].asString();
            }
        }
    }
    
    // Load last used model
    if (root.hasKey("lastUsedApi")) {
        lastUsedApi = root["lastUsedApi"].asString();
    }
    
    if (root.hasKey("lastUsedModel")) {
        lastUsedModel = root["lastUsedModel"].asString();
    }
}

void Config::initDefaultEndpoints() {
    endpoints["openai"] = "https://api.openai.com/v1";
    endpoints["ollama"] = "http://localhost:11434";
    endpoints["gemini"] = "https://generativelanguage.googleapis.com/v1beta";
    endpoints["deepseek"] = "https://api.deepseek.com/v1";
    endpoints["openrouter"] = "https://openrouter.ai/api/v1";
} 