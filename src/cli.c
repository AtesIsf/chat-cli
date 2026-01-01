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

/*
 * Prints out the header for the CLI. Asserts that
 * the given parameter is not NULL.
 */

void header_print(const char *username) {
  assert(username != NULL);

  puts("~~~~~~~~~~~~~~~~~~~~~~~~");
  puts("\tChat-CLI");
  puts("~~~~~~~~~~~~~~~~~~~~~~~~");
  printf("Logged in as: %s\n", username);
  puts("~~~~~~~~~~~~~~~~~~~~~~~~");
  puts("- Chats:");
}

/*
 * Displays the selected messages from a given chat, also has settings to send
 * a new message. Asserts the given database is not NULL and the given id is
 * nonnegative.
 */

void display_chat_interface(sqlite3 *db, int id, const char *chat_name) {
  assert(db != NULL && id >= 0);
  int n_msgs = 0;

  // TODO: You may not want to load all messages at the same time
  msg_t *messages = get_messages_from_chat_id(db, id, &n_msgs);

  bool exit_screen = false;
  while (!exit_screen) {
    int choice = -1;
    clear_screen();
    puts("~~~~~~~~~~~~~~~~~~~~~~~~");
    printf(">> Chat with: %s\n", chat_name);
    puts("~~~~~~~~~~~~~~~~~~~~~~~~");
    puts(">> Select 1 to exit");
    puts("~~~~~~~~~~~~~~~~~~~~~~~~");

    for (size_t i = 0; i < n_msgs; i++) {
      puts(messages[i].content);
    }
    
    // TODO: Implement this after finishing message displaying
    int status_code = scanf("%d", &choice);
    if (status_code != 1) {
      char temp = '\n';
      do {
        temp = getchar();
      } while (temp != '\n' && temp != EOF);
    }
    exit_screen = choice == 1 ? true : false;
  }

  free(messages);
  messages = NULL;
  clear_screen();
}

/*
 * The loop that serves as the interface for the user. Asserts
 * that the parameters are not NULL.
 */

void cli_loop(sqlite3 *db, const char *username) {
  assert(db != NULL && username != NULL);

  signal(SIGINT, terminate);
  int n_chats = 0;
  const char **chat_names = get_chats(db, &n_chats);

  int *ids = malloc(sizeof(int) * n_chats);
  assert(ids != NULL);
  for (int i = 0; i < n_chats; i++) {
    ids[i] = get_id_of_username(db, chat_names[i]);
  }

  while (!terminate_program) {
    int choice = -1;
    header_print(username);
    size_t i = 0;
    for (i = 0; i < n_chats; i++) {
      if (chat_names[i] == NULL) {
        continue;
      }
      printf("%lu) %s\n", i + 1, chat_names[i]);
    }
    puts("~~~~~~~~~~~~~~~~~~~~~~~~");
    printf("- Select a chat to go to by typing a number below (1-%lu):\n", i);
    printf("- Select %lu to exit Chat-CLI.\n", i + 1);

    int status_code = scanf("%d", &choice);
    if (status_code != 1) {
      char temp = '\n';
      do {
        temp = getchar();
      } while (temp != '\n' && temp != EOF);
    }

    // This equals the exit choice's number due to the loop
    if (choice == i + 1) {
      terminate_program = true;
    } else if (choice > i + 1) {
      // Invalid input, don't try to display a non-existant chat
      clear_screen();
    } else {
      clear_screen();
      const char *selected_username = chat_names[choice - 1];
      int id = ids[choice - 1];
      if (id >= 0) {
        display_chat_interface(db, id, selected_username);
      }
    }
  }

  for (size_t i = 0; i < n_chats; i++) {
    free((void *) chat_names[i]);
    chat_names[i] = NULL;
  }
  free(chat_names);
  chat_names = NULL;
  free(ids);
  ids = NULL;

  puts("\n[Info] Shutting Down...");
}

