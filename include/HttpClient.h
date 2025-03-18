#pragma once

#include <string>
#include <functional>
#include <gtkmm.h>

// Simple HTTP client using standard C++ and GTK
class HttpClient {
public:
    // Constructor
    HttpClient();
    
    // Destructor
    ~HttpClient();
    
    // Set headers for the request
    void setHeader(const std::string& name, const std::string& value);
    
    // Clear all headers
    void clearHeaders();
    
    // Perform a GET request
    std::string get(const std::string& url);
    
    // Perform a POST request
    std::string post(const std::string& url, const std::string& data);
    
    // Perform a POST request with streaming response
    void postStreaming(
        const std::string& url, 
        const std::string& data,
        const std::function<bool(const std::string&)>& dataCallback
    );
    
    // Cancel ongoing requests
    void cancelRequest();

private:
    // Headers
    std::map<std::string, std::string> headers;
    
    // Connection object
    Glib::RefPtr<Gio::SocketClient> client;
    
    // Current connection
    Glib::RefPtr<Gio::SocketConnection> connection;
    
    // Cancel flag
    bool cancelled;
    
    // Common code for making a request
    std::string makeRequest(const std::string& method, const std::string& url, const std::string& data);
    
    // Parse URL
    struct UrlParts {
        std::string protocol;
        std::string host;
        int port;
        std::string path;
    };
    UrlParts parseUrl(const std::string& url);
}; 