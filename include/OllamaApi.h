#pragma once

#include "LLMApi.h"
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>

class OllamaApi : public LLMApi {
public:
    OllamaApi();
    ~OllamaApi() override;

    // LLMApi interface implementation
    std::vector<std::string> getAvailableModels() override;
    void sendMessage(const std::string& message, const std::string& model, 
                    const std::function<void(const std::string&, bool)>& callback) override;
    void sendChatRequest(const std::vector<Message>& messages, 
                        const std::string& model,
                        const std::function<void(const std::string&, bool)>& callback) override;
    bool isConfigured() const override;
    std::string getName() const override;

private:
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