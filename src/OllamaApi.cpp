#include "OllamaApi.h"
#include <sstream>
#include <iostream>

OllamaApi::OllamaApi() : cancelRequestFlag(false) {
    // Set default endpoint
    setEndpoint("http://localhost:11434");
}

OllamaApi::~OllamaApi() {
    // Cleanup thread
    cleanupThread();
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
    cancelRequestFlag = false;
    
    // Create a new thread for the request
    requestThread = std::thread([this, messages, model, callback]() {
        try {
            // Create URL
            std::string url = getEndpoint() + "/api/chat";
            
            // Create request payload
            SimpleJson payload = createRequestPayload(messages, model);
            
            // Convert to string
            std::string jsonPayload = payload.toJsonString();
            
            // Set content type
            httpClient.clearHeaders();
            httpClient.setHeader("Content-Type", "application/json");
            
            // Use streaming response
            std::string buffer;
            
            // Make the request with streaming response
            httpClient.postStreaming(
                url, 
                jsonPayload,
                [&buffer, &callback, this](const std::string& chunk) -> bool {
                    // Check if request was cancelled
                    if (cancelRequestFlag) {
                        return false;
                    }
                    
                    buffer += chunk;
                    
                    // Process buffer line by line
                    size_t pos = 0;
                    while ((pos = buffer.find("\n")) != std::string::npos) {
                        std::string line = buffer.substr(0, pos);
                        buffer.erase(0, pos + 1);
                        
                        if (line.empty()) continue;
                        
                        // Try to parse as JSON
                        try {
                            // Here we're looking for JSON in the format:
                            // {"message":{"content":"text"},"done":false|true}
                            
                            // Simple JSON parsing for "done" field
                            size_t donePos = line.find("\"done\":");
                            if (donePos != std::string::npos) {
                                size_t valueStart = donePos + 7; // Length of "done":
                                bool isDone = (line.find("true", valueStart) != std::string::npos);
                                
                                if (isDone) {
                                    callback("", true);
                                    return true;
                                }
                            }
                            
                            // Simple JSON parsing for message content
                            size_t contentPos = line.find("\"content\":");
                            if (contentPos != std::string::npos) {
                                size_t valueStart = contentPos + 10; // Length of "content":
                                
                                // Find the opening quote
                                size_t quoteStart = line.find("\"", valueStart);
                                if (quoteStart != std::string::npos) {
                                    size_t quoteEnd = line.find("\"", quoteStart + 1);
                                    if (quoteEnd != std::string::npos) {
                                        std::string content = line.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                                        
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
                                        
                                        callback(unescaped, false);
                                    }
                                }
                            }
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing response: " << e.what() << std::endl;
                        }
                    }
                    
                    return true;
                }
            );
            
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

void OllamaApi::cancelRequest() {
    // Set the cancel flag
    cancelRequestFlag = true;
    
    // Cancel any ongoing HTTP request
    httpClient.cancelRequest();
}

void OllamaApi::cleanupThread() {
    // Check if thread is running
    if (requestThread.joinable()) {
        {
            std::lock_guard<std::mutex> lock(threadMutex);
            cancelRequestFlag = true;
        }
        
        // Cancel any ongoing HTTP request
        httpClient.cancelRequest();
        
        // Wait for thread to finish
        requestThread.join();
    }
}

SimpleJson OllamaApi::createRequestPayload(const std::vector<Message>& messages, const std::string& model) {
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
    
    // Add stream flag
    payload.addToObject("stream", SimpleJson(true));
    
    return payload;
}

std::vector<std::string> OllamaApi::parseModelsResponse(const std::string& response) {
    std::vector<std::string> models;
    
    try {
        // The response is JSON in the format:
        // {"models":[{"name":"model1"},{"name":"model2"}]}
        
        // Simple parsing for model names
        size_t pos = 0;
        while ((pos = response.find("\"name\":\"", pos)) != std::string::npos) {
            pos += 8; // Length of "name":"
            size_t end = response.find("\"", pos);
            if (end != std::string::npos) {
                std::string modelName = response.substr(pos, end - pos);
                models.push_back(modelName);
                pos = end;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing models response: " << e.what() << std::endl;
    }
    
    return models;
}

std::string OllamaApi::parseStreamingResponse(const std::string& response) {
    try {
        // The streaming response is a series of JSON objects
        // Each object has a "message" field with the generated text
        
        return response;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing streaming response: " << e.what() << std::endl;
        return "";
    }
}

std::string OllamaApi::performHttpRequest(const std::string& url, const std::string& data) {
    try {
        // Set content type if we have data
        httpClient.clearHeaders();
        if (!data.empty()) {
            httpClient.setHeader("Content-Type", "application/json");
            return httpClient.post(url, data);
        } else {
            return httpClient.get(url);
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("HTTP request failed: " + std::string(e.what()));
    }
} 