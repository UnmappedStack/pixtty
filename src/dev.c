/* Handles the vtty device which child programs write to. 
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under the Mozilla Public License 2.0.
 * See the LICENSE file in the Git repository's root for mroe information. */

#include <pixtty.h>
#include <gui.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <pty.h>

static int open_pty_master() {
    int master_fd;
    if ((master_fd=posix_openpt(O_RDWR | O_NOCTTY)) < 0) {
        fprintf(stderr, "posix_openpt returned error");
        return -1;
    }
    return master_fd;
}

static int grant_and_unlock_slave(int master_fd) {
    if (grantpt(master_fd) || unlockpt(master_fd)) {
        fprintf(stderr, "grantpt/unlockpt returned error");
        return -1;
    }
    return 0;
}

static char *get_slave_name(int master_fd) {
    char *slave_name = ptsname(master_fd);
    if (!slave_name) {
        fprintf(stderr, "ptsname returned error");
        close(master_fd);
        return NULL;
    }
    return slave_name;
}

static void handle_tty_write(char *buf, Win win) {
    printf("the VTTY got data written to it:\n");
    int len = strlen(buf);
    fwrite(buf, 1, (len < 256) ? len : 256, stdout);
    write_str(win, buf);
}

int wait_for_vtty_write_iteration(int master_fd, Win win) {
    char buf[256];
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(master_fd, &rfds);
    int ret = select(master_fd + 1, &rfds, NULL, NULL, NULL);
    if (ret < 0) {
        fprintf(stderr, "select returned error");
        return -1;
    }
    if (!FD_ISSET(master_fd, &rfds)) return 0;
    ssize_t n = read(master_fd, buf, sizeof(buf) - 1);
    if (!n) return 0;
    if (n < 0) {
        fprintf(stderr, "read returned error");
        return -1;
    }
    handle_tty_write(buf, win);
    return 0;
}

int set_fds_and_run_shell(int new_stdout) {
    // save old stderr/stdout
    int old_stdout = dup(1);
    int old_stderr = dup(2);
    if (old_stdout < 0 || old_stderr < 0) {
        fprintf(stderr, "dup() returned error");
        return -1;
    }
    // copy the new stderr/stdout to where we want them
    if (dup2(new_stdout, 1) < 0 || dup2(new_stdout, 2) < 0) {
        // just exit() here because it can't safely report the error
        // as the file descriptors are unknown
        exit(-1);
    }
    exec_shell();
    // go back to the old stderr/stdout for the terminal itself
    if (dup2(old_stdout, 1) < 0 || dup2(old_stderr, 2) < 0) {
        // just exit() here because it can't safely report the error
        // as the file descriptors are unknown
        exit(-1);
    }
    return 0;
}

int init_dev(void) {
    int master_fd;
    char *slave_name;
    if ((master_fd=open_pty_master()) < 0) return -1;
    if (grant_and_unlock_slave(master_fd) < 0) return -1;
    if (!(slave_name=get_slave_name(master_fd))) return -1;
    printf("slave pty dev: %s\n", slave_name);
    // caller is expected to clean up
    return master_fd;
}
