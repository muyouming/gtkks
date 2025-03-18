#include "HttpClient.h"
#include <sstream>
#include <iostream>
#include <regex>

HttpClient::HttpClient() : cancelled(false) {
    client = Gio::SocketClient::create();
}

HttpClient::~HttpClient() {
    // Ensure connection is closed
    if (connection) {
        try {
            connection->close();
        } catch (...) {
            // Ignore errors during cleanup
        }
    }
}

void HttpClient::setHeader(const std::string& name, const std::string& value) {
    headers[name] = value;
}

void HttpClient::clearHeaders() {
    headers.clear();
}

std::string HttpClient::get(const std::string& url) {
    return makeRequest("GET", url, "");
}

std::string HttpClient::post(const std::string& url, const std::string& data) {
    return makeRequest("POST", url, data);
}

HttpClient::UrlParts HttpClient::parseUrl(const std::string& url) {
    UrlParts parts;
    
    // Use regex to parse URL
    std::regex urlRegex("(http|https)://([^:/]+)(:[0-9]+)?(/.*)?");
    std::smatch match;
    
    if (std::regex_match(url, match, urlRegex)) {
        parts.protocol = match[1].str();
        parts.host = match[2].str();
        
        // Parse port
        if (match[3].length() > 0) {
            parts.port = std::stoi(match[3].str().substr(1));
        } else {
            parts.port = (parts.protocol == "https") ? 443 : 80;
        }
        
        // Parse path
        if (match[4].length() > 0) {
            parts.path = match[4].str();
        } else {
            parts.path = "/";
        }
    } else {
        throw std::runtime_error("Invalid URL: " + url);
    }
    
    return parts;
}

std::string HttpClient::makeRequest(const std::string& method, const std::string& url, const std::string& data) {
    // Reset cancellation flag
    cancelled = false;
    
    // Parse URL
    UrlParts parts = parseUrl(url);
    
    // Create connection
    try {
        connection = client->connect_to_host(parts.host, parts.port);
    } catch (const Glib::Error& e) {
        std::cerr << "Connection failed to " << parts.host << ":" << parts.port << " - " << e.what() << std::endl;
        throw std::runtime_error("Connection failed: " + std::string(e.what()));
    }
    
    // Create request
    std::stringstream request;
    request << method << " " << parts.path << " HTTP/1.1\r\n";
    request << "Host: " << parts.host << "\r\n";
    
    // Add headers
    for (const auto& [name, value] : headers) {
        request << name << ": " << value << "\r\n";
    }
    
    // Add content length if we have data
    if (!data.empty()) {
        request << "Content-Length: " << data.length() << "\r\n";
        // If no content type is specified, add a default one
        if (headers.find("Content-Type") == headers.end()) {
            request << "Content-Type: application/json\r\n";
        }
    }
    
    // End headers
    request << "Connection: close\r\n"; // Add this to ensure the server closes the connection after response
    request << "\r\n";
    
    // Add data if we have it
    if (!data.empty()) {
        request << data;
    }
    
    // Send request
    std::string requestStr = request.str();
    
    // Debug output
    std::cerr << "Sending request to: " << url << std::endl;
    
    try {
        connection->get_output_stream()->write(requestStr.data(), requestStr.size());
    } catch (const Glib::Error& e) {
        std::cerr << "Failed to send request: " << e.what() << std::endl;
        throw std::runtime_error("Failed to send request: " + std::string(e.what()));
    }
    
    // Read response
    char buffer[4096];
    std::string response;
    
    try {
        while (!cancelled) {
            gssize bytes_read = connection->get_input_stream()->read(buffer, sizeof(buffer));
            if (bytes_read <= 0) break;
            response.append(buffer, bytes_read);
            
            // Check if we've received the full headers
            size_t headerEnd = response.find("\r\n\r\n");
            if (headerEnd != std::string::npos) {
                // Extract headers
                std::string headers = response.substr(0, headerEnd);
                
                // Debug output
                std::cerr << "Response headers: " << std::endl << headers << std::endl;
                
                // Extract status code
                std::regex statusRegex("HTTP/[0-9.]+ ([0-9]+)");
                std::smatch match;
                if (std::regex_search(headers, match, statusRegex)) {
                    int statusCode = std::stoi(match[1].str());
                    if (statusCode >= 400) {
                        // Get response body for error details
                        std::string errorBody = response.substr(headerEnd + 4);
                        std::cerr << "HTTP error " << statusCode << ": " << errorBody << std::endl;
                        throw std::runtime_error("HTTP error " + std::to_string(statusCode) + ": " + errorBody);
                    }
                }
                
                // Extract content length if available
                std::regex contentLengthRegex("Content-Length: ([0-9]+)");
                if (std::regex_search(headers, match, contentLengthRegex)) {
                    size_t contentLength = std::stoull(match[1].str());
                    size_t bodyStart = headerEnd + 4;
                    size_t bodySize = response.size() - bodyStart;
                    
                    // If we have the full content, return it
                    if (bodySize >= contentLength) {
                        return response.substr(bodyStart, contentLength);
                    }
                } else {
                    // Check for chunked transfer encoding
                    std::regex chunkedRegex("Transfer-Encoding:\\s*chunked", std::regex::icase);
                    if (std::regex_search(headers, chunkedRegex)) {
                        // Handle chunked response
                        std::string body = response.substr(headerEnd + 4);
                        // For now, just return the raw chunked body
                        return body;
                    } else {
                        // If no content length and not chunked, assume we got everything
                        return response.substr(headerEnd + 4);
                    }
                }
            }
        }
    } catch (const Glib::Error& e) {
        std::cerr << "Error reading response: " << e.what() << std::endl;
        throw std::runtime_error("Error reading response: " + std::string(e.what()));
    }
    
    if (cancelled) {
        throw std::runtime_error("Request cancelled");
    }
    
    return response;
}

