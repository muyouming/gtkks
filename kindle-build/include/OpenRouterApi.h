#pragma once

#include "LLMApi.h"
#include <string>
#include <vector>
#include <functional>
#include <json/json.h>
#include <curlpp/cURLpp.hpp>
#include <mutex>
#include <thread>
#include <atomic>

class OpenRouterApi : public LLMApi {
public:
    OpenRouterApi();
    virtual ~OpenRouterApi();

    // Set API key
    void setApiKey(const std::string& apiKey) override;
    
    // Set API endpoint
    void setEndpoint(const std::string& endpoint) override;
    
    // Get available models
    std::vector<std::string> getAvailableModels() override;
    
    // Send a chat completion request
    void sendChatRequest(const std::vector<Message>& messages, 
                        const std::string& model,
                        const std::function<void(const std::string&, bool)>& callback) override;
    
    // Check if API is properly configured
    bool isConfigured() const override;
    
    // Get API name
    std::string getName() const override;

    // Override base class methods
    virtual void sendMessage(const std::string& message, const std::string& model, 
                            const std::function<void(const std::string&, bool)>& callback) override;

private:
    std::string apiKey;
    std::string endpoint;
    std::vector<std::string> availableModels;
    
    // Helper methods
    Json::Value createRequestPayload(const std::vector<Message>& messages, const std::string& model);
    std::string performHttpRequest(const std::string& url, const std::string& jsonPayload);
    std::vector<std::string> parseModelsResponse(const std::string& response);
    std::string parseCompletionResponse(const std::string& response);

    // Thread management
    std::mutex mutex;
    std::atomic<bool> cancelRequest;
    std::thread requestThread;
    
    void cleanupThread();
}; 