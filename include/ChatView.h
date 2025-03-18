#pragma once

#include <gtkmm.h>
#include "LLMApi.h"
#include <string>
#include <vector>
#include <memory>

class ChatView : public Gtk::VBox {
public:
    ChatView();
    virtual ~ChatView();

    // Set the API to use
    void setApi(std::shared_ptr<LLMApi> api, const std::string& model);

    // Clear chat history
    void clearChat();

    // Save chat history to file
    void saveChat(const std::string& filename);

    // Load chat history from file
    void loadChat(const std::string& filename);

private:
    // UI elements
    Gtk::ScrolledWindow chatScrolledWindow;
    Gtk::TextView chatTextView;
    Gtk::TextView inputTextView;
    Gtk::Button sendButton;
    Gtk::Button clearButton;
    Gtk::ProgressBar progressBar;
    Gtk::HBox inputBox;
    Gtk::HBox buttonBox;

    // Chat history
    std::vector<Message> messages;

    // Current API and model
    std::shared_ptr<LLMApi> currentApi;
    std::string currentModel;

    // Text buffers
    Glib::RefPtr<Gtk::TextBuffer> chatBuffer;
    Glib::RefPtr<Gtk::TextBuffer> inputBuffer;
    
    // Streaming response tracking
    bool isFirstResponseChunk;
    std::string currentResponseText;
    Gtk::TextBuffer::iterator responseStartIter;
    Gtk::TextBuffer::iterator responseEndIter;
    Glib::RefPtr<Gtk::TextBuffer::Mark> responseStartMark;
    Glib::RefPtr<Gtk::TextBuffer::Mark> responseEndMark;

    // Signal handlers
    void onSendClicked();
    void onClearClicked();
    bool onInputKeyPress(GdkEventKey* event);

    // Helper methods
    void appendMessage(const std::string& role, const std::string& content);
    void appendUserMessage(const std::string& content);
    void appendAssistantMessage(const std::string& content);
    void appendSystemMessage(const std::string& content);
    void handleApiResponse(const std::string& response, bool isComplete);
    void setInputSensitivity(bool sensitive);
    void updateProgressBar(bool visible, double progress = 0.0);
}; 