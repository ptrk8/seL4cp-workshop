#include <stdint.h>
#include <stdbool.h>
#include <sel4cp.h>
#include "wordle.h"

#define MOVE_CURSOR_UP "\033[5A"
#define CLEAR_TERMINAL_BELOW_CURSOR "\033[0J"

#define INVALID_CHAR (-1)

/* Start of my changes. */
#define CLIENT_CHANNEL_ID (2)
uintptr_t serial_server_buf;
uintptr_t client_buf;
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

sel4cp_msginfo wordle_server_send(char *word) {
    // Implement this function to send the word over PPC
    return sel4cp_msginfo_new(0, WORD_LENGTH);
}

// Implement this function to get the serial server to print the string.
void serial_send(char *str) {

    int curr_idx = 0;
    /* We iterate through the string until we hit the NULL terminator,
    which we don't bother sending to the serial_server. */
    while (str[curr_idx] != '\0') {
        /* Save the character in the client buf. */
        *((char *) client_buf) = str[curr_idx];
        /* Notify the `serial_server` like so. Since, we have a lower
        priority than the `serial_server`, we will be pre-empted after
        the call to `sel4cp_notify()` until the `serial_server` has finished 
        printing the character to the screen. */
        sel4cp_notify(CLIENT_CHANNEL_ID);
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

void notified(sel4cp_channel channel) {}
