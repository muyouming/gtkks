#include "OllamaApi.h"
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <json/json.h>
#include <sstream>
#include <iostream>

OllamaApi::OllamaApi() : cancelRequest(false) {
    // Initialize curlpp
    curlpp::initialize();
    
    // Set default endpoint
    setEndpoint("http://localhost:11434");
}

OllamaApi::~OllamaApi() {
    // Cleanup thread
    cleanupThread();
    
    // Cleanup curlpp
    curlpp::terminate();
}

std::vector<std::string> OllamaApi::getAvailableModels() {
    std::vector<std::string> models;
    
    try {
        // Create URL
        std::string url = getEndpoint() + "/api/tags";
        
        // Perform request
        std::string response = performHttpRequest(url, "");
        
        // Parse response
        models = parseModelsResponse(response);
    } catch (const std::exception& e) {
        std::cerr << "Error fetching models: " << e.what() << std::endl;
    }
    
    return models;
}

void OllamaApi::sendMessage(const std::string& message, const std::string& model, 
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

void OllamaApi::sendChatRequest(const std::vector<Message>& messages, 
                               const std::string& model,
                               const std::function<void(const std::string&, bool)>& callback) {
    // Cleanup any existing thread
    cleanupThread();
    
    // Reset cancel flag
    cancelRequest = false;
    
    // Create a new thread for the request
    requestThread = std::thread([this, messages, model, callback]() {
        try {
            // Create URL
            std::string url = getEndpoint() + "/api/chat";
            
            // Create request payload
            Json::Value payload = createRequestPayload(messages, model);
            
            // Convert to string
            Json::StreamWriterBuilder writer;
            writer["indentation"] = "";
            std::string jsonPayload = Json::writeString(writer, payload);
            
            // Set up curl request
            curlpp::Easy request;
            
            // Set URL
            request.setOpt(new curlpp::options::Url(url));
            
            // Set headers
            std::list<std::string> headers;
            headers.push_back("Content-Type: application/json");
            request.setOpt(new curlpp::options::HttpHeader(headers));
            
            // Set POST data
            request.setOpt(new curlpp::options::PostFields(jsonPayload));
            request.setOpt(new curlpp::options::PostFieldSize(jsonPayload.length()));
            
            // Set up write function to handle streaming
            std::string buffer;
            request.setOpt(new curlpp::options::WriteFunction([&buffer, &callback, this](char* ptr, size_t size, size_t nmemb) -> size_t {
                // Check if request was cancelled
                if (cancelRequest) {
                    return 0;
                }
                
                size_t totalSize = size * nmemb;
                std::string chunk(ptr, totalSize);
                buffer += chunk;
                
                // Process buffer line by line
                size_t pos = 0;
                while ((pos = buffer.find("\n")) != std::string::npos) {
                    std::string line = buffer.substr(0, pos);
                    buffer.erase(0, pos + 1);
                    
                    // Parse JSON
                    Json::Value root;
                    Json::CharReaderBuilder reader;
                    std::string errors;
                    std::istringstream iss(line);
                    
                    if (Json::parseFromStream(reader, iss, &root, &errors)) {
                        // Check if this is a done message
                        if (root.isMember("done") && root["done"].asBool()) {
                            callback("", true);
                            return totalSize;
                        }
                        
                        // Extract content
                        if (root.isMember("message") && 
                            root["message"].isMember("content")) {
                            std::string content = root["message"]["content"].asString();
                            callback(content, false);
                        }
                    }
                }
                
                return totalSize;
            }));
            
            // Perform request
            request.perform();
            
        } catch (const std::exception& e) {
            // Handle errors
            callback("Error: " + std::string(e.what()), true);
        }
    });
}

bool OllamaApi::isConfigured() const {
    // Ollama doesn't require an API key, just a valid endpoint
    return !getEndpoint().empty();
}

std::string OllamaApi::getName() const {
    return "Ollama";
}

Json::Value OllamaApi::createRequestPayload(const std::vector<Message>& messages, const std::string& model) {
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
    root["stream"] = true;
    
    return root;
}

std::string OllamaApi::performHttpRequest(const std::string& url, const std::string& jsonPayload) {
    try {
        // Create curl request
        curlpp::Easy request;
        
        // Set URL
        request.setOpt(new curlpp::options::Url(url));
        
        // Set headers
        std::list<std::string> headers;
        headers.push_back("Content-Type: application/json");
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

std::vector<std::string> OllamaApi::parseModelsResponse(const std::string& response) {
    std::vector<std::string> models;
    
    try {
        // Parse JSON
        Json::Value root;
        Json::CharReaderBuilder reader;
        std::string errors;
        std::istringstream iss(response);
        
        if (Json::parseFromStream(reader, iss, &root, &errors)) {
            // Extract models
            if (root.isMember("models") && root["models"].isArray()) {
                for (const auto& model : root["models"]) {
                    if (model.isMember("name") && model["name"].isString()) {
                        models.push_back(model["name"].asString());
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing models response: " << e.what() << std::endl;
    }
    
    return models;
}

std::string OllamaApi::parseCompletionResponse(const std::string& response) {
    try {
        // Parse JSON
        Json::Value root;
        Json::CharReaderBuilder reader;
        std::string errors;
        std::istringstream iss(response);
        
        if (Json::parseFromStream(reader, iss, &root, &errors)) {
            // Extract response
            if (root.isMember("response") && root["response"].isString()) {
                return root["response"].asString();
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing completion response: " << e.what() << std::endl;
    }
    
    return "";
}

void OllamaApi::cleanupThread() {
    // Cancel any ongoing request
    cancelRequest = true;
    
    // Wait for thread to finish
    if (requestThread.joinable()) {
        requestThread.join();
    }
} 