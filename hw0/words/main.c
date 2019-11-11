/*

  Word Count using dedicated lists

*/

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include "word_count.h"

/* Global data structure tracking the words encountered */

WordCount *word_counts = NULL;

size_t get_word(FILE *infile, char *buf, size_t buflen) {
  /*
    Stateless input parser - extract next word from a input stream, skipping initial non-alpha
    returns strlen(buf)
  */
  return 0;
}

void count_words(WordCount **wclist, FILE *infile) {
}

int main(int argc, char **argv) {

    return 0;
}
