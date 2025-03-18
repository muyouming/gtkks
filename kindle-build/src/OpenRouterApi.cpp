#include "OpenRouterApi.h"
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <json/json.h>
#include <sstream>
#include <iostream>
#include <thread>

OpenRouterApi::OpenRouterApi() 
    : cancelRequest(false) {
    // Initialize curlpp
    curlpp::initialize();
    
    // Set default endpoint
    setEndpoint("https://openrouter.ai/api/v1");
}

OpenRouterApi::~OpenRouterApi() {
    // Cleanup thread
    cleanupThread();
    
    // Cleanup curlpp
    curlpp::terminate();
}

std::vector<std::string> OpenRouterApi::getAvailableModels() {
    std::vector<std::string> models;
    
    try {
        // Create URL - OpenRouter models endpoint
        std::string url = getEndpoint() + "/models";
        
        // Perform request
        std::string response = performHttpRequest(url, "");
        
        // Parse response
        models = parseModelsResponse(response);
    } catch (const std::exception& e) {
        std::cerr << "Error fetching models: " << e.what() << std::endl;
        // Add default models in case of error
        models.push_back("openai/gpt-3.5-turbo");
        models.push_back("openai/gpt-4");
        models.push_back("anthropic/claude-3-opus");
        models.push_back("anthropic/claude-3-sonnet");
        models.push_back("anthropic/claude-3-haiku");
        models.push_back("google/gemini-pro");
        models.push_back("meta-llama/llama-3-70b-instruct");
    }
    
    return models;
}

void OpenRouterApi::sendMessage(const std::string& message, const std::string& model, 
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

void OpenRouterApi::sendChatRequest(const std::vector<Message>& messages, 
                                   const std::string& model,
                                   const std::function<void(const std::string&, bool)>& callback) {
    // Cleanup any existing thread
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
            Json::StreamWriterBuilder writer;
            writer["indentation"] = "";
            std::string jsonPayload = Json::writeString(writer, payload);
            
            // Create URL
            std::string url = getEndpoint() + "/chat/completions";
            
            // Initialize curlpp
            curlpp::Cleanup cleanup;
            curlpp::Easy request;
            
            // Set URL
            request.setOpt(new curlpp::options::Url(url));
            
            // Set headers
            std::list<std::string> headers;
            headers.push_back("Content-Type: application/json");
            headers.push_back("Authorization: Bearer " + getApiKey());
            headers.push_back("HTTP-Referer: https://gtkks.app");
            request.setOpt(new curlpp::options::HttpHeader(headers));
            
            // Set POST data
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

bool OpenRouterApi::isConfigured() const {
    return !getApiKey().empty();
}

std::string OpenRouterApi::getName() const {
    return "OpenRouter";
}

Json::Value OpenRouterApi::createRequestPayload(const std::vector<Message>& messages, const std::string& model) {
    Json::Value root;
    Json::Value jsonMessages(Json::arrayValue);
    
    // Add messages
    for (const auto& message : messages) {
        Json::Value jsonMessage;
        jsonMessage["role"] = message.role;
        jsonMessage["content"] = message.content;
        jsonMessages.append(jsonMessage);
    }
    
    // Set model and messages
    root["model"] = model;
    root["messages"] = jsonMessages;
    
    // Add parameters
    root["temperature"] = 0.7;
    root["max_tokens"] = 2000;
    
    return root;
}

std::string OpenRouterApi::performHttpRequest(const std::string& url, const std::string& jsonPayload) {
    try {
        // Create curl request
        curlpp::Cleanup cleanup;
        curlpp::Easy request;
        
        // Set URL
        request.setOpt(new curlpp::options::Url(url));
        
        // Set headers
        std::list<std::string> headers;
        headers.push_back("Content-Type: application/json");
        headers.push_back("Authorization: Bearer " + getApiKey());
        headers.push_back("HTTP-Referer: https://gtkks.app");
        request.setOpt(new curlpp::options::HttpHeader(headers));
        
        // Set POST data if provided
        if (!jsonPayload.empty()) {
            request.setOpt(new curlpp::options::PostFields(jsonPayload));
            request.setOpt(new curlpp::options::PostFieldSize(jsonPayload.length()));
        }
        
        // Capture response
        std::ostringstream responseStream;
        request.setOpt(new curlpp::options::WriteStream(&responseStream));
        
        // Perform request
        request.perform();
        
        return responseStream.str();
        
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("HTTP request failed: ") + e.what());
    }
}

std::vector<std::string> OpenRouterApi::parseModelsResponse(const std::string& response) {
    std::vector<std::string> models;
    
    try {
        // Parse JSON
        Json::Value root;
        Json::CharReaderBuilder reader;
        std::string errors;
        std::istringstream iss(response);
        
        if (Json::parseFromStream(reader, iss, &root, &errors)) {
            // OpenRouter API returns models in a 'data' array
            if (root.isMember("data") && root["data"].isArray()) {
                for (const auto& model : root["data"]) {
                    if (model.isMember("id") && model["id"].isString()) {
                        models.push_back(model["id"].asString());
                    }
                }
            } else if (root.isArray()) {
                // Fallback to the original implementation for backward compatibility
                for (const auto& model : root) {
                    if (model.isMember("id") && model["id"].isString()) {
                        models.push_back(model["id"].asString());
                    }
                }
            }
            
            // If no models were found, add some default OpenRouter models
            if (models.empty()) {
                models.push_back("openai/gpt-3.5-turbo");
                models.push_back("openai/gpt-4");
                models.push_back("anthropic/claude-3-opus");
                models.push_back("anthropic/claude-3-sonnet");
                models.push_back("anthropic/claude-3-haiku");
                models.push_back("google/gemini-pro");
                models.push_back("meta-llama/llama-3-70b-instruct");
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing models response: " << e.what() << std::endl;
        // Add default models in case of error
        models.push_back("openai/gpt-3.5-turbo");
        models.push_back("openai/gpt-4");
        models.push_back("anthropic/claude-3-opus");
        models.push_back("anthropic/claude-3-sonnet");
        models.push_back("anthropic/claude-3-haiku");
        models.push_back("google/gemini-pro");
        models.push_back("meta-llama/llama-3-70b-instruct");
    }
    
    return models;
}

std::string OpenRouterApi::parseCompletionResponse(const std::string& response) {
    try {
        // Parse JSON
        Json::Value root;
        Json::CharReaderBuilder reader;
        std::string errors;
        std::istringstream iss(response);
        
        if (Json::parseFromStream(reader, iss, &root, &errors)) {
            // Extract response
            if (root.isMember("choices") && root["choices"].isArray() && 
                root["choices"].size() > 0 && root["choices"][0].isMember("message") && 
                root["choices"][0]["message"].isMember("content")) {
                return root["choices"][0]["message"]["content"].asString();
            }
            
            // Check for errors
            if (root.isMember("error") && root["error"].isMember("message")) {
                return "Error: " + root["error"]["message"].asString();
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing completion response: " << e.what() << std::endl;
    }
    
    return "Error: Could not parse response";
}

void OpenRouterApi::cleanupThread() {
    // Cancel any ongoing request
    cancelRequest = true;
    
    // Wait for thread to finish
    if (requestThread.joinable()) {
        requestThread.join();
    }
}

void OpenRouterApi::setApiKey(const std::string& apiKey) {
    LLMApi::setApiKey(apiKey);
}

void OpenRouterApi::setEndpoint(const std::string& endpoint) {
    LLMApi::setEndpoint(endpoint);
} 