void HttpClient::postStreaming(
    const std::string& url, 
    const std::string& data,
    const std::function<bool(const std::string&)>& dataCallback
) {
    // Reset cancellation flag
    cancelled = false;
    
    // Parse URL
    UrlParts parts = parseUrl(url);
    
    // Create connection
    try {
        connection = client->connect_to_host(parts.host, parts.port);
    } catch (const Glib::Error& e) {
        throw std::runtime_error("Connection failed: " + std::string(e.what()));
    }
    
    // Create request
    std::stringstream request;
    request << "POST " << parts.path << " HTTP/1.1\r\n";
    request << "Host: " << parts.host << "\r\n";
    
    // Add headers
    for (const auto& [name, value] : headers) {
        request << name << ": " << value << "\r\n";
    }
    
    // Add content length
    request << "Content-Length: " << data.length() << "\r\n";
    
    // If no content type is specified, add a default one
    if (headers.find("Content-Type") == headers.end()) {
        request << "Content-Type: application/json\r\n";
    }
    
    // End headers
    request << "\r\n";
    
    // Add data
    request << data;
    
    // Send request
    std::string requestStr = request.str();
    connection->get_output_stream()->write(requestStr.data(), requestStr.size());
    
    // Read response
    char buffer[4096];
    std::string response;
    bool headersReceived = false;
    size_t bodyStart = 0;
    
    while (!cancelled) {
        gssize bytes_read = connection->get_input_stream()->read(buffer, sizeof(buffer));
        if (bytes_read <= 0) break;
        response.append(buffer, bytes_read);
        
        if (!headersReceived) {
            // Check if we've received the full headers
            size_t headerEnd = response.find("\r\n\r\n");
            if (headerEnd != std::string::npos) {
                // Extract headers
                std::string headers = response.substr(0, headerEnd);
                
                // Extract status code
                std::regex statusRegex("HTTP/[0-9.]+ ([0-9]+)");
                std::smatch match;
                if (std::regex_search(headers, match, statusRegex)) {
                    int statusCode = std::stoi(match[1].str());
                    if (statusCode >= 400) {
                        throw std::runtime_error("HTTP error: " + std::to_string(statusCode));
                    }
                }
                
                headersReceived = true;
                bodyStart = headerEnd + 4;
                
                // Process any body data we already have
                if (response.size() > bodyStart) {
                    std::string chunk = response.substr(bodyStart);
                    if (!dataCallback(chunk)) {
                        cancelled = true;
                        break;
                    }
                    response.erase(bodyStart);
                }
            }
        } else {
            // Process body data
            if (!dataCallback(std::string(buffer, bytes_read))) {
                cancelled = true;
                break;
            }
        }
    }
    
    // Close connection
    if (connection) {
        try {
            connection->close();
            connection.reset();
        } catch (...) {
            // Ignore errors during cleanup
        }
    }
}

void HttpClient::cancelRequest() {
    cancelled = true;
    
    // Close connection to interrupt any in-progress operations
    if (connection) {
        try {
            connection->close();
        } catch (...) {
            // Ignore errors during cancellation
        }
    }
} 