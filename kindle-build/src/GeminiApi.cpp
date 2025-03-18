#include "GeminiApi.h"
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <json/json.h>
#include <sstream>
#include <iostream>

GeminiApi::GeminiApi() 
    : cancelRequest(false) {
    // Set default endpoint
    setEndpoint("https://generativelanguage.googleapis.com/v1");
}

GeminiApi::~GeminiApi() {
    cleanupThread();
}

void GeminiApi::setApiKey(const std::string& newApiKey) {
    std::lock_guard<std::mutex> lock(mutex);
    apiKey = newApiKey;
}

void GeminiApi::setEndpoint(const std::string& newEndpoint) {
    std::lock_guard<std::mutex> lock(mutex);
    endpoint = newEndpoint;
}

std::vector<std::string> GeminiApi::getAvailableModels() {
    // Gemini has a fixed set of models
    return {"gemini-pro", "gemini-pro-vision", "gemini-1.5-pro", "gemini-1.5-flash"};
}

void GeminiApi::sendMessage(const std::string& message, const std::string& model, 
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

void GeminiApi::sendChatRequest(const std::vector<Message>& messages, 
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
            
            // Create URL with API key
            std::string url = getEndpoint() + "/models/" + model + ":generateContent?key=" + getApiKey();
            
            // Initialize curlpp
            curlpp::Cleanup cleanup;
            curlpp::Easy request;
            
            // Set URL
            request.setOpt(new curlpp::options::Url(url));
            
            // Set method and headers
            std::list<std::string> headers;
            headers.push_back("Content-Type: application/json");
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

bool GeminiApi::isConfigured() const {
    return !getApiKey().empty();
}

std::string GeminiApi::getName() const {
    return "Gemini";
}

Json::Value GeminiApi::createRequestPayload(const std::vector<Message>& messages, const std::string& model) {
    Json::Value payload;
    
    // Create contents array
    Json::Value contents(Json::arrayValue);
    
    // Convert messages to Gemini format
    for (size_t i = 0; i < messages.size(); ++i) {
        const auto& message = messages[i];
        
        Json::Value content;
        Json::Value parts(Json::arrayValue);
        Json::Value textPart;
        
        textPart["text"] = message.content;
        parts.append(textPart);
        
        content["parts"] = parts;
        
        // Map roles to Gemini roles
        if (message.role == "user") {
            content["role"] = "user";
        } else if (message.role == "assistant") {
            content["role"] = "model";
        } else if (message.role == "system") {
            // Gemini doesn't have a system role, so we'll add it as a user message
            content["role"] = "user";
        }
        
        contents.append(content);
    }
    
    payload["contents"] = contents;
    
    // Add generation config
    Json::Value generationConfig;
    generationConfig["temperature"] = 0.7;
    generationConfig["topK"] = 40;
    generationConfig["topP"] = 0.95;
    generationConfig["maxOutputTokens"] = 8192;
    
    payload["generationConfig"] = generationConfig;
    
    return payload;
}

std::string GeminiApi::performHttpRequest(const std::string& url, const std::string& jsonPayload) {
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

std::vector<std::string> GeminiApi::parseModelsResponse(const std::string& response) {
    // Not used for Gemini as we return a fixed set of models
    return {};
}

std::string GeminiApi::parseCompletionResponse(const std::string& response) {
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
        if (root.isMember("candidates") && root["candidates"].isArray() && 
            root["candidates"].size() > 0 && root["candidates"][0].isMember("content") && 
            root["candidates"][0]["content"].isMember("parts") && 
            root["candidates"][0]["content"]["parts"].isArray() && 
            root["candidates"][0]["content"]["parts"].size() > 0 && 
            root["candidates"][0]["content"]["parts"][0].isMember("text")) {
            return root["candidates"][0]["content"]["parts"][0]["text"].asString();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing completion: " << e.what() << std::endl;
    }
    
    return "Error: Could not parse response";
}

void GeminiApi::cleanupThread() {
    // Set cancel flag
    cancelRequest = true;
    
    // Join thread if it's running
    if (requestThread.joinable()) {
        requestThread.join();
    }
} 