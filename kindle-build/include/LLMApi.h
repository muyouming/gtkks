#pragma once

#include <string>
#include <vector>
#include <functional>
#include <json/json.h>

// Message structure for chat requests
struct Message {
    std::string role;
    std::string content;
};

// Base class for LLM API implementations
class LLMApi {
public:
    // Constructor and destructor
    LLMApi() = default;
    virtual ~LLMApi() = default;
    
    // API configuration
    virtual void setApiKey(const std::string& apiKey);
    virtual void setEndpoint(const std::string& endpoint);
    
    // Get API key and endpoint
    virtual std::string getApiKey() const;
    virtual std::string getEndpoint() const;
    
    // Get available models
    virtual std::vector<std::string> getAvailableModels() = 0;
    
    // Send a single message
    virtual void sendMessage(const std::string& message, const std::string& model, 
                           const std::function<void(const std::string&, bool)>& callback) = 0;
    
    // Send a chat request with multiple messages
    virtual void sendChatRequest(const std::vector<Message>& messages, 
                               const std::string& model,
                               const std::function<void(const std::string&, bool)>& callback) = 0;
    
    // Check if API is configured
    virtual bool isConfigured() const = 0;
    
    // Get API name
    virtual std::string getName() const = 0;
    
    // Cancel any ongoing requests
    virtual void cancelRequest();

private:
    std::string apiKey;
    std::string endpoint;
}; 