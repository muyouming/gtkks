#pragma once

#include "LLMApi.h"
#include "HttpClient.h"
#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include <thread>
#include <atomic>

class OpenAIApi : public LLMApi {
public:
    OpenAIApi();
    virtual ~OpenAIApi();

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
    std::string getName() const override {
        return "OpenAI";
    }

    // Override base class methods
    virtual void sendMessage(const std::string& message, const std::string& model, 
                            const std::function<void(const std::string&, bool)>& callback) override;

    // Cancel ongoing requests
    void cancelRequest() override;

private:
    // HTTP client
    HttpClient httpClient;
    
    // Available models
    std::vector<std::string> availableModels;
    
    // Helper methods
    SimpleJson createRequestPayload(const std::vector<Message>& messages, const std::string& model);
    std::string performHttpRequest(const std::string& url, const std::string& jsonPayload);
    std::vector<std::string> parseModelsResponse(const std::string& response);
    std::string parseCompletionResponse(const std::string& response);

    // Thread management
    std::mutex threadMutex;
    std::atomic<bool> cancelRequestFlag;
    std::thread requestThread;
    
    void cleanupThread();
}; 