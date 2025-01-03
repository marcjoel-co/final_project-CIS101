/*a simple file encryption utility that reads a file, encrypts its contents using a 
 * modulo-based transformation, and saves the result to a new file.
 */
#include <stdio.h>  
#include <string.h> 
#include <ctype.h>  
#include <dirent.h> // For directory traversal functions like readdir(), and closedir()
#include <time.h>   // For time-related functions like time(), ctime(), and localtime()


// Constants for configuration
#define MAX_FILENAME 256            //maximum file name
#define BUFFER_SIZE 4096            // buffer size
#define MAX_PEG 255                 // Maximum encryption shift value
#define MIN_PEG 0                   // we want positive values
#define HISTORY_FILE "history.md"   // A different kind of text file

// Function Declarations

//Main functionality
int encrypt_file(const char *input_file, const char *output_file, int pegs);      // encryption loop
int decrypt_file(const char *input_file, const char *output_file, int pegs);      // same as encryption although subtracts 
void handle_file_error(const char *filename);                                     // handles errors

//for validation
int safe_input(char *buffer, int size);                                           // checks if input is valid
int validate_file(const char *filename);                                          // check if file exist and is not empty
int has_txt_extension(const char *filename);                                      // helper function for validation
int validate_peg_value(int peg);                                                  // validate if 0 to 255 
void clear_stdin(void);                                                           // clears input buffer

//auxiliary functions
void log_encryption(const char *input_file, const char *output_file, int pegs);   // function to view history
void read_file(const char *filename);                                             // read files
void view_history(void);                                                          // view previous records of files
void search_files(void);                                                          // search file in directory      


int main() {
    char input_file[MAX_FILENAME], output_file[MAX_FILENAME];
    int pegs, choice;

    while (1) {
        // Menu
        printf("\n--- Caesar Cipher Utility ---\n");
        printf("1. Encrypt File\n");                        // Encrypt file
        printf("2. Decrypt File\n");                        // Decrypt file (negative shifts)
        printf("3. Read File\n");                           // Read a specific file 
        printf("4. Search Files\n");                        // Search for files in the current directory
        printf("5. Encryption History\n");                  // Veiew history
        printf("6. Exit\n");                                // Exit the program
        printf("Enter your choice: ");

        // check if choice is valid
        if (scanf("%d", &choice) != 1) {
            clear_stdin();
            printf("Invalid input. Please try again.\n");
            continue;
        }

        clear_stdin();

        switch (choice) {
            case 1:
                // Encryption
                printf("Enter input filename: ");
                if (!safe_input(input_file, sizeof(input_file))) { // check if filename is acceptable
                    printf("Input error. Try again.\n");
                    continue;
                }

                if (!validate_file(input_file)) {                  // check if file content exist and valid
                    printf("Input file validation failed.\n");
                    continue;
                }

                printf("Enter output filename: ");
                if (!safe_input(output_file, sizeof(output_file))) {
                    printf("Input error. Try again.\n");
                    continue;
                }

                if (!has_txt_extension(output_file)) {
                    printf("Error: File '%s' must have a .txt extension.\n", output_file);
                    continue;
                }

                if (strcmp(input_file, output_file) == 0) {
                    printf("Input and output files must not be the same. Try again.\n");
                    continue;
                }

                printf("Enter number of pegs (%d to %d): ", MIN_PEG, MAX_PEG);
                if (scanf("%d", &pegs) != 1 || !validate_peg_value(pegs)) {
                    clear_stdin();
                    printf("Invalid peg value. Must be between %d and %d.\n", MIN_PEG, MAX_PEG);
                    continue;
                }

                if (encrypt_file(input_file, output_file, pegs)) {
                    log_encryption(input_file, output_file, pegs);
                } else {
                    printf("Encryption failed.\n");
                }
                break;

            case 2:
                // Decryption
                printf("Enter input filename: ");
                if (!safe_input(input_file, sizeof(input_file))) {
                    printf("Input error. Try again.\n");
                    continue;
                }

                if (!validate_file(input_file)) {
                    printf("Input file validation failed.\n");
                    continue;
                }

                printf("Enter output filename: ");
                if (!safe_input(output_file, sizeof(output_file))) {
                    printf("Input error. Try again.\n");
                    continue;
                }

                if (!has_txt_extension(output_file)) {
                    printf("Error: File '%s' must have a .txt extension.\n", output_file);
                    continue;
                }

                if (strcmp(input_file, output_file) == 0) {
                    printf("Input and output files must not be the same. Try again.\n");
                    continue;
                }

                printf("Enter number of pegs (%d to %d): ", MIN_PEG, MAX_PEG);
                if (scanf("%d", &pegs) != 1 || !validate_peg_value(pegs)) {
                    clear_stdin();
                    printf("Invalid peg value. Must be between %d and %d.\n", MIN_PEG, MAX_PEG);
                    continue;
                }

                if (decrypt_file(input_file, output_file, pegs)) {
                    printf("Decryption successful.\n");
                } else {
                    printf("Decryption failed.\n");
                }
                break;

            case 3:
                // Read file
                printf("Enter filename to read: ");
                if (safe_input(input_file, sizeof(input_file))) {
                    read_file(input_file);
                }
                break;

            case 4:
                // Search files
                search_files();
                break;

            case 5:
                // View history
                view_history();
                break;

            case 6:
                // Exit
                return 0;

            default:
                printf("Invalid choice. Try again.\n");
                break;
        }
    }
}

