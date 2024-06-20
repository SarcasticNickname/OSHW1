#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>

#define BUF_SIZE 5000

// Checks if the character is a consonant.
int is_consonant(char ch) {
    return ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) && !(ch == 'a'
    || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u' || ch == 'A' || ch == 'E' || ch == 'I' || ch == 'O'
    || ch == 'U');
}

// Replaces all consonants in a string with their ASCII codes.
void encode_consonants(char *text) {
    int idx;
    char encoded[BUF_SIZE] = "";
    for (idx = 0; text[idx] != '\0'; idx++) {
        if (is_consonant(text[idx])) {
            char code[10];
            int num = (int) text[idx];
            sprintf(code, "%d", num);
            strcat(encoded, code);
        } else {
            char code[10];
            sprintf(code, "%c", text[idx]);
            strcat(encoded, code);
        }
    }
    // Clears the original string and copies the processed string back.
    memset(text, 0, BUF_SIZE);
    strcpy(text, encoded);
}

// Verifies correct usage of the program.
bool validate_usage(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage error: Two command line arguments are required\n");
        return false;
    }
    return true;
}

int main(int argc, char *argv[]) {
    char buffer[BUF_SIZE];
    int bytesRead;
    char *sourceFile, *destinationFile;

    if (!validate_usage(argc, argv)) {
        return 0;
    } else {
        sourceFile = argv[1];
        destinationFile = argv[2];
    }

    int pipe1[2], pipe2[2];

    if (pipe(pipe1) < 0) {
        printf("Pipe error: Unable to open pipe\n");
        exit(-1);
    }

    if (pipe(pipe2) < 0) {
        printf("Pipe error: Unable to open pipe\n");
        exit(-1);
    }

    int child1 = fork();

    if (child1 < 0) {
        printf("Fork error: Failed to fork process\n");
        exit(-1);
    } else if (child1 == 0) {
        int child2 = fork();

        if (child2 < 0) {
            printf("Fork error: Failed to fork process\n");
            exit(-1);
        } else if (child2 == 0) {
            close(pipe2[1]);
            bytesRead = read(pipe2[0], buffer, BUF_SIZE);
            if (bytesRead < 0) {
                printf("Read error: Failed to read from pipe\n");
                exit(-1);
            }
            close(pipe2[0]);
            int outFile = open(destinationFile, O_WRONLY | O_CREAT, 0666);
            if (outFile < 0) {
                printf("File error: Failed to open output file\n");
                exit(-1);
            }
            bytesRead = write(outFile, buffer, strlen(buffer));
            if (bytesRead != strlen(buffer)) {
                printf("Write error: Failed to write all data to file\n");
                exit(-1);
            }
            close(outFile);
        } else {
            close(pipe1[1]);
            bytesRead = read(pipe1[0], buffer, BUF_SIZE);
            if (bytesRead < 0) {
                printf("Read error: Failed to read from pipe\n");
                exit(-1);
            }
            printf(buffer);
            encode_consonants(buffer);
            close(pipe1[0]);
            write(pipe2[1], buffer, BUF_SIZE);
            close(pipe2[1]);
        }
    } else {
        close(pipe1[0]);
        int inFile = open(sourceFile, O_RDONLY, 0666);
        if (inFile < 0) {
            printf("File error: Failed to open input file\n");
            exit(-1);
        }
        bytesRead = read(inFile, buffer, BUF_SIZE - 1);
        buffer[bytesRead] = '\0';
        close(inFile);
        write(pipe1[1], buffer, BUF_SIZE);
        close(pipe1[1]);
    }
    return 0;
}
