/* Entry file which sets everything up and calls the rest of the program.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under the Mozilla Public License 2.0.
 * See the LICENSE file in the Git repository's root for mroe information. */

#include <pixtty.h>
#include <dev.h>
#include <gui.h>

#include <stdio.h>
#include <unistd.h>

void exec_shell(void) {
    pid_t pid = fork();
    if (!pid)
        execve(SHELL, (char*[]){SHELL, "-a", NULL}, __environ);
}

int main(void) {
    printf("hello i am pixtty terminal\n");
    int stdio_fd;
    if ((stdio_fd=init_dev()) < 0) return -1;
    if (set_fds_and_run_shell(stdio_fd) < 0) return -1;
    if (init_gui_window(stdio_fd) < 0) {
        close(stdio_fd);
        return -1;
    }
    close(stdio_fd);
    return 0;
}
