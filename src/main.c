#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>

#define SRC_TARGET_FILE "/var/local-pwm/target"

char* get_target_file() {
	FILE *file = fopen(SRC_TARGET_FILE, "r");
	if (file == NULL) {
		perror("Target file missing.\nIt's created when assigning a database file.");
		return;
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
void get_database() {
	char* db_path = get_target_file();
	if (db_path == NULL) {
		return;
	}
	sqlite3_open(db_path);
	free(db_path);
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
