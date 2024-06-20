#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>

#define DATA_CAPACITY 5000

int consonant_check(char c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) && !(c == 'a'
    || c == 'e' || c == 'i' || c == 'o' || c == 'u' || c == 'A' || c == 'E' || c == 'I' || c == 'O'
    || c == 'U');
}

void encode_consonants(char *text) {
    int index;
    char modified[DATA_CAPACITY] = "";
    for (index = 0; text[index] != '\0'; index++) {
        if (consonant_check(text[index])) {
            char num_str[10];
            sprintf(num_str, "%d", (int) text[index]);
            strcat(modified, num_str);
        } else {
            char char_str[10];
            sprintf(char_str, "%c", text[index]);
            strcat(modified, char_str);
        }
    }
    memset(text, 0, DATA_CAPACITY);
    strcpy(text, modified);
}

bool args_check(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Execution requires two arguments.\n");
        return false;
    }
    return true;
}

const char *inputFIFO = "input_fifo.fifo";
const char *outputFIFO = "output_fifo.fifo";

int main(int argc, char *argv[]) {
    char buffer[DATA_CAPACITY];
    int readCount;
    char *inputFile, *outputFile;

    if (!args_check(argc, argv)) {
        return 0;
    } else {
        inputFile = argv[1];
        outputFile = argv[2];
    }
    mknod(inputFIFO, S_IFIFO | 0666, 0);
    mknod(outputFIFO, S_IFIFO | 0666, 0);

    int readFD, writeFD = 0;
    int childProc = fork();

    if (childProc < 0) {
        printf("Fork error: Could not create child process.\n");
        exit(-1);
    } else if (childProc == 0) {
        readFD = open(inputFIFO, O_RDONLY);
        if (readFD < 0) {
            printf("FIFO error: Failed to open for reading.\n");
            exit(-1);
        }

        readCount = read(readFD, buffer, DATA_CAPACITY);

        if (readCount < 0) {
            printf("FIFO read error: Could not read data.\n");
            exit(-1);
        }
        encode_consonants(buffer);

        if (close(readFD) < 0) {
            printf("FIFO close error: Failed to close read side.\n");
            exit(-1);
        }

        writeFD = open(outputFIFO, O_WRONLY);
        if (writeFD < 0) {
            printf("FIFO error: Failed to open for writing.\n");
            exit(-1);
        }
        readCount = write(writeFD, buffer, DATA_CAPACITY);
        if (readCount != DATA_CAPACITY) {
            printf("FIFO write error: Could not write all data.\n");
            exit(-1);
        }

        if (close(writeFD) < 0) {
            printf("FIFO close error: Failed to close write side.\n");
            exit(-1);
        }
        exit(0);
    } else {
        int sourceFD = open(inputFile, O_RDONLY, 0666);
        if (sourceFD < 0) {
            printf("File open error: Could not open input file.\n");
            exit(-1);
        }
        readCount = read(sourceFD, buffer, DATA_CAPACITY);
        buffer[readCount] = '\0';
        if (close(sourceFD) < 0) {
            printf("File close error: Could not close input file.\n");
        }
        readFD = open(inputFIFO, O_WRONLY);
        if (readFD < 0) {
            printf("FIFO error: Failed to open for writing.\n");
            exit(-1);
        }
        readCount = write(readFD, buffer, DATA_CAPACITY);

        if (readCount != DATA_CAPACITY) {
            printf("FIFO write error: Could not write all data.\n");
            exit(-1);
        }

        if (close(readFD) < 0) {
            printf("FIFO close error: Failed to close write side.\n");
            exit(-1);
        }
    }
    writeFD = open(outputFIFO, O_RDONLY);
    if (writeFD < 0) {
        printf("FIFO error: Failed to open for reading.\n");
        exit(-1);
    }
    readCount = read(writeFD, buffer, DATA_CAPACITY);
    if (readCount < 0) {
        printf("FIFO read error: Could not read data.\n");
        exit(-1);
    }
    if (close(writeFD) < 0) {
        printf("FIFO close error: Failed to close read side.\n");
        exit(-1);
    }
    int destinationFD = open(outputFile, O_WRONLY | O_CREAT, 0666);
    if (destinationFD < 0) {
        printf("File open error: Could not open output file.\n");
        exit(-1);
    }
    readCount = write(destinationFD, buffer, strlen(buffer));
    if (readCount != strlen(buffer)) {
        printf("File write error: Could not write all data to file.\n");
        exit(-1);
    }
    if (close(destinationFD) < 0) {
        printf("File close error: Could not close output file.\n");
    }
    unlink(inputFIFO);
    unlink(outputFIFO);
    return 0;
}
