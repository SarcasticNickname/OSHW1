#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#define BUFFER_SIZE 5000

int is_consonant(char c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) && !(c == 'a'
    || c == 'e' || c == 'i' || c == 'o' || c == 'u' || c == 'A' || c == 'E' || c == 'I' || c == 'O'
    || c == 'U');
}

void convert_consonants_to_ascii(char *str) {
    int i;
    char output[BUFFER_SIZE] = "";
    for (i = 0; str[i] != '\0'; i++) {
        if (is_consonant(str[i])) {
            char symbol[10];
            int digit = (int) str[i];
            sprintf(symbol, "%d", digit);
            strcat(output, symbol);
        } else {
            char symbol[10];
            sprintf(symbol, "%c", str[i]);
            strcat(output, symbol);
        }
    }
    memset(str, 0, BUFFER_SIZE);
    strcpy(str, output);
}

const char *pipe_first = "first_pipe.fifo";
const char *pipe_second = "second_pipe.fifo";

int main(int argc, char *argv[]) {
    char buffer[BUFFER_SIZE];
    int size;
    int first;
    int second;
    mknod(pipe_first, S_IFIFO | 0666, 0);
    mknod(pipe_second, S_IFIFO | 0666, 0);
    first = open(pipe_first, O_RDONLY);
    if (first < 0) {
        printf("ERROR: open FIFO for reading\n");
        exit(-1);
    }
    size = read(first, buffer, BUFFER_SIZE);
    if (size < 0) {
        printf("ERROR: read string from FIFO\n");
        exit(-1);
    }
    convert_consonants_to_ascii(buffer);
    if (close(first) < 0) {
        printf("ERROR: close FIFO\n");
        exit(-1);
    }
    second = open(pipe_second, O_WRONLY);
    if (second < 0) {
        printf("ERROR: open FIFO for writing\n");
        exit(-1);
    }
    size = write(second, buffer, BUFFER_SIZE);
    if (size != BUFFER_SIZE) {
        printf("ERROR: write all string to FIFO\n");
        exit(-1);
    }
    if (close(second) < 0) {
        printf("ERROR: close FIFO\n");
        exit(-1);
    }
    return 0;
}