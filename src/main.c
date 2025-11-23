#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"
#include <stdbool.h>

#define SRC_TARGET_FILE "/var/lpass/target"

char* get_target_file() {
    FILE *file = fopen(SRC_TARGET_FILE, "r");
    if (file == NULL) {
        perror("fopen");
        return NULL;
    }
    char buffer[256];
    if (fgets(buffer, sizeof(buffer), file) == NULL) {
        fclose(file);
        return NULL;
    }
    fclose(file);

    // Remove newline if present
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
        len--;
    }

    // Allocate and return
    char* result = malloc(len + 1);
    if (result != NULL) {
        strcpy(result, buffer);
    }
    return result;
}

sqlite3* get_database() {
    char* db_path = get_target_file();
    if (db_path == NULL) {
        return NULL;
    }

    sqlite3 *db;
    int rc = sqlite3_open(db_path, &db);
    free(db_path); // always free, even if open fails

    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }

    return db;
}

// Copy to clipboard using xclip
void copy_to_clipboard(const char *text) {
    FILE *pipe = popen("xclip -selection clipboard", "w");
    if (pipe == NULL) {
        fprintf(stderr, "Error: Could not open xclip. Make sure xclip is installed.\n");
        return;
    }
    fprintf(pipe, "%s", text);
    pclose(pipe);
}

// Get field from database by name
char* get_field(const char *name, const char *field) {
    sqlite3 *db = get_database();
    if (db == NULL) {
        return NULL;
    }

    char query[512];
    snprintf(query, sizeof(query), "SELECT %s FROM passwords WHERE name = ?", field);

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }

    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);

    char *result = NULL;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *text = sqlite3_column_text(stmt, 0);
        if (text != NULL) {
            result = strdup((const char*)text);
        }
    } else {
        fprintf(stderr, "No entry found with name: %s\n", name);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

void get_pw(int argc, char *argv[]) {
    if (argc < 2 || argv[1] == NULL) {
        fprintf(stderr, "Error: -pw requires a name argument\n");
        fprintf(stderr, "Usage: lpass <name> -pw\n");
        return;
    }
    const char *name = argv[1];
    char *password = get_field(name, "password");
    if (password != NULL) {
        copy_to_clipboard(password);
        printf("Password copied to clipboard\n");
        free(password);
    }
}

void get_otp(int argc, char *argv[]) {
    if (argc < 2 || argv[1] == NULL) {
        fprintf(stderr, "Error: -otp requires a name argument\n");
        fprintf(stderr, "Usage: lpass <name> -otp\n");
        return;
    }
    const char *name = argv[1];
    char *otp = get_field(name, "otp");
    if (otp != NULL) {
        copy_to_clipboard(otp);
        printf("OTP copied to clipboard\n");
        free(otp);
    }
}

void get_email(int argc, char *argv[]) {
    if (argc < 2 || argv[1] == NULL) {
        fprintf(stderr, "Error: -email requires a name argument\n");
        fprintf(stderr, "Usage: lpass <name> -email\n");
        return;
    }
    const char *name = argv[1];
    char *email = get_field(name, "email");
    if (email != NULL) {
        copy_to_clipboard(email);
        printf("Email copied to clipboard\n");
        free(email);
    }
}

// Function implementations
void help(int argc, char *argv[]) {
    printf("Usage: lpass <name> [OPTION]\n");
    printf("       lpass -init <path>\n");
    printf("       lpass -set-target <path>\n");
    printf("\nOptions:\n");
    printf("  -help -h           Show this help message\n");
    printf("  -pw                Get password and copy to clipboard\n");
    printf("  -otp               Get OTP and copy to clipboard\n");
    printf("  -email             Get email and copy to clipboard\n");
    printf("  -init <path>       Create a new database at the specified path\n");
    printf("  -set-target <path> Set an existing database as the target\n");
}

// Set the target file to point to an existing database
void set_target(int argc, char *argv[]) {
    if (argc < 3 || argv[2] == NULL) {
        fprintf(stderr, "Error: -set-target requires a path argument\n");
        fprintf(stderr, "Usage: lpass -set-target <path>\n");
        return;
    }
    const char *db_path = argv[2];
    FILE *file = fopen(SRC_TARGET_FILE, "w");
    if (file == NULL) {
        perror("Error: Could not write to target file");
        return;
    }
    fprintf(file, "%s", db_path);
    fclose(file);
    printf("Target database set to: %s\n", db_path);
}

// Create a new database at the given path with the passwords table
void create_database(int argc, char *argv[]) {
    if (argc < 3 || argv[2] == NULL) {
        fprintf(stderr, "Error: -init requires a path argument\n");
        fprintf(stderr, "Usage: lpass -init <path>\n");
        return;
    }
    const char *db_path = argv[2];
    sqlite3 *db;
    int rc = sqlite3_open(db_path, &db);
    
    if (rc) {
        fprintf(stderr, "Can't create database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Create the passwords table
    const char *sql = 
        "CREATE TABLE IF NOT EXISTS passwords ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL UNIQUE, "
        "password TEXT, "
        "email TEXT, "
        "otp TEXT, "
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");";

    char *err_msg = NULL;
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return;
    }

    sqlite3_close(db);
    printf("Database created at: %s\n", db_path);
    
    // Automatically set as target
    set_target(argc, argv);
}

// Command map structure
typedef struct {
    const char *flag;
    void (*function)(int argc, char *argv[]);
} CommandMap;

// Define the command map
CommandMap secondary_commands[] = {
    {"-pw", get_pw},
    {"-otp", get_otp},
    {"-email", get_email},
    {NULL, NULL}
};

CommandMap primary_commands[] = {
    {"-init", create_database},
    {"-set-target", set_target},
    {"-help", help},
    {"-h", help},
    {NULL, NULL}
};

CommandMap* get_command(CommandMap *commands, const char *flag) {
    for (int i = 0; commands[i].flag != NULL; i++) {
        if (strcmp(commands[i].flag, flag) == 0) {
            return &commands[i];
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        help(argc, argv);
        return 1;
    }

    // Check for primary commands (format: lpass -init <path>)
    CommandMap *command = get_command(primary_commands, argv[1]);
    if (command != NULL) {
        command->function(argc, argv);
        return 0;
    }

    // Check for secondary commands (format: lpass Github -pw)
    if (argc < 3) {
        fprintf(stderr, "Error: Missing arguments\n");
        help(argc, argv);
        return 1;
    }

    command = get_command(secondary_commands, argv[2]);
    if (command != NULL) {
        command->function(argc, argv);
        return 0;
    }

    // No valid command found
    fprintf(stderr, "Unknown option: %s\n", argv[2]);
    fprintf(stderr, "Use -help for help.\n");
    return 1;
}
