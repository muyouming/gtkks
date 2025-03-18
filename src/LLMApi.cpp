#include "LLMApi.h"
#include <sstream>

std::string SimpleJson::toJsonString() const {
    std::stringstream ss;
    
    switch (type) {
        case Null:
            ss << "null";
            break;
        case Boolean:
            ss << (boolValue ? "true" : "false");
            break;
        case Number:
            ss << numberValue;
            break;
        case String:
            ss << "\"";
            // Escape special characters
            for (char c : stringValue) {
                if (c == '\"' || c == '\\') {
                    ss << '\\';
                }
                ss << c;
            }
            ss << "\"";
            break;
        case Array:
            ss << "[";
            for (size_t i = 0; i < arrayValues.size(); ++i) {
                if (i > 0) ss << ",";
                ss << arrayValues[i].toJsonString();
            }
            ss << "]";
            break;
        case Object:
            ss << "{";
            {
                bool first = true;
                for (const auto& [key, value] : objectValues) {
                    if (!first) ss << ",";
                    first = false;
                    ss << "\"" << key << "\":" << value.toJsonString();
                }
            }
            ss << "}";
            break;
    }
    
    return ss.str();
}

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