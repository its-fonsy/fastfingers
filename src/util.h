#ifndef UTIL_H_
#define UTIL_H_

#define MAX_WORDS 255
#define MAX_WORD_LENGHT 16

int typing_word_correctly(char *user, char *word_to_type);
int word_typed_right(char *user, char *word_to_type);
int string_len(char *s);

int shuffle(char *array[], int len);
int feed_words_into_array(char* filename, char* array[]);
#endif
