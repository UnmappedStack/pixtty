#pragma once

#include <gui.h>

int init_dev(void);
int wait_for_vtty_write_iteration(int master_fd, Win win);
int set_fds_and_run_shell(int new_stdout);
