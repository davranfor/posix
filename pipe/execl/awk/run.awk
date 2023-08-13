#!/usr/bin/awk -f

BEGIN {

    RS = "\v"; # Record separator or delimiter for getline()

    if (ARGC != 3)
    {
        print("Usage:", ARGV[0], "<fd read> <fd write>");
        exit(1);
    }

    fdr = sprintf("/dev/fd/%s", ARGV[1]);
    fdw = sprintf("/dev/fd/%s", ARGV[2]);

    count = 0;
    while ((getline < fdr) > 0)
    {
        printf("%02d | %2d * %2d = %d\v", count++, $1, $2, $1 * $2) > fdw;
    }

}
