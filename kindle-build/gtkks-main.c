#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <libgen.h>

#define LOG_FILE "gtkks_log.txt"
#define MAX_PATH 1024

FILE *log_file = NULL;

void log_message(const char *message) {
    if (log_file) {
        time_t now;
        struct tm *timeinfo;
        char timestamp[30];
        
        time(&now);
        timeinfo = localtime(&now);
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
        
        fprintf(log_file, "[%s] %s\n", timestamp, message);
        fflush(log_file);
    }
}

void log_user_input(const char *input) {
    if (log_file) {
        char buffer[1100];
        snprintf(buffer, sizeof(buffer), "User input: %s", input);
        log_message(buffer);
    }
}

void log_system_response(const char *response) {
    if (log_file) {
        char buffer[1100];
        snprintf(buffer, sizeof(buffer), "System response: %s", response);
        log_message(buffer);
    }
}

void initialize_logging(const char *exec_path) {
    char log_path[MAX_PATH];
    char dir_path[MAX_PATH];
    
    // Get the directory of the executable
    strncpy(dir_path, exec_path, sizeof(dir_path));
    char *dir = dirname(dir_path);
    
    // Create the log file path
    snprintf(log_path, sizeof(log_path), "%s/%s", dir, LOG_FILE);
    
    // Open the log file
    log_file = fopen(log_path, "a");
    if (!log_file) {
        fprintf(stderr, "Error opening log file: %s\n", strerror(errno));
        return;
    }
    
    log_message("=== GTKKS Application Started ===");
    log_message("Logging initialized");
}

void close_logging() {
    if (log_file) {
        log_message("=== GTKKS Application Closed ===");
        fclose(log_file);
        log_file = NULL;
    }
}

void display_welcome() {
    printf("\n");
    printf("  ██████╗ ████████╗██╗  ██╗██╗  ██╗███████╗\n");
    printf(" ██╔════╝ ╚══██╔══╝██║ ██╔╝██║ ██╔╝██╔════╝\n");
    printf(" ██║  ███╗   ██║   █████╔╝ █████╔╝ ███████╗\n");
    printf(" ██║   ██║   ██║   ██╔═██╗ ██╔═██╗ ╚════██║\n");
    printf(" ╚██████╔╝   ██║   ██║  ██╗██║  ██╗███████║\n");
    printf("  ╚═════╝    ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝\n");
    printf("\n");
    printf("Welcome to GTKKS for Kindle Scribe (armhf)\n");
    printf("A GTK-based LLM client\n");
    printf("\n");
    
    log_message("Welcome message displayed");
}

void process_input() {
    char input[1024];
    char response[1024];
    
    printf("Enter your message (or 'exit' to quit):\n> ");
    log_message("Waiting for user input");
    
    while (fgets(input, sizeof(input), stdin)) {
        // Remove newline
        input[strcspn(input, "\n")] = 0;
        
        log_user_input(input);
        
        if (strcmp(input, "exit") == 0) {
            printf("Goodbye!\n");
            log_message("User requested exit");
            break;
        }
        
        printf("\nProcessing your message: \"%s\"\n", input);
        printf("Simulating response from GPT-4...\n\n");
        log_message("Processing user message");
        
        // Simulate thinking
        printf("Thinking");
        for (int i = 0; i < 3; i++) {
            fflush(stdout);
            sleep(1);
            printf(".");
        }
        printf("\n\n");
        
        // Generate a simple response
        if (strstr(input, "hello") || strstr(input, "hi")) {
            strcpy(response, "Hello! How can I assist you today on your Kindle Scribe?");
            printf("%s\n", response);
        } else if (strstr(input, "help")) {
            strcpy(response, "This is a placeholder for the GTKKS application. In the full version, you would be able to:\n1. Chat with various LLM models\n2. Configure API settings\n3. Save and load conversations\n4. Customize the interface");
            printf("%s\n", response);
        } else if (strstr(input, "kindle") || strstr(input, "scribe")) {
            strcpy(response, "Yes, this application is designed specifically for the Kindle Scribe with armhf architecture.\nIt provides a lightweight interface for interacting with language models.");
            printf("%s\n", response);
        } else if (strstr(input, "log") || strstr(input, "logs")) {
            strcpy(response, "Yes, this application automatically creates logs in its root folder.\nCheck the gtkks_log.txt file for the complete log history.");
            printf("%s\n", response);
        } else {
            snprintf(response, sizeof(response), "I understand you're interested in \"%s\". In the full version of GTKKS,\nI would provide a more detailed and helpful response based on your query.\nThis is just a placeholder to demonstrate the application structure.", input);
            printf("%s\n", response);
        }
        
        log_system_response(response);
        
        printf("\nEnter your message (or 'exit' to quit):\n> ");
        log_message("Waiting for user input");
    }
}

int main(int argc, char *argv[]) {
    // Initialize logging with the executable path
    initialize_logging(argv[0]);
    
    // Log system information
    log_message("System: Kindle Scribe (armhf)");
    
    display_welcome();
    process_input();
    
    close_logging();
    return 0;
}
