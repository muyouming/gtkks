# GTKKS - GTK LLM Client

A GTK3-based GUI client for Ollama and other LLM services written in C++.

## Features

- Support for multiple LLM services:
  - Ollama (local models)
  - OpenAI API
  - Google Gemini API
  - DeepSeek API
  - OpenRouter API (access to multiple models from different providers)
- Chat interface with message history
- Model selection
- API key management
- Save and load chat history

## Dependencies

- C++17 compiler
- GTK3 / gtkmm-3.0
- libcurl / curlpp
- jsoncpp
- CMake (3.10 or higher)

## Installation

### Ubuntu/Debian

```bash
# Install dependencies
sudo apt update
sudo apt install build-essential cmake pkg-config libgtkmm-3.0-dev libcurlpp-dev libjsoncpp-dev
```

### macOS with Homebrew

```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake gtkmm3 curlpp jsoncpp
```

## Building

```bash
# Clone the repository
git clone https://github.com/yourusername/gtkks.git
cd gtkks

# Build
mkdir build
cd build
cmake ..
make

# Run
./gtkks
```

Alternatively, you can use the provided build script:

```bash
chmod +x build.sh
./build.sh
```

## Usage

1. Launch the application
2. Set up your API keys in the settings (click the gear icon)
3. Select the LLM service you want to use from the dropdown
4. Choose a model
5. Start chatting!

### API Keys

- **Ollama**: No API key required for local deployment
- **OpenAI**: Get your API key from [OpenAI Platform](https://platform.openai.com/api-keys)
- **Gemini**: Get your API key from [Google AI Studio](https://makersuite.google.com/app/apikey)
- **DeepSeek**: Get your API key from [DeepSeek Platform](https://platform.deepseek.com/)
- **OpenRouter**: Get your API key from [OpenRouter](https://openrouter.ai/keys)

### Keyboard Shortcuts

- `Ctrl+Enter`: Send message

## Configuration

The application stores its configuration in `~/.config/gtkks/config.json` or in the same directory as the executable (`config.json`). The configuration includes:

- API keys for different services
- API endpoints
- Last used API and model

You can manually edit this file or use the settings dialog in the application. Here's an example configuration:

```json
{
  "api_keys": {
    "OpenAI": "your-openai-api-key-here",
    "Gemini": "your-gemini-api-key-here",
    "Deepseek": "your-deepseek-api-key-here",
    "OpenRouter": "your-openrouter-api-key-here"
  },
  "endpoints": {
    "Ollama": "http://localhost:11434",
    "OpenAI": "https://api.openai.com/v1",
    "Gemini": "https://generativelanguage.googleapis.com/v1",
    "Deepseek": "https://api.deepseek.com/v1",
    "OpenRouter": "https://openrouter.ai/api/v1"
  },
  "last_used_api": "Ollama",
  "last_used_model": "llama3"
}
```

The application will automatically load this configuration at startup and save changes when you modify settings.

## License

MIT 