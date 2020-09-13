#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "util.h"

int typing_word_correctly(char *user, char *word_to_type)
{
	for( ; *user != '\0' ; user++, word_to_type++ )
		if (*user != *word_to_type)
			return 0;
	return 1;
}

int word_typed_right(char *user, char *word_to_type)
{
	for( ; *user == *word_to_type ; user++, word_to_type++ )
		if (*user == '\0')
			return 1;
	return 0;
}

int string_len(char *s)
{
	int i = 0;
	for(; *s != '\0'; s++, i++)
		if(*s == -61)
			i--;
	return i;
}

int feed_words_into_array(char* filename, char* array[])
{
	FILE *words_file;
	words_file = fopen(filename, "r");

	int i = 0;
	char *buffer = (char*)malloc(MAX_WORD_LENGHT*sizeof(char));

	srand((unsigned) time(NULL));

	// 1/3 of probabilty that the word will be added to the array
	while ((fscanf(words_file, "%s", buffer) != EOF) && i < MAX_WORDS )
		if (rand() % 4 == 1)
			strcpy(array[i++], buffer);
	
	free(buffer);
	fclose(words_file);

	// shuffle the elemets in the array
	shuffle(array, MAX_WORDS);

	return 1;
}

#define SWAP(x, y) do { typeof(x) temp = x; x = y; y = temp; } while (0)
int shuffle(char *array[], int len)
{
	srand((unsigned) time(NULL));

	for (int i = 0; i < 500; i++) 
		SWAP(array[rand() % (len -1)], array[rand() % (len -1)]);

	return 1;
}
