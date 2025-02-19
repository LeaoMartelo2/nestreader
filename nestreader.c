/* Copyright 2025 LeaoMartelo2

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <getopt.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define LAST_LINES 5
#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))

typedef enum {
    LOG_DEBUG = 1 << 0,
    LOG_TRACE = 1 << 1,
    LOG_WARN = 1 << 2,
    LOG_ERROR = 1 << 3,
} log_types_t;

void print_usage(char *program_name) {

    printf("Usage: %s [options] <filename>\n", program_name);
    printf("Options: \n"
           "    -d, --debug         Print DEBUG messages    \n"
           "    -t, --trace         Print TRACE/LOG messages\n"
           "    -w, --warn          Print WARN messages     \n"
           "    -e, --error         Print ERROR messages    \n"
           "    -a, --all           Prints all messages     \n"
           "    -n, --no-color      Disables color at output\n"
           "    -h, --help          Prints this message     \n");
}

int main(int argc, char **argv) {

    int opt;
    log_types_t log_types = 0;
    size_t print_all = 0;
    size_t no_color = 0;
    char *program_name = argv[0];

    static struct option long_options[] = {
        {"debug", no_argument, 0, 'd'},
        {"trace", no_argument, 0, 't'},
        {"warn", no_argument, 0, 'w'},
        {"error", no_argument, 0, 'e'},
        {"all", no_argument, 0, 'a'},
        {"no-color", no_argument, 0, 'n'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0},
    };

    while ((opt = getopt_long(argc, argv, "dtweahn", long_options, NULL)) != -1) {
        switch (opt) {
        case 'd':
            log_types |= LOG_DEBUG;
            break;

        case 't':
            log_types |= LOG_TRACE;
            break;

        case 'w':
            log_types |= LOG_WARN;
            break;

        case 'e':
            log_types |= LOG_ERROR;
            break;

        case 'a':
            print_all = 1;
            break;

        case 'n':
            no_color = 1;
            break;

        case 'h':
            print_usage(program_name);
            return 0;
            break;

        default:
            print_usage(program_name);
            return 1;
            break;
        }
    }

    if (optind >= argc) {
        print_usage(program_name);
        return 1;
    }

    char *filename = argv[optind];

    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Could not open file");
        return 1;
    }

    int fd = inotify_init();
    if (fd < 0) {
        perror("File notify fail");
        return 1;
    }

    int wd = inotify_add_watch(fd, filename, IN_MODIFY);
    if (wd < 0) {
        perror("inotify_add_watch error");
        return 1;
    }

    struct stat stat_buf;
    off_t current_size;

    if (fstat(fileno(file), &stat_buf) != 0) {
        perror("Error at fstat");
        return 1;
    }
    current_size = stat_buf.st_size;

    fseek(file, 0, SEEK_END);
    off_t end_pos = ftell(file);
    off_t start_pos = end_pos - (LAST_LINES * BUFFER_SIZE);

    if (start_pos < 0) {
        start_pos = 0;
    }

    fseek(file, start_pos, SEEK_SET);
    char buffer[BUFFER_SIZE];

    /* while (fgets(buffer, BUFFER_SIZE, file)) {
        if (print_all ||
            (log_types & LOG_DEBUG && strstr(buffer, "DEBUG") != NULL) ||
            (log_types & LOG_TRACE && strstr(buffer, "LOG") != NULL) ||
            (log_types & LOG_WARN && strstr(buffer, "WARN") != NULL) ||
            (log_types & LOG_ERROR && strstr(buffer, "ERROR") != NULL)) {

            if (strstr(buffer, "DEBUG") != NULL) {
                printf("\033[34m%s\033[0m", buffer);
            } else if (strstr(buffer, "LOG") != NULL) {
                printf("\033[32m%s\033[0m", buffer);
            } else if (strstr(buffer, "WARN") != NULL) {
                printf("\033[33m%s\033[0m", buffer);
            } else if (strstr(buffer, "ERROR") != NULL) {
                printf("\033[31m%s\033[0m", buffer);
            } else {
                printf("%s", buffer);
            }
        }
    } */

    char event_buf[EVENT_BUF_LEN];

    while (1) {
        int len = read(fd, event_buf, EVENT_BUF_LEN);
        if (len < 0) {
            perror("Error reading event buffer");
            return 1;
        }

        int i = 0;
        while (i < len) {
            struct inotify_event *event = (struct inotify_event *)&event_buf[i];
            if (event->mask & IN_MODIFY) {
                // File has been modified, print new lines
                fseek(file, current_size, SEEK_SET);
                while (fgets(buffer, BUFFER_SIZE, file)) {
                    if (print_all ||
                        (log_types & LOG_DEBUG && strstr(buffer, "DEBUG") != NULL) ||
                        (log_types & LOG_TRACE && strstr(buffer, "TRACE") != NULL) ||
                        (log_types & LOG_WARN && strstr(buffer, "WARN") != NULL) ||
                        (log_types & LOG_ERROR && strstr(buffer, "ERROR") != NULL)) {

                        if (no_color) {
                            printf("%s", buffer);
                        } else if (strstr(buffer, "DEBUG") != NULL) {
                            printf("\033[34m%s\033[0m", buffer);
                        } else if (strstr(buffer, "LOG") != NULL) {
                            printf("\033[32m%s\033[0m", buffer);
                        } else if (strstr(buffer, "WARN") != NULL) {
                            printf("\033[33m%s\033[0m", buffer);
                        } else if (strstr(buffer, "ERROR") != NULL) {
                            printf("\033[31m%s\033[0m", buffer);
                        } else {
                            printf("%s", buffer);
                        }
                    }
                }
                struct stat stat_buf;
                if (fstat(fileno(file), &stat_buf) != 0) {
                    perror("fstat");
                    return 1;
                }
                current_size = stat_buf.st_size;
            }
            i += sizeof(struct inotify_event) + event->len;
        }
    }

    fclose(file);
    close(fd);
    return 0;
}
