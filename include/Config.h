#pragma once

#include <string>
#include <map>
#include <utility>

class Config {
public:
    // Get singleton instance
    static Config& getInstance();

    // Load configuration from file
    void load();
    
    // Save configuration to file
    void save();
    
    // Get API key for a specific service
    std::string getApiKey(const std::string& apiName) const;
    
    // Set API key for a specific service
    void setApiKey(const std::string& apiName, const std::string& apiKey);
    
    // Get endpoint for a specific service
    std::string getEndpoint(const std::string& apiName) const;
    
    // Set endpoint for a specific service
    void setEndpoint(const std::string& apiName, const std::string& endpoint);
    
    // Get last used API and model
    std::pair<std::string, std::string> getLastUsedModel() const;
    
    // Set last used API and model
    void setLastUsedModel(const std::string& api, const std::string& model);

private:
    // Private constructor for singleton
    Config();
    
    // Delete copy constructor and assignment operator
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    
    // Configuration data
    std::map<std::string, std::string> apiKeys;
    std::map<std::string, std::string> endpoints;
    std::string lastUsedApi;
    std::string lastUsedModel;
    
    // Configuration file path
    std::string getConfigPath() const;
    
    // Initialize default endpoints
    void initDefaultEndpoints();
}; 