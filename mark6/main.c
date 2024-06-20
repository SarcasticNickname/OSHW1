#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>

#define DATA_SIZE 5000

int is_consonant(char c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) && !(c == 'a'
    || c == 'e' || c == 'i' || c == 'o' || c == 'u' || c == 'A' || c == 'E' || c == 'I' || c == 'O'
    || c == 'U');
}

void transform_consonants(char *line) {
    int i;
    char transformed[DATA_SIZE] = "";
    for (i = 0; line[i] != '\0'; i++) {
        if (is_consonant(line[i])) {
            char code[10];
            sprintf(code, "%d", (int) line[i]);
            strcat(transformed, code);
        } else {
            char nonConsonant[10];
            sprintf(nonConsonant, "%c", line[i]);
            strcat(transformed, nonConsonant);
        }
    }
    memset(line, 0, DATA_SIZE);
    strcpy(line, transformed);
}

bool validate_args(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Program usage error: Please provide exactly two arguments.\n");
        return false;
    }
    return true;
}

int main(int argc, char *argv[]) {
    char dataBuffer[DATA_SIZE];
    int readSize;
    char *inputPath, *outputPath;

    if (!validate_args(argc, argv)) {
        return 0;
    } else {
        inputPath = argv[1];
        outputPath = argv[2];
    }

    int pipeIn[2], pipeOut[2];

    if (pipe(pipeIn) < 0 || pipe(pipeOut) < 0) {
        printf("Pipe creation error: Failed to open necessary pipes.\n");
        exit(-1);
    }

    int pid = fork();

    if (pid < 0) {
        printf("Process creation error: Failed to fork child process.\n");
        exit(-1);
    } else if (pid == 0) {
        close(pipeIn[1]);
        readSize = read(pipeIn[0], dataBuffer, DATA_SIZE);
        if (readSize < 0) {
            printf("Pipe read error: Failed to read data from pipe.\n");
            exit(-1);
        }
        dataBuffer[readSize] = '\0';
        transform_consonants(dataBuffer);
        close(pipeIn[0]);
        
        write(pipeOut[1], dataBuffer, DATA_SIZE);
        close(pipeOut[1]);
        exit(0);
    } else {
        close(pipeIn[0]);
        int fileDescriptor;

        if ((fileDescriptor = open(inputPath, O_RDONLY)) < 0) {
            printf("File open error: Failed to open input file.\n");
            exit(-1);
        }
        readSize = read(fileDescriptor, dataBuffer, DATA_SIZE);
        dataBuffer[readSize] = '\0';
        close(fileDescriptor);

        write(pipeIn[1], dataBuffer, DATA_SIZE);
        close(pipeIn[1]);
    }

    while (wait(NULL) > 0);  // Wait for child process to terminate

    close(pipeOut[1]);
    readSize = read(pipeOut[0], dataBuffer, DATA_SIZE);
    if (readSize < 0) {
        printf("Pipe read error: Failed to read final data from output pipe.\n");
        exit(-1);
    }
    close(pipeOut[0]);

    int outputFile = open(outputPath, O_WRONLY | O_CREAT, 0666);
    if (outputFile < 0) {
        printf("File open error: Failed to open output file.\n");
        exit(-1);
    }
    if (write(outputFile, dataBuffer, strlen(dataBuffer)) != strlen(dataBuffer)) {
        printf("File write error: Failed to write all transformed data to file.\n");
        exit(-1);
    }
    close(outputFile);
    return 0;
}
