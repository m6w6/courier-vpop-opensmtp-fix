#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <vpopmail_config.h>
#include <vauth.h>
#include <vpopmail.h>

#ifndef IMAPD
#   define IMAPD "/usr/bin/imapd"
#endif

extern char **environ;

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        printf("1 NO argc != 2\n");
        return -1;
    }

    open_smtp_relay();
    /* no need to chown vpopmail.vchkpw the open-smtp file
     * because vchkpw used by qmail-pop3d is setuid root */

    sleep(1);

    if (setgid(VPOPMAILGID) || setuid(VPOPMAILUID)) {
        printf("1 NO setuid/gid, getuid=%d\n", getuid());
        return -2;
    }

    execl(IMAPD, IMAPD, argv[1], NULL);

    printf("1 NO exec %s failed\n", IMAPD);
    return -3;
}
