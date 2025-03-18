#include "OpenAIApi.h"
#include <sstream>
#include <iostream>
#include <regex>

OpenAIApi::OpenAIApi() : cancelRequestFlag(false) {
    // Set default endpoint
    setEndpoint("https://api.openai.com/v1");
}

OpenAIApi::~OpenAIApi() {
    // Cleanup thread
    cleanupThread();
}

void OpenAIApi::setApiKey(const std::string& apiKey) {
    LLMApi::setApiKey(apiKey);
}

void OpenAIApi::setEndpoint(const std::string& endpoint) {
    LLMApi::setEndpoint(endpoint);
}

std::vector<std::string> OpenAIApi::getAvailableModels() {
    if (!availableModels.empty()) {
        return availableModels;
    }

    try {
        // Create URL
        std::string url = getEndpoint() + "/models";
        
        // Set headers
        httpClient.clearHeaders();
        httpClient.setHeader("Authorization", "Bearer " + getApiKey());
        
        // Perform request
        std::string response = httpClient.get(url);
        
        // Parse response
        availableModels = parseModelsResponse(response);
    } catch (const std::exception& e) {
        std::cerr << "Error fetching models: " << e.what() << std::endl;
    }
    
    return availableModels;
}

void OpenAIApi::sendMessage(const std::string& message, const std::string& model, 
                          const std::function<void(const std::string&, bool)>& callback) {
    // Create a message vector with a single user message
    std::vector<Message> messages;
    Message userMessage;
    userMessage.role = "user";
    userMessage.content = message;
    messages.push_back(userMessage);
    
    // Call the chat request method
    sendChatRequest(messages, model, callback);
}

void OpenAIApi::sendChatRequest(const std::vector<Message>& messages, 
                             const std::string& model,
                             const std::function<void(const std::string&, bool)>& callback) {
    // Clean up previous request
    cleanupThread();
    
    // Reset cancel flag
    cancelRequestFlag = false;
    
    // Create a new thread for the request
    requestThread = std::thread([this, messages, model, callback]() {
        try {
            // Create URL
            std::string url = getEndpoint() + "/chat/completions";
            
            // Create payload
            SimpleJson payload = createRequestPayload(messages, model);
            
            // Convert to string
            std::string jsonPayload = payload.toJsonString();
            
            // Set headers
            httpClient.clearHeaders();
            httpClient.setHeader("Content-Type", "application/json");
            httpClient.setHeader("Authorization", "Bearer " + getApiKey());
            
            // Perform request
            std::string response = httpClient.post(url, jsonPayload);
            
            // Extract the content from the response
            std::string content = parseCompletionResponse(response);
            
            // Send the response to the callback
            callback(content, true);
            
        } catch (const std::exception& e) {
            // Handle errors
            callback("Error: " + std::string(e.what()), true);
        }
    });
}

std::string OpenAIApi::performHttpRequest(const std::string& url, const std::string& jsonPayload) {
    try {
        // Set content type if posting data
        httpClient.clearHeaders();
        httpClient.setHeader("Content-Type", "application/json");
        httpClient.setHeader("Authorization", "Bearer " + getApiKey());
        
        if (!jsonPayload.empty()) {
            return httpClient.post(url, jsonPayload);
        } else {
            return httpClient.get(url);
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("HTTP request failed: " + std::string(e.what()));
    }
}

SimpleJson OpenAIApi::createRequestPayload(const std::vector<Message>& messages, const std::string& model) {
    SimpleJson payload;
    
    // Add model
    payload.addToObject("model", SimpleJson(model));
    
    // Add messages
    SimpleJson jsonMessages;
    
    for (const auto& message : messages) {
        SimpleJson jsonMessage;
        jsonMessage.addToObject("role", SimpleJson(message.role));
        jsonMessage.addToObject("content", SimpleJson(message.content));
        
        jsonMessages.addToArray(jsonMessage);
    }
    
    payload.addToObject("messages", jsonMessages);
    
    // Add other parameters
    payload.addToObject("temperature", SimpleJson(0.7));
    payload.addToObject("max_tokens", SimpleJson(1000.0));
    
    return payload;
}

std::vector<std::string> OpenAIApi::parseModelsResponse(const std::string& response) {
    std::vector<std::string> models;
    
    try {
        // Simple parsing - look for model IDs in the response
        std::regex modelRegex("\"id\"\\s*:\\s*\"([^\"]+)\"");
        
        auto begin = std::sregex_iterator(response.begin(), response.end(), modelRegex);
        auto end = std::sregex_iterator();
        
        for (std::sregex_iterator i = begin; i != end; ++i) {
            std::smatch match = *i;
            if (match.size() > 1) {
                std::string modelId = match[1].str();
                // Only include chat models
                if (modelId.find("gpt") != std::string::npos) {
                    models.push_back(modelId);
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing models: " << e.what() << std::endl;
    }
    
    return models;
}

std::string OpenAIApi::parseCompletionResponse(const std::string& response) {
    try {
        // Simple parsing - look for the content in the response
        std::regex contentRegex("\"content\"\\s*:\\s*\"((?:\\\\.|[^\"])*?)\"");
        std::smatch match;
        
        if (std::regex_search(response, match, contentRegex) && match.size() > 1) {
            std::string content = match[1].str();
            
            // Unescape JSON string
            std::string unescaped;
            for (size_t i = 0; i < content.length(); ++i) {
                if (content[i] == '\\' && i + 1 < content.length()) {
                    if (content[i + 1] == 'n') {
                        unescaped += '\n';
                    } else if (content[i + 1] == 'r') {
                        unescaped += '\r';
                    } else if (content[i + 1] == 't') {
                        unescaped += '\t';
                    } else if (content[i + 1] == '\"') {
                        unescaped += '\"';
                    } else if (content[i + 1] == '\\') {
                        unescaped += '\\';
                    } else {
                        unescaped += content[i + 1];
                    }
                    i++; // Skip the escaped character
                } else {
                    unescaped += content[i];
                }
            }
            
            return unescaped;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing completion response: " << e.what() << std::endl;
    }
    
    return "";
}

bool OpenAIApi::isConfigured() const {
    return !getApiKey().empty() && !getEndpoint().empty();
}

void OpenAIApi::cancelRequest() {
    cancelRequestFlag = true;
    httpClient.cancelRequest();
}

void OpenAIApi::cleanupThread() {
    if (requestThread.joinable()) {
        {
            std::lock_guard<std::mutex> lock(threadMutex);
            cancelRequestFlag = true;
        }
        
        httpClient.cancelRequest();
        requestThread.join();
    }
} 