#pragma once

#include "LLMApi.h"
#include "Config.h"
#include <memory>
#include <map>
#include <string>

class ApiManager {
public:
    ApiManager();
    ~ApiManager();

    // Get instance of the API manager (singleton)
    static ApiManager& getInstance();

    // Get API by name
    std::shared_ptr<LLMApi> getApi(const std::string& name);

    // Get all available APIs
    std::vector<std::string> getAvailableApis() const;

    // Set API key for a specific API
    void setApiKey(const std::string& apiName, const std::string& apiKey);

    // Set endpoint for a specific API
    void setEndpoint(const std::string& apiName, const std::string& endpoint);

    // Get last used API and model
    std::pair<std::string, std::string> getLastUsedModel() const;
    
    // Set last used API and model
    void setLastUsedModel(const std::string& api, const std::string& model);

private:
    // Map of API name to API instance
    std::map<std::string, std::shared_ptr<LLMApi>> apis;

    // Initialize APIs
    void initApis();
}; 