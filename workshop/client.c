#include <stdint.h>
#include <stdbool.h>
#include <sel4cp.h>
#include "wordle.h"

#define MOVE_CURSOR_UP "\033[5A"
#define CLEAR_TERMINAL_BELOW_CURSOR "\033[0J"

#define INVALID_CHAR (-1)

/* Start of my changes. */
#define CLIENT_TO_SERIAL_SERVER_CHANNEL_ID (2)
#define CLIENT_TO_WORDLE_SERVER_CHANNEL_ID (3)
/* This is the buffer we read from and `serial_server` writes to. */
uintptr_t serial_server_buf;
/* This is the buffer we write to and `serial_server` reads from. */
uintptr_t client_buf;
/* This is the buffer we read from and `wordle_server_buf` writes to. */
uintptr_t wordle_server_buf;
/* ASCII code for the ENTER key. */
#define ENTER_KEY (10)
#define CARRIAGE_RETURN_KEY (13)
#define BACKSPACE_KEY (8)
#define DELETE_KEY (127)
/* End of my changes. */

struct wordle_char {
    int ch;
    enum character_state state;
};

// Store game state
static struct wordle_char table[NUM_TRIES][WORD_LENGTH];
// Use these global variables to keep track of the character index that the
// player is currently trying to input.
static int curr_row = 0;
static int curr_letter = 0;

// Call this function to update the game state once we receive
// a message from the Wordle server.
void update_state() {
    for (int i = 0; i < WORD_LENGTH; i++) {
        table[curr_row - 1][i].state = sel4cp_mr_get(i);
    }
}

// Implement this function to send the word over PPC.
sel4cp_msginfo wordle_server_send(char *word) {
    /* Create a `msg` the length of the word. */
    sel4cp_msginfo msg = sel4cp_msginfo_new(0, WORD_LENGTH);
    /* Note, we don't bother sending '\0' in `word` in the following for-loop. */
    for (int i = 0; i < WORD_LENGTH; i++) {
        /* Set each message register to the corresponding character in `word`. */
        sel4cp_mr_set(i, word[i]);
    }
    /* Make our protected procedure call with our `msg`. */
    return sel4cp_ppcall(CLIENT_TO_WORDLE_SERVER_CHANNEL_ID, msg);
    // return sel4cp_msginfo_new(0, WORD_LENGTH);
}

// Implement this function to get the serial server to print the string.
void serial_send(char *str) {

    int curr_idx = 0;
    /* We iterate through the string until we hit the NULL terminator,
    which we don't bother sending to the serial_server. */
    while (str[curr_idx] != '\0') {
        /* Save the character in `client_buf`. */
        *((char *) client_buf) = str[curr_idx];
        /* Notify the `serial_server` like so. Since, we have a lower
        priority than the `serial_server`, we will be pre-empted after
        the call to `sel4cp_notify()` until the `serial_server` has finished 
        printing the character to the screen. */
        sel4cp_notify(CLIENT_TO_SERIAL_SERVER_CHANNEL_ID);
        /* Increment the index to send the next character to `serial_server`. */
        curr_idx++;
    }
}

// This function prints a CLI Wordle using pretty colours for what characters
// are correct, or correct but in the wrong place etc.
void print_table(bool clear_terminal) {
    if (clear_terminal) {
        // Assuming we have already printed a Wordle table, this will clear the
        // table we have already printed and then print the updated one. This
        // is done by moving the cursor up 5 lines and then clearing everything
        // below it.
        serial_send(MOVE_CURSOR_UP);
        serial_send(CLEAR_TERMINAL_BELOW_CURSOR);
    }

    for (int row = 0; row < NUM_TRIES; row++) {
        for (int letter = 0; letter < WORD_LENGTH; letter++) {
            serial_send("[");
            enum character_state state = table[row][letter].state;
            int ch = table[row][letter].ch;
            if (ch != INVALID_CHAR) {
                switch (state) {
                    case INCORRECT: break;
                    case CORRECT_PLACEMENT: serial_send("\x1B[32m"); break;
                    case INCORRECT_PLACEMENT: serial_send("\x1B[33m"); break;
                    default:
                        // Print out error messages/debug info via debug output
                        sel4cp_dbg_puts("CLIENT|ERROR: unexpected character state\n");
                }
                char ch_str[] = { ch, '\0' };
                serial_send(ch_str);
                // Reset colour
                serial_send("\x1B[0m");
            } else {
                serial_send(" ");
            }
            serial_send("] ");
        }
        serial_send("\n");
    }
}

