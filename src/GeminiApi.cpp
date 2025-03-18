#include "GeminiApi.h"
#include <sstream>
#include <iostream>
#include <regex>

GeminiApi::GeminiApi() : cancelRequestFlag(false) {
    // Set default endpoint
    setEndpoint("https://generativelanguage.googleapis.com/v1beta");
}

GeminiApi::~GeminiApi() {
    // Cleanup thread
    cleanupThread();
}

void GeminiApi::setApiKey(const std::string& apiKey) {
    LLMApi::setApiKey(apiKey);
}

void GeminiApi::setEndpoint(const std::string& endpoint) {
    LLMApi::setEndpoint(endpoint);
}

std::vector<std::string> GeminiApi::getAvailableModels() {
    if (!availableModels.empty()) {
        return availableModels;
    }

    try {
        if (getApiKey().empty()) {
            throw std::runtime_error("API key not set");
        }
        
        // Try to fetch models from API if needed
        std::string url = getEndpoint() + "/models?key=" + getApiKey();
        
        try {
            // Set headers
            httpClient.clearHeaders();
            
            // Perform request with a timeout
            std::string response = httpClient.get(url);
            
            // Parse response
            std::regex modelRegex("\"name\"\\s*:\\s*\"([^\"]+)\"");
            std::smatch match;
            std::string::const_iterator searchStart(response.cbegin());
            
            while (std::regex_search(searchStart, response.cend(), match, modelRegex)) {
                if (match.size() > 1) {
                    // Extract model name
                    std::string fullName = match[1].str();
                    // The API returns full paths like "models/gemini-pro", extract just the model name
                    size_t lastSlash = fullName.find_last_of('/');
                    if (lastSlash != std::string::npos) {
                        availableModels.push_back(fullName.substr(lastSlash + 1));
                    } else {
                        availableModels.push_back(fullName);
                    }
                }
                searchStart = match.suffix().first;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error fetching models from API: " << e.what() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in getAvailableModels: " << e.what() << std::endl;
    }
    
    // If no models were found or there was an error, use defaults
    if (availableModels.empty()) {
        // Gemini models are predefined
        availableModels.push_back("gemini-pro");
        availableModels.push_back("gemini-pro-vision");
    }
    
    return availableModels;
}

void GeminiApi::sendMessage(const std::string& message, const std::string& model, 
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

void GeminiApi::sendChatRequest(const std::vector<Message>& messages, 
                              const std::string& model,
                              const std::function<void(const std::string&, bool)>& callback) {
    // Clean up previous request
    cleanupThread();
    
    // Reset cancel flag
    cancelRequestFlag = false;
    
    // Create a new thread for the request
    requestThread = std::thread([this, messages, model, callback]() {
        try {
            // Create URL with API key
            std::string url = getEndpoint() + "/models/" + model + ":generateContent?key=" + getApiKey();
            
            // Create payload
            SimpleJson payload = createRequestPayload(messages, model);
            
            // Convert to string
            std::string jsonPayload = payload.toJsonString();
            
            // Set headers
            httpClient.clearHeaders();
            httpClient.setHeader("Content-Type", "application/json");
            
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

std::string GeminiApi::performHttpRequest(const std::string& url, const std::string& jsonPayload) {
    try {
        // Set content type if posting data
        httpClient.clearHeaders();
        httpClient.setHeader("Content-Type", "application/json");
        
        if (!jsonPayload.empty()) {
            return httpClient.post(url, jsonPayload);
        } else {
            return httpClient.get(url);
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("HTTP request failed: " + std::string(e.what()));
    }
}

SimpleJson GeminiApi::createRequestPayload(const std::vector<Message>& messages, const std::string& model) {
    SimpleJson payload;
    
    // Create contents array
    SimpleJson contents;
    
    // Convert messages to Gemini format
    // In Gemini API, we need to group consecutive messages by the same role
    std::string currentRole = "";
    SimpleJson currentParts;
    
    for (const auto& message : messages) {
        if (message.role != currentRole && !currentRole.empty()) {
            // Add the previous message to contents
            SimpleJson content;
            content.addToObject("role", SimpleJson(currentRole == "user" ? "user" : "model"));
            content.addToObject("parts", currentParts);
            contents.addToArray(content);
            
            // Reset for next message
            currentParts = SimpleJson();
        }
        
        // Set current role
        currentRole = message.role;
        
        // Add part
        SimpleJson part;
        part.addToObject("text", SimpleJson(message.content));
        currentParts.addToArray(part);
    }
    
    // Add the last message
    if (!currentRole.empty()) {
        SimpleJson content;
        content.addToObject("role", SimpleJson(currentRole == "user" ? "user" : "model"));
        content.addToObject("parts", currentParts);
        contents.addToArray(content);
    }
    
    payload.addToObject("contents", contents);
    
    // Add generation config
    SimpleJson generationConfig;
    generationConfig.addToObject("temperature", SimpleJson(0.7));
    generationConfig.addToObject("maxOutputTokens", SimpleJson(800.0));
    payload.addToObject("generationConfig", generationConfig);
    
    return payload;
}

std::string GeminiApi::parseCompletionResponse(const std::string& response) {
    try {
        // Simple parsing - look for text content in the response
        std::regex contentRegex("\"text\"\\s*:\\s*\"((?:\\\\.|[^\"])*?)\"");
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
        
        // Also check for error messages
        std::regex errorRegex("\"message\"\\s*:\\s*\"((?:\\\\.|[^\"])*?)\"");
        if (std::regex_search(response, match, errorRegex) && match.size() > 1) {
            return "Error: " + match[1].str();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing completion response: " << e.what() << std::endl;
    }
    
    return "";
}

bool GeminiApi::isConfigured() const {
    return !getApiKey().empty() && !getEndpoint().empty();
}

void GeminiApi::cancelRequest() {
    cancelRequestFlag = true;
    httpClient.cancelRequest();
}

void GeminiApi::cleanupThread() {
    if (requestThread.joinable()) {
        {
            std::lock_guard<std::mutex> lock(threadMutex);
            cancelRequestFlag = true;
        }
        
        httpClient.cancelRequest();
        requestThread.join();
    }
} 