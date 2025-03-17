#pragma once

#include "LLMApi.h"
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>

class DeepseekApi : public LLMApi {
public:
    DeepseekApi();
    ~DeepseekApi() override;

    // LLMApi interface implementation
    void setApiKey(const std::string& apiKey) override;
    void setEndpoint(const std::string& endpoint) override;
    std::vector<std::string> getAvailableModels() override;
    void sendMessage(const std::string& message, const std::string& model, 
                   const std::function<void(const std::string&, bool)>& callback) override;
    void sendChatRequest(const std::vector<Message>& messages, 
                        const std::string& model,
                        const std::function<void(const std::string&, bool)>& callback) override;
    bool isConfigured() const override;
    std::string getName() const override;

private:
    std::string apiKey;
    std::string endpoint;
    std::mutex mutex;
    std::atomic<bool> cancelRequest;
    std::thread requestThread;

    // Helper methods
    Json::Value createRequestPayload(const std::vector<Message>& messages, const std::string& model);
    std::string performHttpRequest(const std::string& url, const std::string& jsonPayload);
    std::vector<std::string> parseModelsResponse(const std::string& response);
    std::string parseCompletionResponse(const std::string& response);
    void cleanupThread();
}; 