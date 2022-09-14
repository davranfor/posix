/*
gcc -std=c11 -Wpedantic -Wall -Wextra -Wconversion -Wcast-qual -o myecho main.c
# Examples:
./myecho --output=output.txt "Hello world!"
./myecho -n 5 "Hello world!"
./myecho "Hello world!"
*/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

static void print_usage(const char *path)
{
    printf("usage: %s [on] text\n", path);
    exit(EXIT_FAILURE);
}

static void print_version(void)
{
    printf("myecho [Version 1.0]\n");
    exit(EXIT_SUCCESS);
}

static void print_help(void)
{
    printf("myecho\n"
            "  -o, --output[=FILENAME]\tSave text to a file\n"
            "  -n, --ntimes[=NTIMES]\t\tPrint text n times\n"
            "  -s, --silent\t\t\tDon't show errors\n"
            "      --version\t\t\tShow the program version and exit\n"
            "      --help\t\t\tShow this text and exit\n"
    );
    exit(EXIT_SUCCESS);
}

static void echo(FILE *file, const char *text, int ntimes)
{
    for (int i = 0; i < ntimes; i++)
    {
        fprintf(file, "%s\n", text);
    }
}

int main(int argc, char *argv[])
{
    const char *output = NULL;
    const char *text = NULL;
    int ntimes = 1;
    int silent = 0;

    struct option long_options[] =
    {
        { "version", no_argument, NULL, 'v' },
        { "help", no_argument, NULL, 'h' },
        { "silent", no_argument, NULL, 's' },
        { "output", required_argument, NULL, 'o' },
        { "ntimes", required_argument, NULL, 'n' },
        { 0, 0, 0, 0 }
    };
    int opt = 0;

    while ((opt = getopt_long(argc, argv, "-so:n:", long_options, NULL)) != -1)
    {
        switch (opt)
        {
            // Handle non-option arguments here if you put a `-`
            // at the beginning of getopt_long's 3rd argument
            case 1:
                text = optarg;
                break;
            case 'v':
                print_version();
                break;
            case 'h':
                print_help();
                break;
            case 's':
                silent = 1;
                break;
            case 'o':
                output = optarg;
                break;
            case 'n':
                ntimes = atoi(optarg);
                break;
            case '?':
            default:
                // Set "opterr = 0;" before calling getopt_long to skip getopt errors
                print_usage(argv[0]);
                break;
        }
    }
    if (text == NULL)
    {
        print_usage(argv[0]);
    }

    FILE *file = stdout;

    if (output != NULL)
    {
        file = fopen(output, "w");
        if (file == NULL)
        {
            if (!silent)
            {
                perror("fopen");
            }
            exit(EXIT_FAILURE);
        }
    }
    if ((ntimes < 0) || (ntimes > 1000))
    {
        ntimes = 1;
    }
    echo(file, text, ntimes);
    if (output != NULL)
    {
        fclose(file);
    }
    return 0;
}

