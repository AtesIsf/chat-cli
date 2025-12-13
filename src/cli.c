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
    // Also, inputting "`" causes a bug, fix it.
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
 * Displays the configurations. Throws an assertion
 * if the parameter is NULL. Can also edit configs if requested.
 */

void display_settings(configs_t *conf) {
  assert(conf != NULL);

  bool exit_screen = false;
  while (!exit_screen) {
    int choice = -1;
    clear_screen();
    puts("~~~~~~~~~~~~~~~~~~~~~~~~");
    puts(">> Settings");
    puts("~~~~~~~~~~~~~~~~~~~~~~~~");
    puts(">> Select 1 to exit");
    puts(">> Select 2 to edit");
    puts("~~~~~~~~~~~~~~~~~~~~~~~~");
    printf("* Client Port: %lu\n", conf->client_port);
    printf("* Server Port: %lu\n", conf->server_port);

    int status_code = scanf("%d", &choice);
    if (status_code != 1) {
      char temp = '\n';
      do {
        temp = getchar();
      } while (temp != '\n' && temp != EOF);
    }
    exit_screen = choice == 1 ? true : false;
    // TODO: editing logic
    if (choice ==  2) {

    }
  }
  clear_screen();
}

/*
 * The loop that serves as the interface for the user. Asserts
 * that the parameter is not NULL.
 */

void cli_loop(sqlite3 *db, configs_t *conf) {
  assert(db != NULL);

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
    header_print();
    size_t i = 0;
    for (i = 0; i < n_chats; i++) {
      if (chat_names[i] == NULL) {
        continue;
      }
      printf("%lu) %s\n", i + 1, chat_names[i]);
    }
    printf("- Select a chat to go to by typing a number below (1-%lu):\n", i);
    printf("- Select %lu to exit Chat-CLI.\n", i + 1);
    printf("- Select %lu to view settings.\n", i + 2);

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
    } else if (choice == i + 2) {
      display_settings(conf);
    } else if (choice > i + 2) {
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

