# GTKKS for Kindle

GTKKS (GTK LLM Client) is a native GTK application that allows you to chat with various Large Language Models (LLMs) directly from your Kindle e-reader.

## Features

- Chat with multiple LLM providers:
  - OpenAI (GPT-3.5, GPT-4)
  - Anthropic (Claude)
  - Google (Gemini)
  - Meta (Llama)
  - Ollama (local models)
  - OpenRouter (aggregator for multiple providers)
  - Deepseek

- Save and load chat conversations
- Configure API keys and endpoints
- Optimized for Kindle's e-ink display

## Installation

1. Connect your Kindle to your computer via USB.
2. Copy the `gtkks-kindle.tar.gz` and `kindle-install.sh` files to your Kindle.
3. Safely eject your Kindle from your computer.
4. On your Kindle, enable developer mode:
   - Open the search bar by tapping the top of the screen
   - Type `~ds`
   - Enable "Allow SSH connections"
5. Connect to your Kindle via SSH:
   ```
   ssh root@192.168.15.244  # Replace with your Kindle's IP address
   # Default password is usually 'mario'
   ```
6. Run the installation script:
   ```
   cd /mnt/us
   sh kindle-install.sh
   ```
7. Restart your Kindle.
8. After restart, you should see GTKKS in the Kindle menu.

## Usage

1. Open GTKKS from the Kindle menu.
2. Go to Settings and select your preferred API provider.
3. Enter your API key for the selected provider.
4. Select a model from the dropdown list.
5. Start chatting!

## Tips for Kindle Usage

- The application is optimized for e-ink displays, with appropriate refresh rates.
- Use larger font sizes for better readability.
- Be patient when waiting for responses, as network operations may be slower on Kindle.
- The application will automatically refresh the display when necessary.
- For best results, use a Wi-Fi connection with good signal strength.

## Troubleshooting

- If the application doesn't appear in the menu, check that the installation script ran correctly.
- If you encounter display issues, try restarting the application.
- For network-related issues, ensure your Kindle is connected to Wi-Fi.
- Check logs at `/var/log/messages` for error information.

## Uninstalling

To uninstall GTKKS from your Kindle:

```
rm -rf /mnt/us/gtkks-kindle
rm -rf /mnt/us/extensions/gtkks
```

Then restart your Kindle. 