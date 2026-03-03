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
      printf("[%s] %s %s\n", messages[i].timestamp, 
             messages[i].is_sent ? ">>" : "<<", 
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
      ip_addr_t lookup_addr = (ip_addr_t) { .family = AF_INET, .addr.v4.s_addr = htonl(LOOKUP_ADDR)};
      ip_addr_t peer_addr = fetch_user_ip(chat_name, lookup_addr, ctx, &success);
      
      if (success) {
        if (send_message(my_username, input_buf, peer_addr, ctx, NULL) == 0) {
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

  while (!global_terminate_program) {
    int n_chats = 0;
    const char **chat_names = get_chats(db, &n_chats);

    int *ids = NULL;
    if (n_chats > 0) {
      ids = malloc(sizeof(int) * n_chats);
      assert(ids != NULL);
      for (int i = 0; i < n_chats; i++) {
        ids[i] = get_id_of_username(db, chat_names[i]);
      }
    }

    int choice = -1;
    header_print(username);
    int i = 0;
    for (i = 0; i < n_chats; i++) {
      if (chat_names[i] == NULL) {
        continue;
      }
      printf("%d) %s\n", i + 1, chat_names[i]);
    }
    puts("~~~~~~~~~~~~~~~~~~~~~~~~");
    printf("- Select %d to start a new chat.\n", i + 1);
    printf("- Select %d to exit Chat-CLI.\n", i + 2);
    printf("- Enter 0 to refresh the chat list.\n");

    int return_val = scanf("%d", &choice);
    if (return_val != 1) {
      // Clear the buffer if user entered something else
      int c;
      while ((c = getchar()) != '\n' && c != EOF);
      clear_screen();
      choice = -1; // Reset choice to stay in loop
    } else {
      // Clear the buffer after scanf to avoid issues with fgets later
      int c;
      while ((c = getchar()) != '\n' && c != EOF);
    }

    // This equals the exit choice's number due to the loop
    if (choice == i + 2) {
      global_terminate_program = true;
    } else if (choice == i + 1) {
      clear_screen();
      start_new_chat(db, username, ctx);
    } else if (choice == 0) {
      clear_screen();
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
    if (chat_names != NULL) {
      for (int j = 0; j < n_chats; j++) {
        free((void *) chat_names[j]);
      }
      free(chat_names);
    }
    if (ids != NULL) {
      free(ids);
    }
  }
  puts("\n[Info] Shutting Down...");
}

void start_new_chat(sqlite3 *db, const char *my_username, SSL_CTX *ctx) {
  assert(db != NULL && my_username != NULL && ctx != NULL);

  printf(">> Enter username of the person you want to chat with:\n>> ");
  char target_username[32] = { '\0' };
  if (fgets(target_username, 31, stdin) == NULL) {
    return;
  }
  target_username[strcspn(target_username, "\n")] = 0;

  if (strlen(target_username) == 0) {
    return;
  }

  bool success = false;
  ip_addr_t lookup_addr = (ip_addr_t) { .family = AF_INET, .addr.v4.s_addr = htonl(LOOKUP_ADDR)};
  ip_addr_t peer_addr = fetch_user_ip(target_username, lookup_addr, ctx, &success);

  if (!success) {
    printf("[ERROR] User '%s' not found on the lookup server.\n", target_username);
    getchar();
    return;
  }

  printf(">> Enter your first message to %s:\n>> ", target_username);
  char message[256] = { '\0' };
  if (fgets(message, 255, stdin) == NULL) {
    return;
  }
  message[strcspn(message, "\n")] = 0;

  unsigned char real_fingerprint[32] = { 0 }; 
  if (send_message(my_username, message, peer_addr, ctx, real_fingerprint) == 0) {
    // Message sent, now add chat to DB with REAL fingerprint.
    int chat_id = add_chat(db, target_username, real_fingerprint);
    if (chat_id != -1) {
      insert_message(db, chat_id, true, message);
      printf("[INFO] Chat started with %s.\n", target_username);
    } else {
      puts("[ERROR] Failed to add chat to database.");
    }
  } else {
    puts("[ERROR] Failed to send message to peer.");
  }
  getchar();
}

