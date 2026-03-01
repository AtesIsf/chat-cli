#include "cli.h"
#include "database.h"
#include "server.h"

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void terminate(int n) {
  global_terminate_program = true;
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

void display_chat_interface(sqlite3 *db, int id, const char *chat_name, const char *my_username, SSL_CTX *ctx) {
  assert(db != NULL && id >= 0 && my_username != NULL && ctx != NULL);
  int n_msgs = 0;

  msg_t *messages = get_messages_from_chat_id(db, id, &n_msgs);

  bool exit_screen = false;
  while (!exit_screen) {
    clear_screen();
    puts("~~~~~~~~~~~~~~~~~~~~~~~~");
    printf(">> Chat with: %s\n", chat_name);
    puts("~~~~~~~~~~~~~~~~~~~~~~~~");
    
    for (size_t i = 0; i < n_msgs; i++) {
      printf("[%s] %s: %s\n", messages[i].timestamp, 
             messages[i].is_sent ? my_username : chat_name, 
             messages[i].content);
    }
    
    puts("~~~~~~~~~~~~~~~~~~~~~~~~");
    puts(">> Enter message to send, or ':q' to exit:");
    printf(">> ");
    
    char input_buf[256] = { '\0' };
    if (fgets(input_buf, 255, stdin) == NULL) {
      break;
    }
    
    // Remove newline
    input_buf[strcspn(input_buf, "\n")] = 0;

    if (strcmp(input_buf, ":q") == 0) {
      exit_screen = true;
    } else if (strlen(input_buf) > 0) {
      // Send logic
      bool success = false;
      // TODO: Make the lookup server IP configurable!
      ip_addr_t lookup_addr = (ip_addr_t) { .family = AF_INET, .addr.v4.s_addr = htonl(INADDR_LOOPBACK)};
      ip_addr_t peer_addr = fetch_user_ip(chat_name, lookup_addr, ctx, &success);
      
      if (success) {
        if (send_message(my_username, input_buf, peer_addr, ctx) == 0) {
          insert_message(db, id, true, input_buf);
          // Refresh messages
          for(int i = 0; i < n_msgs; i++) {
            free(messages[i].content);
            free(messages[i].timestamp);
          }
          free(messages);
          messages = get_messages_from_chat_id(db, id, &n_msgs);
        } else {
          puts("[ERROR] Failed to send message to peer.");
          getchar(); // Wait for user
        }
      } else {
        puts("[ERROR] Could not find peer IP.");
        getchar();
      }
    }
  }

  for(int i = 0; i < n_msgs; i++) {
    free(messages[i].content);
    free(messages[i].timestamp);
  }
  free(messages);
  messages = NULL;
  clear_screen();
}

/*
 * The loop that serves as the interface for the user. Asserts
 * that the parameters are not NULL.
 */

void cli_loop(sqlite3 *db, const char *username, SSL_CTX *ctx) {
  assert(db != NULL && username != NULL && ctx != NULL);

  signal(SIGINT, terminate);
  int n_chats = 0;
  const char **chat_names = get_chats(db, &n_chats);

  int *ids = malloc(sizeof(int) * n_chats);
  assert(ids != NULL);
  for (int i = 0; i < n_chats; i++) {
    ids[i] = get_id_of_username(db, chat_names[i]);
  }

  while (!global_terminate_program) {
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

    scanf("%d", &choice);
    // Clear the buffer after scanf to avoid issues with fgets later
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    // This equals the exit choice's number due to the loop
    if (choice == i + 1) {
      global_terminate_program = true;
    } else if (choice > 0 && choice <= i) {
      clear_screen();
      const char *selected_username = chat_names[choice - 1];
      int id = ids[choice - 1];
      if (id >= 0) {
        display_chat_interface(db, id, selected_username, username, ctx);
      }
    } else {
      clear_screen();
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

