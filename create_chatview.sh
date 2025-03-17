#!/bin/bash

cat > src/ChatView.cpp << 'EOF'
#include "ChatView.h"
#include <iostream>
#include <fstream>
#include <json/json.h>

ChatView::ChatView()
    : Gtk::Box(Gtk::ORIENTATION_VERTICAL, 10),
      inputBox(Gtk::ORIENTATION_HORIZONTAL, 5),
      buttonBox(Gtk::ORIENTATION_HORIZONTAL, 5) {
    
    // Set up chat text view
    chatTextView.set_editable(false);
    chatTextView.set_wrap_mode(Gtk::WRAP_WORD_CHAR);
    chatTextView.set_cursor_visible(false);
    chatTextView.set_left_margin(10);
    chatTextView.set_right_margin(10);
    chatTextView.set_top_margin(10);
    chatTextView.set_bottom_margin(10);
    
    // Set up chat scrolled window
    chatScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    chatScrolledWindow.add(chatTextView);
    chatScrolledWindow.set_size_request(-1, 400);
    
    // Set up input text view
    inputTextView.set_wrap_mode(Gtk::WRAP_WORD_CHAR);
    inputTextView.set_left_margin(10);
    inputTextView.set_right_margin(10);
    inputTextView.set_top_margin(10);
    inputTextView.set_bottom_margin(10);
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
        Json::Value root;
        
        // Add messages
        Json::Value jsonMessages(Json::arrayValue);
        for (const auto& message : messages) {
            Json::Value jsonMessage;
            jsonMessage["role"] = message.role;
            jsonMessage["content"] = message.content;
            jsonMessages.append(jsonMessage);
        }
        root["messages"] = jsonMessages;
        
        // Write to file
        std::ofstream file(filename);
        if (file.is_open()) {
            Json::StreamWriterBuilder builder;
            builder["indentation"] = "  ";
            std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
            writer->write(root, &file);
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
        
        // Parse JSON
        Json::Value root;
        Json::CharReaderBuilder builder;
        std::string errs;
        if (!Json::parseFromStream(builder, file, &root, &errs)) {
            appendSystemMessage("Error parsing file: " + errs);
            return;
        }
        
        // Clear current chat
        clearChat();
        
        // Load messages
        if (root.isMember("messages") && root["messages"].isArray()) {
            const Json::Value& jsonMessages = root["messages"];
            for (const auto& jsonMessage : jsonMessages) {
                if (jsonMessage.isMember("role") && jsonMessage.isMember("content")) {
                    std::string role = jsonMessage["role"].asString();
                    std::string content = jsonMessage["content"].asString();
                    
                    // Add message to chat
                    appendMessage(role, content);
                    
                    // Add message to history
                    Message message;
                    message.role = role;
                    message.content = content;
                    messages.push_back(message);
                }
            }
            
            appendSystemMessage("Chat history loaded from " + filename);
        }
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
    
    // Add role and content to chat
    Gtk::TextBuffer::iterator iter = chatBuffer->end();
    
    // Add newline if buffer is not empty
    if (!chatBuffer->get_text().empty()) {
        chatBuffer->insert(iter, "\n\n");
        iter = chatBuffer->end();
    }
    
    // Add role
    Gtk::TextBuffer::iterator start = iter;
    chatBuffer->insert(iter, role + ": ");
    Gtk::TextBuffer::iterator end = chatBuffer->end();
    chatBuffer->apply_tag(roleTag, start, end);
    
    // Add content
    chatBuffer->insert(end, content);
    
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
    // Hide progress bar
    if (isComplete) {
        updateProgressBar(false);
    }
    
    // Add assistant message to chat
    appendAssistantMessage(response);
    
    // Add assistant message to history
    Message assistantMessage;
    assistantMessage.role = "assistant";
    assistantMessage.content = response;
    messages.push_back(assistantMessage);
    
    // Enable input
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
EOF

echo "ChatView.cpp created successfully." 