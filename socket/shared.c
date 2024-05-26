#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "shared.h"

int unblock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);

    if (flags == -1)
    {
        return -1;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

uint16_t string_to_uint16(const char *str)
{
    char *end;
    unsigned long result = strtoul(str, &end, 10);

    if ((result > 65535) || (end[strspn(end, " \f\n\r\t\v")] != '\0'))
    {
        return 0;
    }
    return (uint16_t)result;
}

