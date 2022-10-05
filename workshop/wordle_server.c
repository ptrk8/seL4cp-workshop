#include <stdint.h>
#include <stdbool.h>
#include <sel4cp.h>
#include "dictionary.h"
#include "wordle.h"

#define CLIENT_CHANNEL 1

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