// Safely reads filename into a buffer 
int safe_input(char *buffer, int size) {
    if (fgets(buffer, size, stdin) == NULL) {
        return 0;
    }
    buffer[strcspn(buffer, "\n")] = 0;
    return 1;
}

// Clears input buffer
void clear_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Handles file errors with informative messages
void handle_file_error(const char *filename) {
    printf("Error: Unable to open or process file '%s'.\n"
           "Ensure the file exists and you have the necessary permissions.\n", 
           filename);
}

// Validates file existence, non-emptiness, and .txt extension
int validate_file(const char *filename) {
    // First check if the file has .txt extension
    if (!has_txt_extension(filename)) {
        printf("Error: File '%s' must have a .txt extension.\n", filename);
        return 0;
    }

    // Try to open the file
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Error: Cannot open file '%s'. Check if the file exists.\n", filename);
        return 0;
    }

    // Check if file is empty
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fclose(file);

    if (file_size == 0) {
        printf("Error: File '%s' is empty.\n", filename);
        return 0;
    }

    return 1;
}

// check if the peg is within the range
int validate_peg_value(int peg) {

    // check if input is from 0 to 256 
    return (peg >= MIN_PEG && peg <= MAX_PEG);
}

// main encryption 
int encrypt_file(const char *input_file, const char *output_file, int pegs) {
    FILE *in = NULL, *out = NULL;
    unsigned char buffer[BUFFER_SIZE];
    size_t bytes_read;
    int success = 0;

    // Open input and output files
    in = fopen(input_file, "rb");
    if (in == NULL) {
        printf("Error opening input file: %s\n", input_file);
        return 0;
    }

    out = fopen(output_file, "wb");
    if (out == NULL) {
        printf("Error opening output file: %s\n", output_file);
        fclose(in);
        return 0;
    }

    printf("Encrypting %s -> %s (Pegs: %d)\n", input_file, output_file, pegs);

    // Process file in chunks with direct peg value
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, in)) > 0) { //4096
        for (size_t i = 0; i < bytes_read; i++) {  
            buffer[i] = (buffer[i] + pegs) % 256;

        }

        if (fwrite(buffer, 1, bytes_read, out) != bytes_read) { // 
            printf("Write error occurred during encryption.\n");
            goto cleanup;
    }
        }

    success = 1;
    printf("File encrypted successfully.\n");

cleanup:
    if (in) fclose(in);
    if (out) fclose(out);
    return success;
}

// decryption (just like encryption but reverse)
int decrypt_file(const char *input_file, const char *output_file, int pegs) {
    FILE *in = NULL, *out = NULL;
    unsigned char buffer[BUFFER_SIZE];
    size_t bytes_read;
    int success = 0;

    // Open input and output files
    in = fopen(input_file, "rb");
    if (in == NULL) {
        printf("Error opening input file: %s\n", input_file);
        return 0;
    }

    out = fopen(output_file, "wb");
    if (out == NULL) {
        printf("Error opening output file: %s\n", output_file);
        fclose(in);
        return 0;
    }

    printf("Decrypting %s -> %s (Pegs: %d)\n", input_file, output_file, pegs);

    // Process file in chunks with reversed peg value
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, in)) > 0) {
        for (size_t i = 0; i < bytes_read; i++) {
            buffer[i] = (buffer[i] - pegs + 256) % 256; // Ensure result is non-negative
        }

        if (fwrite(buffer, 1, bytes_read, out) != bytes_read) {
            printf("Write error occurred during decryption.\n");
            goto cleanup;
        }
    }

    success = 1;
    printf("File decrypted successfully.\n");

cleanup:
    if (in) fclose(in);
    if (out) fclose(out);
    return success;
}

// Displays file contents
void read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        handle_file_error(filename);
        return;
    }

    char buffer[BUFFER_SIZE];
    printf("\nFile contents:\n");
    printf("-------------------\n");
    
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        printf("%s", buffer);
    }
    printf("\n-------------------\n");
    
    fclose(file);
}

// Lists files in current directory
void search_files(void) {
    DIR *dir; // struct type
    struct dirent *entry; //each entry
    int count = 0;

    dir = opendir("."); // "." represent current directory
    if (dir == NULL) {  // make sure that the directory exist 
        printf("Error: Cannot open current directory.\n");
        return;
    }

    printf("\nFiles in current directory:\n");
    printf("-------------------\n");
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {  //d_type is a type of file
            printf("%s\n", entry->d_name);
            count++;
        }
    }
    
    printf("-------------------\n");
    printf("Total files: %d\n", count);
    
    closedir(dir);
}

// Displays encryption history
void view_history(void) {
    FILE *file = fopen(HISTORY_FILE, "r");
    if (file == NULL) {
        printf("No encryption history found.\n");
        return;
    }

    char buffer[BUFFER_SIZE];
    printf("\nEncryption History:\n");
    printf("-------------------\n");
    
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        printf("%s", buffer);
    }
    printf("-------------------\n");
    
    fclose(file);
}

// Logs encryption operations with timestamp
void log_encryption(const char *input_file, const char *output_file, int pegs) {
    FILE *file = fopen(HISTORY_FILE, "a");
    if (file == NULL) {
        printf("Warning: Could not log encryption history.\n");
        return;
    }

    time_t now;
    time(&now);
    char *date = ctime(&now);
    date[strlen(date) - 1] = '\0';  // Remove newline

    fprintf(file, "%s -> %s (pegs: %d) | %s\n", 
           input_file, output_file, pegs, date);
    
    fclose(file);
}

//check for a .txt file
int has_txt_extension(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (dot == NULL) {
        return 0;  // 
    }
    return strcmp(dot, ".txt") == 0;
}