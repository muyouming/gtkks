#include "ApiManager.h"
#include "OllamaApi.h"
#include "OpenAIApi.h"
#include "GeminiApi.h"
#include "DeepseekApi.h"
#include "OpenRouterApi.h"
#include "Config.h"
#include <iostream>

ApiManager::ApiManager() {
    // Initialize APIs
    initApis();
    
    // Load API keys and endpoints from config
    Config& config = Config::getInstance();
    
    // Apply configuration to APIs
    for (const auto& [name, api] : apis) {
        // Set API key
        std::string apiKey = config.getApiKey(name);
        if (!apiKey.empty()) {
            api->setApiKey(apiKey);
        }
        
        // Set endpoint
        std::string endpoint = config.getEndpoint(name);
        if (!endpoint.empty()) {
            api->setEndpoint(endpoint);
        }
    }
}

ApiManager::~ApiManager() {
    // Nothing to do here, Config class handles saving
}

ApiManager& ApiManager::getInstance() {
    static ApiManager instance;
    return instance;
}

std::shared_ptr<LLMApi> ApiManager::getApi(const std::string& name) {
    auto it = apis.find(name);
    if (it != apis.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::string> ApiManager::getAvailableApis() const {
    std::vector<std::string> result;
    for (const auto& pair : apis) {
        result.push_back(pair.first);
    }
    return result;
}

void ApiManager::setApiKey(const std::string& apiName, const std::string& apiKey) {
    auto api = getApi(apiName);
    if (api) {
        api->setApiKey(apiKey);
        
        // Save to config
        Config::getInstance().setApiKey(apiName, apiKey);
        Config::getInstance().save();
    }
}

void ApiManager::setEndpoint(const std::string& apiName, const std::string& endpoint) {
    auto api = getApi(apiName);
    if (api) {
        api->setEndpoint(endpoint);
        
        // Save to config
        Config::getInstance().setEndpoint(apiName, endpoint);
        Config::getInstance().save();
    }
}

std::pair<std::string, std::string> ApiManager::getLastUsedModel() const {
    return Config::getInstance().getLastUsedModel();
}

void ApiManager::setLastUsedModel(const std::string& api, const std::string& model) {
    Config::getInstance().setLastUsedModel(api, model);
    Config::getInstance().save();
}

void ApiManager::initApis() {
    // Create API instances
    apis["Ollama"] = std::make_shared<OllamaApi>();
    apis["OpenAI"] = std::make_shared<OpenAIApi>();
    apis["Gemini"] = std::make_shared<GeminiApi>();
    apis["Deepseek"] = std::make_shared<DeepseekApi>();
    apis["OpenRouter"] = std::make_shared<OpenRouterApi>();
    
    // Set default endpoints from Config
    Config& config = Config::getInstance();
    for (const auto& [name, api] : apis) {
        std::string endpoint = config.getEndpoint(name);
        if (!endpoint.empty()) {
            api->setEndpoint(endpoint);
        }
    }
} 