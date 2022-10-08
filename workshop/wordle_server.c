#include <stdint.h>
#include <stdbool.h>
#include <sel4cp.h>
#include "dictionary.h"
#include "wordle.h"

/* Start of my changes. */
#define WORDLE_SERVER_TO_CLIENT_CHANNEL_ID (4)
/* This is the buffer we write to and `client` reads from. */
uintptr_t wordle_server_buf;
/* End of my changes. */

const char *word = "hello";

bool is_character_in_word(const char *word, int ch) {
    for (int i = 0; i < WORD_LENGTH; i++) {
        if (word[i] == ch) {
            return true;
        }
    }

    return false;
}

enum character_state char_to_state(int ch, const char *word, uint64_t index) {
    if (ch == word[index]) {
        return CORRECT_PLACEMENT;
    } else if (is_character_in_word(word, ch)) {
        return INCORRECT_PLACEMENT;
    } else {
        return INCORRECT;
    }
}

void
init(void) {
    sel4cp_dbg_puts("WORDLE SERVER: starting\n");
}

void
notified(sel4cp_channel channel) {}

/* Protected Procedure Entry point. */
sel4cp_msginfo protected(sel4cp_channel ch, sel4cp_msginfo msginfo) {
    /* Code for Part 3: */
    switch (ch) {
        /* This case will execute when the `client` sends us a Protected
        Procedure Call (PPC) */
        case WORDLE_SERVER_TO_CLIENT_CHANNEL_ID: {
            /* Create an empty string to receive the user's attempt. */
            char attempt[WORD_LENGTH + 1] = {0};
            /* We only iterate `WORLD_LENGTH` times since the last char in `attempt`
            will always be '\0' */
            for (int i = 0; i < WORD_LENGTH; i++) {
                /* Obtain each character from the corresponding message registers. */
                attempt[i] = sel4cp_mr_get(i);
                /* Set each element of `wordle_server_buf` to the relevant
                `enum character_state` value. The `client` will check `wordle_server_buf`
                to discover the result of this attempt. */
                ((enum character_state *) wordle_server_buf)[i] = char_to_state(attempt[i], word, i);
            }
            // sel4cp_dbg_puts(attempt); /* For debugging. */
            /* Notify `client` that we're done checking the word. The `client` can
            now iterate through `wordle_server_buf` on their side to discover the
            outcome of their attempt. */
            sel4cp_notify(WORDLE_SERVER_TO_CLIENT_CHANNEL_ID);
            /* Return an empty message. In production code, you probably should be returning
            a message containing some form of error value for the `client` to check. */
            return sel4cp_msginfo_new(0, 0);
        }
        default: {
            break;
        }
    }
    return sel4cp_msginfo_new(0, WORD_LENGTH);
}