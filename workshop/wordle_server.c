#include <stdint.h>
#include <stdbool.h>
#include <sel4cp.h>
#include "dictionary.h"
#include "wordle.h"

#define WORDLE_SERVER_TO_CLIENT_CHANNEL_ID (4)

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
    switch (ch) {
        case WORDLE_SERVER_TO_CLIENT_CHANNEL_ID: {
            /* Create an empty string to receive the user's attempt. */
            char attempt[WORD_LENGTH + 1] = {0};
            /* We only iterate `WORLD_LENGTH` times since the last char in `attempt`
            will always be '\0' */
            for (int i = 0; i < WORD_LENGTH; i++) {
                /* Obtain each character from the corresponding message registers. */
                attempt[i] = sel4cp_mr_get(i);
            }
            sel4cp_dbg_puts(attempt);
            return sel4cp_msginfo_new(0, WORD_LENGTH);
        }
        default: {
            break;
        }
    }
    return sel4cp_msginfo_new(0, WORD_LENGTH);
}