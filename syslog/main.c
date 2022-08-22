/*
# To see full logs of ProgramName in bash
# With boot information
journalctl -t "ProgramName"
# Without boot information
journalctl -q -t "ProgramName"
*/

#include <syslog.h>
#include <unistd.h> // getuid

int main(void)
{
    openlog("ProgramName", LOG_PID, LOG_USER);
    syslog(LOG_INFO, "Start logging by user %d", getuid());
    closelog();
}

