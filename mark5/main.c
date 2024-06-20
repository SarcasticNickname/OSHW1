#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>

#define BUF_SIZE 5000

int is_consonant(char ch) {
    return ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) && !(ch == 'a'
    || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u' || ch == 'A' || ch == 'E' || ch == 'I' || ch == 'O'
    || ch == 'U');
}

void transform_consonants(char *txt) {
    int pos;
    char processed[BUF_SIZE] = "";
    for (pos = 0; txt[pos] != '\0'; pos++) {
        if (is_consonant(txt[pos])) {
            char ascii_code[10];
            sprintf(ascii_code, "%d", (int) txt[pos]);
            strcat(processed, ascii_code);
        } else {
            char non_consonant[10];
            sprintf(non_consonant, "%c", txt[pos]);
            strcat(processed, non_consonant);
        }
    }
    memset(txt, 0, BUF_SIZE);
    strcpy(txt, processed);
}

bool validate_args(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage error: Program requires exactly two arguments.\n");
        return false;
    }
    return true;
}

const char *input_pipe = "input_fifo.fifo";
const char *output_pipe = "output_fifo.fifo";

int main(int argc, char *argv[]) {
    char data[BUF_SIZE];
    int count;
    char *source, *target;

    if (!validate_args(argc, argv)) {
        return 0;
    } else {
        source = argv[1];
        target = argv[2];
    }

    mknod(input_pipe, S_IFIFO | 0666, 0);
    mknod(output_pipe, S_IFIFO | 0666, 0);

    int read_pipe, write_pipe;
    int child_pid = fork();

    if (child_pid < 0) {
        printf("Fork error: Failed to create child process.\n");
        exit(-1);
    } else if (child_pid == 0) {
        int child_result = fork();
        if (child_result < 0) {
            printf("Fork error: Failed to fork secondary process.\n");
            exit(-1);
        } else if (child_result == 0) {
            read_pipe = open(output_pipe, O_RDONLY);
            if (read_pipe < 0) {
                printf("FIFO error: Failed to open output pipe for reading.\n");
                exit(-1);
            }
            count = read(read_pipe, data, BUF_SIZE);
            if (count < 0) {
                printf("FIFO error: Failed to read from output pipe.\n");
                exit(-1);
            }
            close(read_pipe);
            int file_descriptor = open(target, O_WRONLY | O_CREAT, 0666);
            if (file_descriptor < 0) {
                printf("File error: Failed to open output file.\n");
                exit(-1);
            }
            count = write(file_descriptor, data, strlen(data));
            if (count != strlen(data)) {
                printf("File error: Failed to write complete data to file.\n");
                exit(-1);
            }
            close(file_descriptor);
            unlink(input_pipe);
            unlink(output_pipe);
        } else {
            write_pipe = open(input_pipe, O_RDONLY);
            if (write_pipe < 0) {
                printf("FIFO error: Failed to open input pipe for reading.\n");
                exit(-1);
            }
            count = read(write_pipe, data, BUF_SIZE);
            if (count < 0) {
                printf("FIFO error: Failed to read from input pipe.\n");
                exit(-1);
            }
            transform_consonants(data);
            close(write_pipe);
            read_pipe = open(output_pipe, O_WRONLY);
            if (read_pipe < 0) {
                printf("FIFO error: Failed to open output pipe for writing.\n");
                exit(-1);
            }
            count = write(read_pipe, data, BUF_SIZE);
            if (count != BUF_SIZE) {
                printf("FIFO error: Failed to write all data to output pipe.\n");
                exit(-1);
            }
            close(read_pipe);
        }
    } else {
        int file_read;
        if ((file_read = open(source, O_RDONLY)) < 0) {
            printf("File error: Failed to open input file.\n");
            exit(-1);
        }
        count = read(file_read, data, BUF_SIZE);
        data[count] = '\0';
        close(file_read);
        write_pipe = open(input_pipe, O_WRONLY);
        if (write_pipe < 0) {
            printf("FIFO error: Failed to open input pipe for writing.\n");
            exit(-1);
        }
        count = write(write_pipe, data, BUF_SIZE);
        if (count != BUF_SIZE) {
            printf("FIFO error: Failed to write all data to input pipe.\n");
            exit(-1);
        }
        close(write_pipe);
    }
    return 0;
}
