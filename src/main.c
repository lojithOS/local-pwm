#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"

#define SRC_TARGET_FILE "/var/local-pwm/target"

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

void get_pw() {

}
void get_otp() {
  
}
void write_target() {
	FILE *file = fopen(SRC_TARGET_FILE, "w");
	if (file) {
		fprint(file, "%s", path_to_db_file);
		fclose(file);
	}
}
int main() {

}
