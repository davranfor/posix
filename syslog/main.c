/*
# To read entries of ProgramName in bash
# With boot information
journalctl -t "ProgramName"
# Without boot information
journalctl -q -t "ProgramName"
-------------------------------------------------------
# To write entries of ProgramName in bash
logger -p 'user.info' -t "ProgramName" "Test from bash"
*/

#include <syslog.h>
#include <unistd.h> // getuid

int main(void)
{
    openlog("ProgramName", LOG_PID, LOG_USER);
    syslog(LOG_INFO, "Start logging by user %d", getuid());
    closelog();
}

