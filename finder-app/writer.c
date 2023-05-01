#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>

void print_usage(char *prog_name);

int main(int argc, char **argv)
{
    openlog("writer", LOG_PID | LOG_CONS | LOG_NDELAY, LOG_USER);
    syslog(LOG_INFO, "Start logging");
    FILE *fptr;

    if (argc < 3)
    {
        if ((argc == 2) && ((strcmp(argv[1], "--help") == 0) || (strcmp(argv[1], "-h") == 0)))
        {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        }
        syslog(LOG_ERR, "unexpected number of arguments: %d\n", argc);
        print_usage(argv[0]);
        closelog();
        exit(EXIT_FAILURE);
    }

    // use appropriate location if you are using MacOS or Linux
    fptr = fopen(argv[1], "w");
    if (fptr == NULL)
    {
        syslog(LOG_ERR, "No such a file or directory!\n");
        closelog();
        exit(EXIT_FAILURE);
    }

    syslog(LOG_DEBUG, "Writing %s to %s", argv[2], argv[1]);
    fprintf(fptr, "%s", argv[2]);
    fclose(fptr);
    closelog();
    return EXIT_SUCCESS;
}

void print_usage(char *prog_name)
{
    printf("this is a program to write text to certain file\nUsage:\n %s <write_file> <write_text>\n example:\n %s /tmp/aesd/assignment1/sample.txt ios\n", prog_name, prog_name);
}