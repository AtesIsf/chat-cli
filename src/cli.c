#include "cli.h"

#include <signal.h>
#include <stdio.h>

void terminate(int n) {
  terminate_program = true;
}

void clear_screen() {
  printf("\033[2J\033[H");
  fflush(stdout);
}

void cli_loop() {
  signal(SIGINT, terminate);

  while (!terminate_program) {
    // TODO
  }
}

