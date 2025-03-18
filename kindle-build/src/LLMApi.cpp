#include "LLMApi.h"

void LLMApi::setApiKey(const std::string& apiKey) {
    this->apiKey = apiKey;
}

void LLMApi::setEndpoint(const std::string& endpoint) {
    this->endpoint = endpoint;
}

std::string LLMApi::getApiKey() const {
    return apiKey;
}

std::string LLMApi::getEndpoint() const {
    return endpoint;
}

void LLMApi::sendChatRequest(const std::vector<Message>& messages, 
                           const std::string& model,
                           const std::function<void(const std::string&, bool)>& callback) {
    // Default implementation: extract the last user message and send it
    std::string lastUserMessage;
    
    // Find the last user message
    for (auto it = messages.rbegin(); it != messages.rend(); ++it) {
        if (it->role == "user") {
            lastUserMessage = it->content;
            break;
        }
    }
    
    // If no user message found, return an error
    if (lastUserMessage.empty()) {
        callback("Error: No user message found", true);
        return;
    }
    
    // Send the message
    sendMessage(lastUserMessage, model, callback);
}

void LLMApi::cancelRequest() {
    // Base implementation does nothing
    // Derived classes should override this if they support cancellation
} 