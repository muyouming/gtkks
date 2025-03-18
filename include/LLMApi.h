#pragma once

#include <string>
#include <vector>
#include <functional>
#include <map>

// Message structure for chat requests
struct Message {
    std::string role;
    std::string content;
};

// Simple JSON structure implementation using standard C++
class SimpleJson {
public:
    enum Type { Null, Boolean, Number, String, Array, Object };
    
    SimpleJson() : type(Null) {}
    SimpleJson(bool value) : type(Boolean), boolValue(value) {}
    SimpleJson(double value) : type(Number), numberValue(value) {}
    SimpleJson(const std::string& value) : type(String), stringValue(value) {}
    
    Type getType() const { return type; }
    
    bool asBool() const { return boolValue; }
    double asNumber() const { return numberValue; }
    std::string asString() const { return stringValue; }
    
    void addToArray(const SimpleJson& value) {
        if (type != Array) {
            type = Array;
            arrayValues.clear();
        }
        arrayValues.push_back(value);
    }
    
    void addToObject(const std::string& key, const SimpleJson& value) {
        if (type != Object) {
            type = Object;
            objectValues.clear();
        }
        objectValues[key] = value;
    }
    
    bool hasKey(const std::string& key) const {
        return type == Object && objectValues.find(key) != objectValues.end();
    }
    
    const SimpleJson& operator[](const std::string& key) const {
        static SimpleJson nullValue;
        if (type != Object) return nullValue;
        auto it = objectValues.find(key);
        if (it != objectValues.end()) return it->second;
        return nullValue;
    }
    
    std::string toJsonString() const;
    
private:
    Type type;
    bool boolValue = false;
    double numberValue = 0.0;
    std::string stringValue;
    std::vector<SimpleJson> arrayValues;
    std::map<std::string, SimpleJson> objectValues;
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