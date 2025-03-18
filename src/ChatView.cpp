#include "ChatView.h"
#include "MainWindow.h"
#include "LLMApi.h"
#include <gtkmm.h>
#include <iostream>
#include <fstream>
#include <regex>
#include <chrono>
#include <ctime>
#include <sstream>

ChatView::ChatView()
    : Gtk::VBox(false, 10),
      inputBox(false, 5),
      buttonBox(false, 5),
      isFirstResponseChunk(true),
      currentResponseText("") {
    
    // Set up chat text view
    chatTextView.set_editable(false);
    chatTextView.set_wrap_mode(Gtk::WRAP_WORD_CHAR);
    chatTextView.set_cursor_visible(false);
    chatTextView.set_border_width(10);
    
    // Set up chat scrolled window
    chatScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    chatScrolledWindow.add(chatTextView);
    chatScrolledWindow.set_size_request(-1, 400);
    
    // Set up input text view
    inputTextView.set_wrap_mode(Gtk::WRAP_WORD_CHAR);
    inputTextView.set_border_width(10);
    inputTextView.set_size_request(-1, 100);
    
    // Set up buttons
    sendButton.set_label("Send");
    clearButton.set_label("Clear");
    
    // Set up progress bar
    progressBar.set_no_show_all(true);
    progressBar.set_pulse_step(0.1);
    
    // Set up input box
    inputBox.pack_start(inputTextView, true, true, 0);
    
    // Set up button box
    buttonBox.pack_start(clearButton, false, false, 0);
    buttonBox.pack_end(sendButton, false, false, 0);
    
    // Add widgets to the box
    pack_start(chatScrolledWindow, true, true, 0);
    pack_start(progressBar, false, false, 0);
    pack_start(inputBox, false, false, 0);
    pack_start(buttonBox, false, false, 0);
    
    // Get text buffers
    chatBuffer = chatTextView.get_buffer();
    inputBuffer = inputTextView.get_buffer();
    
    // Connect signals
    sendButton.signal_clicked().connect(sigc::mem_fun(*this, &ChatView::onSendClicked));
    clearButton.signal_clicked().connect(sigc::mem_fun(*this, &ChatView::onClearClicked));
    inputTextView.signal_key_press_event().connect(sigc::mem_fun(*this, &ChatView::onInputKeyPress), false);
    
    // Add system message
    appendSystemMessage("Welcome to GTKKS LLM Client. Select an API and model to start chatting.");
}

ChatView::~ChatView() {
}

void ChatView::setApi(std::shared_ptr<LLMApi> api, const std::string& model) {
    currentApi = api;
    currentModel = model;
    
    if (api) {
        appendSystemMessage("Using " + api->getName() + " with model " + model);
    } else {
        appendSystemMessage("No API selected");
    }
}

void ChatView::clearChat() {
    chatBuffer->set_text("");
    messages.clear();
    appendSystemMessage("Chat history cleared");
}

void ChatView::saveChat(const std::string& filename) {
    try {
        // Create JSON object
        SimpleJson root;
        
        // Add messages
        SimpleJson jsonMessages;
        for (const auto& message : messages) {
            SimpleJson jsonMessage;
            jsonMessage.addToObject("role", SimpleJson(message.role));
            jsonMessage.addToObject("content", SimpleJson(message.content));
            jsonMessages.addToArray(jsonMessage);
        }
        root.addToObject("messages", jsonMessages);
        
        // Write to file
        std::ofstream file(filename);
        if (file.is_open()) {
            file << root.toJsonString();
            file.close();
            
            appendSystemMessage("Chat history saved to " + filename);
        } else {
            appendSystemMessage("Error: Failed to save chat history to " + filename);
        }
    } catch (const std::exception& e) {
        appendSystemMessage("Error saving chat: " + std::string(e.what()));
    }
}

