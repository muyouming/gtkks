#include "DeepseekApi.h"
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <json/json.h>
#include <sstream>
#include <iostream>

DeepseekApi::DeepseekApi() 
    : cancelRequest(false) {
    // Set default endpoint
    setEndpoint("https://api.deepseek.com/v1");
}

DeepseekApi::~DeepseekApi() {
    cleanupThread();
}

void DeepseekApi::setApiKey(const std::string& newApiKey) {
    std::lock_guard<std::mutex> lock(mutex);
    apiKey = newApiKey;
}

void DeepseekApi::setEndpoint(const std::string& newEndpoint) {
    std::lock_guard<std::mutex> lock(mutex);
    endpoint = newEndpoint;
}

std::vector<std::string> DeepseekApi::getAvailableModels() {
    // Deepseek has a fixed set of models
    return {"deepseek-chat", "deepseek-coder"};
}

void DeepseekApi::sendMessage(const std::string& message, const std::string& model, 
                            const std::function<void(const std::string&, bool)>& callback) {
    // Create a single message
    std::vector<Message> messages;
    Message userMessage;
    userMessage.role = "user";
    userMessage.content = message;
    messages.push_back(userMessage);
    
    // Send the message
    sendChatRequest(messages, model, callback);
}

void DeepseekApi::sendChatRequest(const std::vector<Message>& messages, 
                                 const std::string& model,
                                 const std::function<void(const std::string&, bool)>& callback) {
    // Clean up any previous request thread
    cleanupThread();
    
    // Reset cancel flag
    cancelRequest = false;
    
    // Create a new thread for the request
    requestThread = std::thread([this, messages, model, callback]() {
        try {
            // Check if API key is set
            if (getApiKey().empty()) {
                callback("Error: API key not set", true);
                return;
            }
            
            // Create request payload
            Json::Value payload = createRequestPayload(messages, model);
            
            // Convert to string
            Json::StreamWriterBuilder builder;
            builder["indentation"] = "";
            std::string jsonPayload = Json::writeString(builder, payload);
            
            // Create URL
            std::string url = getEndpoint() + "/chat/completions";
            
            // Initialize curlpp
            curlpp::Cleanup cleanup;
            curlpp::Easy request;
            
            // Set URL
            request.setOpt(new curlpp::options::Url(url));
            
            // Set method and headers
            std::list<std::string> headers;
            headers.push_back("Content-Type: application/json");
            headers.push_back("Authorization: Bearer " + getApiKey());
            request.setOpt(new curlpp::options::HttpHeader(headers));
            request.setOpt(new curlpp::options::Post(true));
            request.setOpt(new curlpp::options::PostFields(jsonPayload));
            request.setOpt(new curlpp::options::PostFieldSize(jsonPayload.length()));
            
            // Set up response handling
            std::ostringstream responseStream;
            request.setOpt(new curlpp::options::WriteStream(&responseStream));
            
            // Perform request
            request.perform();
            
            // Parse response
            std::string response = responseStream.str();
            std::string content = parseCompletionResponse(response);
            
            // Call callback with response
            if (!cancelRequest) {
                callback(content, true);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error sending chat request: " << e.what() << std::endl;
            if (!cancelRequest) {
                callback("Error: " + std::string(e.what()), true);
            }
        }
    });
}

bool DeepseekApi::isConfigured() const {
    return !getApiKey().empty();
}

std::string DeepseekApi::getName() const {
    return "Deepseek";
}

Json::Value DeepseekApi::createRequestPayload(const std::vector<Message>& messages, const std::string& model) {
    Json::Value payload;
    
    // Set model
    payload["model"] = model;
    
    // Add messages
    Json::Value jsonMessages(Json::arrayValue);
    for (const auto& message : messages) {
        Json::Value jsonMessage;
        jsonMessage["role"] = message.role;
        jsonMessage["content"] = message.content;
        jsonMessages.append(jsonMessage);
    }
    payload["messages"] = jsonMessages;
    
    // Add parameters
    payload["temperature"] = 0.7;
    payload["max_tokens"] = 4000;
    payload["stream"] = false;
    
    return payload;
}

std::string DeepseekApi::performHttpRequest(const std::string& url, const std::string& jsonPayload) {
    try {
        // Initialize curlpp
        curlpp::Cleanup cleanup;
        curlpp::Easy request;
        
        // Set URL
        request.setOpt(new curlpp::options::Url(url));
        
        // Set method and headers if needed
        if (!jsonPayload.empty()) {
            std::list<std::string> headers;
            headers.push_back("Content-Type: application/json");
            headers.push_back("Authorization: Bearer " + getApiKey());
            request.setOpt(new curlpp::options::HttpHeader(headers));
            request.setOpt(new curlpp::options::Post(true));
            request.setOpt(new curlpp::options::PostFields(jsonPayload));
            request.setOpt(new curlpp::options::PostFieldSize(jsonPayload.length()));
        }
        
        // Set up response handling
        std::ostringstream responseStream;
        request.setOpt(new curlpp::options::WriteStream(&responseStream));
        
        // Perform request
        request.perform();
        
        // Return response
        return responseStream.str();
    } catch (const std::exception& e) {
        std::cerr << "HTTP request error: " << e.what() << std::endl;
        throw;
    }
}

std::vector<std::string> DeepseekApi::parseModelsResponse(const std::string& response) {
    // Not used for Deepseek as we return a fixed set of models
    return {};
}

std::string DeepseekApi::parseCompletionResponse(const std::string& response) {
    try {
        // Parse JSON
        Json::Value root;
        Json::CharReaderBuilder builder;
        std::string errs;
        std::istringstream responseStream(response);
        if (!Json::parseFromStream(builder, responseStream, &root, &errs)) {
            std::cerr << "Failed to parse completion response: " << errs << std::endl;
            return "Error parsing response";
        }
        
        // Check for errors
        if (root.isMember("error")) {
            if (root["error"].isMember("message")) {
                return "Error: " + root["error"]["message"].asString();
            }
            return "Unknown API error";
        }
        
        // Extract message content
        if (root.isMember("choices") && root["choices"].isArray() && 
            root["choices"].size() > 0 && root["choices"][0].isMember("message") && 
            root["choices"][0]["message"].isMember("content")) {
            return root["choices"][0]["message"]["content"].asString();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing completion: " << e.what() << std::endl;
    }
    
    return "Error: Could not parse response";
}

void DeepseekApi::cleanupThread() {
    // Set cancel flag
    cancelRequest = true;
    
    // Join thread if it's running
    if (requestThread.joinable()) {
        requestThread.join();
    }
} 