#include <monitor.h>

typedef int (*command_function_t) (int argc, char** argv);

typedef struct {
    const char* name;
    const char* description;
    command_function_t function;
} command_t;

static int command_help();
static int command_echo(int argc, char** argv);
static int command_panic();
static int command_sleep(int argc, char** argv);
static int command_shutdown();
static int tokenize(char *input, char** argv, int max_args);

static const command_t commands[] = {
    {"help", "Display this help message", command_help},
    {"echo", "Echo the input arguments", command_echo},
    {"panic", "Trigger a kernel panic", command_panic},
    {"sleep", "Sleep for a specified duration (ms)", command_sleep},
    {"shutdown", "Shut down the system", command_shutdown}
};

#define NUM_COMMANDS (sizeof(commands) / sizeof(commands[0]))
#define MAX_COMMAND_ARGS 16
#define MAX_INPUT_LENGTH 128

static int command_help() {
    kprintf("Available commands:\n");
    for (int i = 0; i < (int) NUM_COMMANDS; i++) {
        kprintf("%s: %s\n", commands[i].name, commands[i].description);
    }
    return 0;
}

static int command_echo(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        kprintf("%s%s", argv[i], (i < argc - 1) ? " " : ""); // Print space between arguments
    }
    kprintf("\n");
    return 0;
}

// Top notch security right here
static int command_panic() {
    panic("User triggered panic via 'panic' command");
    return 0; // Unreachable
}

static int command_sleep(int argc, char** argv) {
    if (argc != 2) {
        kprintf("Usage: sleep <milliseconds>\n");
        return -1;
    }

    int ms = 0;
    // Simple atoi implementation
    for (const char* p = argv[1]; *p; p++) {
        if (*p < '0' || *p > '9') {
            kprintf("Invalid number: %s\n", argv[1]);
            return -1;
        }
        // Divide by 10 and add digit
        ms = ms * 10 + (*p - '0');
    }

    kprintf("Sleeping for %d ms...\n", ms);
    sleep_ms((uint64_t) ms); // sleep_ms expects 1ms ticks
    kprintf("Woke up after %d ms.\n", ms);
    return 0;
}

static int command_shutdown() {
    kprintf("Bye bye!\n");
    sbi_system_shutdown();
    return 0; // Unreachable
}

static int tokenize(char *input, char** argv, int max_args) {
    int argc = 0;
    
    while (*input && argc < max_args) {
        // Skip leading whitespace
        while (*input == ' ' || *input == '\t' || *input == '\n' || *input == '\r') {
            (void) *input++; // (void) to suppress unused value warning
        }

        // End of string
        if (!*input) break;

        // Start of argument
        argv[argc++] = input;

        // Find end of argument
        while (*input && *input != ' ' && *input != '\t' && *input != '\n' && *input != '\r') {
           (void) *input++;
        }

        // Null-terminate argument
        if (!*input) break;
        *input++ = '\0';
    }

    return argc;
}

void kernel_monitor() {
    char input[MAX_INPUT_LENGTH];
    char* argv[MAX_COMMAND_ARGS];

    while (1) {
        kprintf("tetos> ");
        uart_gets(input, sizeof(input));

        // Tokenize input
        int argc = tokenize(input, argv, MAX_COMMAND_ARGS);
        if (argc == 0) continue; // Empty input

        // Find and execute command
        int found = 0;
        for (int i = 0; i < (int) NUM_COMMANDS; i++) {
            if (strcmp(argv[0], commands[i].name) == 0) {
                commands[i].function(argc, argv);
                found = 1;
                break;
            }
        }

        if (!found) {
            kprintf("Unknown command '%s'. Type 'help' for a list of commands.\n", argv[0]);
        }

        memset(input, 0, sizeof(input)); // Clear input buffer for next command
        memset(argv, 0, sizeof(argv));   // Clear argv array for next command
    }
}