void ChatView::loadChat(const std::string& filename) {
    try {
        // Read file
        std::ifstream file(filename);
        if (!file.is_open()) {
            appendSystemMessage("Error: Failed to open file " + filename);
            return;
        }
        
        // Read entire file into a string
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string fileContents = buffer.str();
        
        // Clear current chat
        clearChat();
        
        // Extract messages using regex
        std::regex messageRegex("\\{\"role\":\"([^\"]+)\",\"content\":\"([^\"]+)\"\\}");
        std::smatch match;
        std::string::const_iterator searchStart(fileContents.cbegin());
        
        while (std::regex_search(searchStart, fileContents.cend(), match, messageRegex)) {
            if (match.size() > 2) {
                std::string role = match[1].str();
                std::string content = match[2].str();
                
                // Add message to chat
                appendMessage(role, content);
                
                // Add message to history
                Message message;
                message.role = role;
                message.content = content;
                messages.push_back(message);
            }
            
            searchStart = match.suffix().first;
        }
        
        appendSystemMessage("Chat history loaded from " + filename);
    } catch (const std::exception& e) {
        appendSystemMessage("Error loading chat: " + std::string(e.what()));
    }
}

void ChatView::onSendClicked() {
    // Get input text
    Glib::ustring text = inputBuffer->get_text();
    if (text.empty()) {
        return;
    }
    
    // Check if API is set
    if (!currentApi) {
        appendSystemMessage("Error: No API selected");
        return;
    }
    
    // Add user message to chat
    appendUserMessage(text);
    
    // Add user message to history
    Message userMessage;
    userMessage.role = "user";
    userMessage.content = text;
    messages.push_back(userMessage);
    
    // Clear input
    inputBuffer->set_text("");
    
    // Reset streaming response state
    isFirstResponseChunk = true;
    currentResponseText = "";
    
    // Disable input while waiting for response
    setInputSensitivity(false);
    
    // Show progress bar
    updateProgressBar(true);
    
    // Start progress bar animation
    Glib::signal_timeout().connect(
        [this]() {
            if (progressBar.get_visible()) {
                progressBar.pulse();
                return true;
            }
            return false;
        },
        100
    );
    
    // Send request to API
    currentApi->sendChatRequest(
        messages,
        currentModel,
        [this](const std::string& response, bool isComplete) {
            // Use Glib::signal_idle to update UI from main thread
            Glib::signal_idle().connect_once(
                [this, response, isComplete]() {
                    handleApiResponse(response, isComplete);
                }
            );
        }
    );
}

void ChatView::onClearClicked() {
    clearChat();
}

bool ChatView::onInputKeyPress(GdkEventKey* event) {
    // Check for Ctrl+Enter to send message
    if (event->keyval == GDK_KEY_Return && (event->state & GDK_CONTROL_MASK)) {
        onSendClicked();
        return true;
    }
    
    return false;
}

void ChatView::appendMessage(const std::string& role, const std::string& content) {
    // Create tags for different roles
    Glib::RefPtr<Gtk::TextBuffer::Tag> roleTag;
    
    if (role == "user") {
        roleTag = chatBuffer->create_tag();
        roleTag->property_foreground() = "#0066cc";
        roleTag->property_weight() = Pango::WEIGHT_BOLD;
    } else if (role == "assistant") {
        roleTag = chatBuffer->create_tag();
        roleTag->property_foreground() = "#006600";
        roleTag->property_weight() = Pango::WEIGHT_BOLD;
    } else if (role == "system") {
        roleTag = chatBuffer->create_tag();
        roleTag->property_foreground() = "#666666";
        roleTag->property_style() = Pango::STYLE_ITALIC;
    }
    
    // Add newline if buffer is not empty
    Gtk::TextBuffer::iterator iter = chatBuffer->end();
    Glib::RefPtr<Gtk::TextBuffer::Mark> endMark = chatBuffer->create_mark("end_mark", iter, false);
    
    if (!chatBuffer->get_text().empty()) {
        iter = chatBuffer->get_iter_at_mark(endMark);
        chatBuffer->insert(iter, "\n\n");
    }
    
    // Get the updated end position
    iter = chatBuffer->get_iter_at_mark(endMark);
    
    // Add role
    Glib::RefPtr<Gtk::TextBuffer::Mark> startMark = chatBuffer->create_mark("start_mark", iter, true);
    chatBuffer->insert(iter, role + ": ");
    
    // Get the updated end position
    iter = chatBuffer->get_iter_at_mark(endMark);
    
    // Apply tag to role
    Gtk::TextBuffer::iterator start = chatBuffer->get_iter_at_mark(startMark);
    chatBuffer->apply_tag(roleTag, start, iter);
    
    // Add content
    chatBuffer->insert(iter, content);
    
    // Clean up marks
    chatBuffer->delete_mark(startMark);
    chatBuffer->delete_mark(endMark);
    
    // Scroll to bottom
    chatTextView.scroll_to(chatBuffer->get_insert());
}