void init_table() {
    for (int row = 0; row < NUM_TRIES; row++) {
        for (int letter = 0; letter < WORD_LENGTH; letter++) {
            table[row][letter].ch = INVALID_CHAR;
        }
    }
}

bool char_is_backspace(int ch) {
    return (ch == 0x7f);
}

bool char_is_valid(int ch) {
    // Only allow alphabetical letters and do not accept a character if the current word
    // has already been filled.
    // return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') && curr_letter != WORD_LENGTH;
    return ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) && curr_letter != WORD_LENGTH;
}

void init(void) {
    sel4cp_dbg_puts("CLIENT: starting\n");
    serial_send("Welcome to the Wordle client!\n");

    init_table();
    // Don't want to clear the terminal yet since this is the first time
    // we are printing it (we want to clear just the Wordle table, not
    // everything on the terminal).
    print_table(false);
}

void notified(sel4cp_channel channel) {
    switch (channel) {
        /* This case will execute when the `serial_server` notifies us
        that the user has pressed a character. */
        case CLIENT_TO_SERIAL_SERVER_CHANNEL_ID: {
            /* This will run if the `serial_server` receives a character. */
            /* Get the character from `serial_server_buf`. */
            char ch = *((char *) serial_server_buf);
            if (char_is_valid(ch)) {
                /* If the character is valid, we simply add it to the table. */
                /* Create a new `wordle_char` struct containing out character `ch`. */
                struct wordle_char valid_ch = {0};
                valid_ch.ch = ch;
                /* If the character is valid, we should add the character to the correct
                position in our 2D `table` array. */
                table[curr_row][curr_letter] = valid_ch;
                /* We should also advance the `curr_letter` for the next character. */
                curr_letter++;
                /* We re-print the Wordle table by setting the `clear_terminal` param to `true`. */
                print_table(true);
            } else if ((ch == CARRIAGE_RETURN_KEY || ch == ENTER_KEY) && curr_letter == WORD_LENGTH) {
                /* Part 3 code: */

                /* If the user entered the `ENTER` key then we must check if the user's attempt is
                correct by making a Protected Procedure Call (PPC) via the `wordle_server_send()`
                function. */
                /* Create an empty string to store the user's attempt. */
                char attempt[WORD_LENGTH + 1] = {0};
                /* We only iterate `WORLD_LENGTH` times since the last char in `attempt`
                will always be '\0' */
                for (int i = 0; i < WORD_LENGTH; i++) {
                    /* Copy the word character by character from the relevant row in 
                    the table to `attempt`. */
                    attempt[i] = table[curr_row][i].ch;
                }
                /* Pass the user's `attempt` to `wordle_server_send()`, which expects 
                a char *. In production code, you probably should be checking the return
                value by parsing the message returned by `wordle_server_send()`. */
                wordle_server_send(attempt);

                /* Part 2 code: */

                /* If the user entered the `ENTER` key and they have already
                entered the full length of the word, then we should take them to a new line of
                the table by incrementing `curr_row`. */
                curr_row++;
                /* Since we are on a fresh row, we should reset `curr_letter` to 0. */
                curr_letter = 0;
                /* Note, we do not need to call `print_table(true)` since there is nothing
                to display to the user. */
            } else if ((ch == BACKSPACE_KEY || ch == DELETE_KEY) && curr_letter > 0) {
                /* Create an "empty" Wordle character that contains a ' ' character. */
                struct wordle_char empty_ch = {0};
                empty_ch.ch = ' ';
                /* Set the last character in the current row to our empty character. */
                table[curr_row][curr_letter - 1] = empty_ch;
                /* Decrement the current letter since we've deleted a letter. */
                curr_letter--;
                /* Re-print the Wordle table by setting the `clear_terminal` param to `true`. */
                print_table(true);
            }
            break;
        }
        /* This case will execute when the `wordle_server` notifies us that
        the `client` has finished checking an attempt by the user. This happens
        after we send the `wordle_server` a PPC via `wordle_server_send()`, after
        they have pressed the `ENTER` key. */
        case CLIENT_TO_WORDLE_SERVER_CHANNEL_ID: {
            for (int i = 0; i < WORD_LENGTH; i++) {
                /* Since this is called AFTER the user has pressed the `ENTER` key,
                the word we want to check is is in `curr_row - 1` instead of `curr_row`. */
                table[curr_row - 1][i].state = ((enum character_state *) wordle_server_buf)[i];
            }
            /* Since we've altered the state of a row in our `table`, we need to 
            re-print the entire table again. */
            print_table(true);
            break;
        }
        default: {
            break;
        }
    }
}



