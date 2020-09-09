#include <curses.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define N_WORDS 8
#define MAX_WORDS 255
#define WORD_LENGHT 16

#define KEY_ESC 27

// color stuff
#define RIGHT_WORD 1
#define WRONG_WORD 2
#define SELECT_WORD 3

// variables
uint8_t print_raw = 0;
int x_offset;
char *words[MAX_WORDS];

// functions
int init_curses();
bool typing_word_correctly(char *user, char *word_to_type);
void print_new_raw();
int read_words();
int shuffle();

int main()
{
	// populate the array with words from a file
	read_words();
	shuffle();
	/* for (int i = 0; i < MAX_WORDS; i++) */
	/* 	printf("%s\n", words[i]); */
	/* return 0; */

	// gui init
	init_curses();


	int cursor_x, cursor_y, n_ch = 0, index_word_to_type = 0;
	int correct_typed_words = 0;
	int ch;
	size_t word_lenght = 0;

	// set the x_offset
	for (int i = 0; i < N_WORDS; i++) {
		word_lenght += strlen(words[i]);
	}
	x_offset = (COLS/2) - (word_lenght + 7)/2;

	print_new_raw();

	char *user_word, *word_to_type;
	user_word = (char*)malloc(WORD_LENGHT*sizeof(char));
	word_to_type = (char*)malloc(WORD_LENGHT*sizeof(char));

	strcpy(word_to_type, words[index_word_to_type]);

	word_lenght = 0;

	attron(COLOR_PAIR(SELECT_WORD));
	mvprintw(5, x_offset, word_to_type);
	attroff(COLOR_PAIR(SELECT_WORD));

	move(7, (COLS/2) - 5);
	while((ch = getch()) != KEY_ESC)
	{
		getyx(stdscr, cursor_y, cursor_x);

		switch (ch)
		{
			case KEY_BACKSPACE:
				mvdelch(cursor_y, cursor_x - 1);
				user_word--;
				n_ch--;
				break;
			case ' ':
				// end the string
				*user_word = '\0';
				user_word -= n_ch;
				n_ch = 0;

				// update the word that must be typed
				index_word_to_type++;
				if (index_word_to_type == (N_WORDS * print_raw))
				{
					print_new_raw();
					word_lenght = 0;
				} else {
					if (typing_word_correctly(user_word, word_to_type))
					{
						correct_typed_words++;
						attron(COLOR_PAIR(RIGHT_WORD));

					} else 
						attron(COLOR_PAIR(WRONG_WORD));

					// mark as correct or wrong the current typed word
					mvprintw(5, x_offset + word_lenght, word_to_type);
					attroff(COLOR_PAIR(WRONG_WORD));
					attroff(COLOR_PAIR(RIGHT_WORD));

					word_lenght += strlen(word_to_type) + 1;
				}
				strcpy(word_to_type, words[index_word_to_type]);

				// select next word to be typed
				attron(COLOR_PAIR(SELECT_WORD));
				mvprintw(5, x_offset + word_lenght, word_to_type);

				// reset the cursor
				move(7, (COLS/2) - 5);
				clrtoeol();

				break;
			case '\n':
				break;
			default:
				*(user_word++) = ch;
				*user_word = '\0';
				n_ch++;

				if( typing_word_correctly(user_word - n_ch, word_to_type) )
					attron(COLOR_PAIR(RIGHT_WORD));
				else
					attron(COLOR_PAIR(WRONG_WORD));

				addch(ch);
				break;
		}
	}

	endwin(); 

	return 0;
}

bool typing_word_correctly(char *user, char *word_to_type){
	for( ; *user != '\0' ; user++, word_to_type++ ){
		if (*user != *word_to_type)
			return 0;
	}
	return 1;
}

int init_curses()
{
	// initial setup
	initscr();
	raw();
	keypad(stdscr, TRUE);
	noecho();

	if(has_colors() == FALSE)
	{	endwin();
		printf("Your terminal does not support color\n");
		exit(1);
	}

	start_color();
	init_pair(RIGHT_WORD, COLOR_GREEN, COLOR_BLACK);
	init_pair(WRONG_WORD, COLOR_RED, COLOR_BLACK);
	init_pair(SELECT_WORD, COLOR_BLACK, COLOR_WHITE);

	mvprintw(8, (COLS/2)-7, "-----------------");
	// ──────────────
	return 1;
}

void print_new_raw()
{
	size_t word_lenght = 0;
	int i;

	attroff(COLOR_PAIR(WRONG_WORD));
	attroff(COLOR_PAIR(RIGHT_WORD));

	move(5,0);
	clrtoeol();
	for (i = N_WORDS * print_raw; i < N_WORDS + N_WORDS * print_raw; i++) {
		mvprintw(5, x_offset + word_lenght, words[i]);
		word_lenght += strlen(words[i]) + 1;
	}

	word_lenght = 0;
	print_raw++;

	move(6,0);
	clrtoeol();

	for (i = N_WORDS * print_raw; i < N_WORDS * (1 + print_raw); i++) {
		mvprintw(6, x_offset + word_lenght, words[i]);
		word_lenght += strlen(words[i]) + 1;
	}
}

int read_words()
{
	FILE *words_file;
	words_file = fopen("words.txt", "r");

	int i = 0, j;
	char *buffer;
	buffer = (char*)malloc(WORD_LENGHT*sizeof(char));

	srand((unsigned) time(NULL));

	while ((fscanf(words_file, "%s", buffer) != EOF) && i < MAX_WORDS )
	{
		if (rand() % 4 == 1)
		{
			words[i] = (char*)malloc(WORD_LENGHT*sizeof(char));
			strcpy(words[i], buffer);
			i++;
		}
	}
	
	free(buffer);
	fclose(words_file);

	return 1;
}

int shuffle()
{
	int indx1, indx2;
	char *dummy;
	srand((unsigned) time(NULL));

	for (int i = 0; i < 500; i++){ 
		indx1 = rand() % 255;
		dummy = words[indx1];
		indx2 = rand() % 255;
		words[indx1] = words[indx2];
		words[indx2] = dummy;
	}
}