void ChatView::appendUserMessage(const std::string& content) {
    appendMessage("user", content);
}

void ChatView::appendAssistantMessage(const std::string& content) {
    appendMessage("assistant", content);
}

void ChatView::appendSystemMessage(const std::string& content) {
    appendMessage("system", content);
}

void ChatView::handleApiResponse(const std::string& response, bool isComplete) {
    // Hide progress bar when complete
    if (isComplete) {
        updateProgressBar(false);
    }
    
    if (isFirstResponseChunk && !response.empty()) {
        // This is the first chunk of the response
        isFirstResponseChunk = false;
        
        // Create tags for assistant role
        Glib::RefPtr<Gtk::TextBuffer::Tag> roleTag = chatBuffer->create_tag();
        roleTag->property_foreground() = "#006600";
        roleTag->property_weight() = Pango::WEIGHT_BOLD;
        
        // Add newline if buffer is not empty
        Gtk::TextBuffer::iterator iter = chatBuffer->end();
        if (!chatBuffer->get_text().empty()) {
            chatBuffer->insert(iter, "\n\n");
            iter = chatBuffer->end();
        }
        
        // Create a mark at the current position
        responseStartMark = chatBuffer->create_mark("response_start", iter, true);
        
        // Add role prefix
        chatBuffer->insert(iter, "assistant: ");
        iter = chatBuffer->end();
        
        // Apply tag to role
        Gtk::TextBuffer::iterator start = chatBuffer->get_iter_at_mark(responseStartMark);
        chatBuffer->apply_tag(roleTag, start, iter);
        
        // Create a mark for the content start
        Glib::RefPtr<Gtk::TextBuffer::Mark> contentStartMark = chatBuffer->create_mark("content_start", iter, true);
        
        // Add the first chunk of content
        chatBuffer->insert(iter, response);
        
        // Create a mark for the end of the content
        responseEndMark = chatBuffer->create_mark("response_end", chatBuffer->end(), false);
        
        // Update current response text
        currentResponseText += response;
        
        // Clean up the content start mark
        chatBuffer->delete_mark(contentStartMark);
    } 
    else if (!isFirstResponseChunk && !isComplete) {
        // This is a continuation of the response
        // Get the current end position
        Gtk::TextBuffer::iterator endIter = chatBuffer->get_iter_at_mark(responseEndMark);
        
        // Add the new content
        chatBuffer->insert(endIter, response);
        
        // Update the end mark
        chatBuffer->move_mark(responseEndMark, chatBuffer->end());
        
        // Update current response text
        currentResponseText += response;
    }
    else if (isComplete) {
        // This is the final chunk
        if (isFirstResponseChunk) {
            // If this is both the first and last chunk, just append it normally
            appendAssistantMessage(response);
            currentResponseText = response;
        } 
        else {
            // Add the final chunk if there is any
            if (!response.empty()) {
                Gtk::TextBuffer::iterator endIter = chatBuffer->get_iter_at_mark(responseEndMark);
                chatBuffer->insert(endIter, response);
                currentResponseText += response;
            }
            
            // Clean up the marks
            chatBuffer->delete_mark(responseStartMark);
            chatBuffer->delete_mark(responseEndMark);
        }
        
        // Add the complete message to history
        Message assistantMessage;
        assistantMessage.role = "assistant";
        assistantMessage.content = currentResponseText;
        messages.push_back(assistantMessage);
        
        // Reset for next response
        isFirstResponseChunk = true;
        currentResponseText = "";
    }
    
    // Scroll to bottom
    chatTextView.scroll_to(chatBuffer->get_insert());
    
    // Enable input when complete
    if (isComplete) {
        setInputSensitivity(true);
    }
}

void ChatView::setInputSensitivity(bool sensitive) {
    inputTextView.set_sensitive(sensitive);
    sendButton.set_sensitive(sensitive);
}

void ChatView::updateProgressBar(bool visible, double progress) {
    progressBar.set_visible(visible);
    if (visible && progress >= 0) {
        progressBar.set_fraction(progress);
    }
}
