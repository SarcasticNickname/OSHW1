#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>

#define DATA_LEN 5000

const char *inputFIFO = "input_fifo.fifo";
const char *outputFIFO = "output_fifo.fifo";

bool validate_args(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Program requires two command-line arguments.\n");
        return false;
    }
    return true;
}

int main(int argc, char *argv[]) {
    char data[DATA_LEN];
    int readBytes;
    char *source, *destination;
    (void) umask(0);
    if (!validate_args(argc, argv)) {
        return 0;
    } else {
        source = argv[1];
        destination = argv[2];
    }

    mknod(inputFIFO, S_IFIFO | 0666, 0);
    mknod(outputFIFO, S_IFIFO | 0666, 0);

    int readFD, writeFD;

    int sourceFile = open(source, O_RDONLY, 0666);
    if (sourceFile < 0) {
        printf("File open error: Unable to open input file.\n");
        exit(-1);
    }
    readBytes = read(sourceFile, data, DATA_LEN);
    data[readBytes] = '\0';
    if (close(sourceFile) < 0) {
        printf("File close error: Failed to close input file.\n");
    }

    if ((readFD = open(inputFIFO, O_WRONLY)) < 0) {
        printf("FIFO open error: Failed to open input FIFO for writing.\n");
        exit(-1);
    }
    readBytes = write(readFD, data, DATA_LEN);
    if (readBytes != DATA_LEN) {
        printf("FIFO write error: Failed to write complete data to FIFO.\n");
        exit(-1);
    }
    if (close(readFD) < 0) {
        printf("FIFO close error: Failed to close the writing side of FIFO.\n");
        exit(-1);
    }

    writeFD = open(outputFIFO, O_RDONLY);
    if (writeFD < 0) {
        printf("FIFO open error: Failed to open output FIFO for reading.\n");
        exit(-1);
    }
    readBytes = read(writeFD, data, DATA_LEN);
    if (readBytes < 0) {
        printf("FIFO read error: Failed to read data from output FIFO.\n");
        exit(-1);
    }
    if (close(writeFD) < 0) {
        printf("FIFO close error: Failed to close the reading side of FIFO.\n");
        exit(-1);
    }

    int destFile = open(destination, O_WRONLY | O_CREAT, 0666);
    if (destFile < 0) {
        printf("File open error: Unable to open output file.\n");
        exit(-1);
    }
    readBytes = write(destFile, data, strlen(data));
    if (readBytes != strlen(data)) {
        printf("File write error: Failed to write all data to output file.\n");
        exit(-1);
    }
    if (close(destFile) < 0) {
        printf("File close error: Failed to close output file.\n");
    }

    unlink(inputFIFO);
    unlink(outputFIFO);
    return 0;
}
