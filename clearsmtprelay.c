#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <vpopmail_config.h>
#include <vauth.h>
#include <vpopmail.h>
#include <fcntl.h>

extern int get_write_lock(FILE*);
extern int lock_reg(int,int,int,off_t,int,off_t);
#define unlock(F) lock_reg(fileno(F), F_SETLK, F_UNLCK, 0, SEEK_SET, 0)

int main(int argc, const char *argv[]) {
    FILE *fs_temp, *fs_smtp, *fs_lock = fopen(OPEN_SMTP_LOK_FILE, "w+");
    time_t clear = RELAY_CLEAR_MINUTES * 60, now = time(NULL);
    int cc = 0, rc = 0;

    if (!fs_lock) {
        return -1;
    }
    if (get_write_lock(fs_lock)) {
        unlock(fs_lock);
        fclose(fs_lock);
        return -2;
    }

    if (!(fs_temp = fopen(OPEN_SMTP_TMP_FILE ".clear", "w"))) {
        rc = -3;
    } else if (!(fs_smtp = fopen(OPEN_SMTP_CUR_FILE, "r+"))) {
        fclose(fs_temp);
        rc = -4;
    } else {
        while (!rc && !feof(fs_smtp)) {
            unsigned stime;
            char sdata[256];

            switch (fscanf(fs_smtp, "%255s\t%u\n", sdata, &stime)) {
                case 2:
                    if ((clear + stime) >= now) {
                        fprintf(fs_temp, "%s\t%u\n", sdata, stime);
                    } else {
                        ++cc;
                    }
                    break;

                default:
                    rc = -5;
                case EOF:
                    break;
            }
        }
        fclose(fs_smtp);
        fclose(fs_temp);
    }

    if (!rc) {
        if (cc) {
            if (rename(OPEN_SMTP_TMP_FILE ".clear", OPEN_SMTP_CUR_FILE)) {
                rc = -6;
            } else if(update_rules()) {
                rc = -7;
            }
        } else {
            unlink(OPEN_SMTP_TMP_FILE ".clear");
        }
    }

    unlock(fs_lock);
    fclose(fs_lock);
    return rc;
}

