#include "cli.h"
#include "database.h"

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void terminate(int n) {
  terminate_program = true;
}

void clear_screen() {
  printf("\033[2J\033[H");
  fflush(stdout);
}

void header_print() {
  puts("~~~~~~~~~~~~~~~~~~~~~~~~");
  puts("\tChat-CLI");
  puts("~~~~~~~~~~~~~~~~~~~~~~~~");
  puts("- Chats:");
}

/*
 * The loop that serves as the interface for the user. Asserts
 * that the parameter is not NULL.
 */

void cli_loop(sqlite3 *db) {
  assert(db != NULL);

  signal(SIGINT, terminate);
  int n_chats = 0;
  const char **chat_names = get_chats(db, &n_chats);

  int choice = -1;
  while (!terminate_program) {
    header_print();
    size_t i = 0;
    for (i = 0; i < n_chats; i++) {
      if (chat_names[i] == NULL) {
        continue;
      }
      printf("%lu) %s\n", i + 1, chat_names[i]);
    }
    printf("- Select a chat to go to by typing a number below (1-%lu):\n", i);
    printf("- Or, select %lu to exit Chat-CLI.\n", i + 1);
    scanf("%d", &choice);

    clear_screen();

    // This equals the exit choice's number due to the loop
    if (choice == i + 1) {
      terminate_program = true;
    }
  }

  for (size_t i = 0; i < n_chats; i++) {
    free((void *) chat_names[i]);
    chat_names[i] = NULL;
  }
  free(chat_names);
  chat_names = NULL;

  puts("\n[Info] Shutting Down...");
}

