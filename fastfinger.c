#include <curses.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define N_WORD_PER_ROW 7
#define MAX_WORDS 255
#define WORD_LENGHT 16

#define KEY_ESC 27

// color stuff
#define RIGHT_WORD 1
#define WRONG_WORD 2
#define SELECT_WORD 3

// variables
int x_offset, y_offset;
char *words[MAX_WORDS];

// functions
int init_curses();
int typing_word_correctly(char *user, char *word_to_type);
int word_typed_right(char *user, char *word_to_type);
int feed_words_into_array();
int shuffle();
int typing_round();
void print_words_to_type();

int main()
{
	// gui init
	init_curses();

	// populate the array with words from a file
	feed_words_into_array();

	// type some words!
	typing_round();

	endwin(); 

	return 0;
}

int init_curses()
{
	// initial curses setup
	initscr();
	raw();
	keypad(stdscr, TRUE);
	noecho();
	/* timeout(1); */

	if(has_colors() == FALSE)
	{	endwin();
		printf("Your terminal does not support color\n");
		exit(1);
	}

	start_color();
	init_pair(RIGHT_WORD, COLOR_GREEN, COLOR_BLACK);
	init_pair(WRONG_WORD, COLOR_RED, COLOR_BLACK);
	init_pair(SELECT_WORD, COLOR_BLACK, COLOR_WHITE);

	x_offset = (COLS/2) - 30;
	y_offset = 5;

	mvprintw(y_offset + 3, (COLS/2)-7, "-----------------");
	// ──────────────
	return 1;
}

int typing_word_correctly(char *user, char *word_to_type){
	for( ; *user != '\0' ; user++, word_to_type++ ){
		if (*user != *word_to_type)
			return 0;
	}
	return 1;
}

int word_typed_right(char *user, char *word_to_type){
	for( ; *user == *word_to_type ; user++, word_to_type++ ){
		if (*user == '\0')
			return 1;
	}
	return 0;
}

uint8_t print_raw = 0;
void print_words_to_type()
{
	size_t word_lenght = 0;
	int i;

	attroff(COLOR_PAIR(WRONG_WORD));
	attroff(COLOR_PAIR(RIGHT_WORD));

	move(y_offset, 0);
	clrtoeol();
	for (i = N_WORD_PER_ROW * print_raw; i < N_WORD_PER_ROW + N_WORD_PER_ROW * print_raw; i++) {
		mvprintw(y_offset, x_offset + word_lenght, words[i]);
		word_lenght += strlen(words[i]) + 1;
	}

	word_lenght = 0;
	print_raw++;

	move(y_offset + 1, 0);
	clrtoeol();

	for (i = N_WORD_PER_ROW * print_raw; i < N_WORD_PER_ROW * (1 + print_raw); i++) {
		mvprintw(y_offset + 1, x_offset + word_lenght, words[i]);
		word_lenght += strlen(words[i]) + 1;
	}
}

int feed_words_into_array()
{
	FILE *words_file;
	words_file = fopen("words.txt", "r");

	int i = 0;
	char *buffer;
	buffer = (char*)malloc(WORD_LENGHT*sizeof(char));

	srand((unsigned) time(NULL));

	for (i = 0; i < MAX_WORDS; i++)
		words[i] = (char*)malloc(WORD_LENGHT*sizeof(char));

	i = 0;
	// 1/3 of probabilty that the word will be added to the array
	while ((fscanf(words_file, "%s", buffer) != EOF) && i < MAX_WORDS )
		if (rand() % 4 == 1)
			strcpy(words[i++], buffer);
	
	free(buffer);
	fclose(words_file);

	shuffle();

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

int typing_round()
{
	print_words_to_type();

	int cursor_x, cursor_y, n_ch = 0, index_word_to_type = 0;
	int correct_typed_words = 0;
	int ch;
	size_t word_lenght = 0;

	char *user_word, *word_to_type;
	user_word = (char*)malloc(WORD_LENGHT*sizeof(char));
	word_to_type = (char*)malloc(WORD_LENGHT*sizeof(char));

	strcpy(word_to_type, words[index_word_to_type]);

	attron(COLOR_PAIR(SELECT_WORD));
	mvprintw(y_offset, x_offset, word_to_type);
	attroff(COLOR_PAIR(SELECT_WORD));

	move(y_offset + 2, (COLS/2) - 5);
	while( (ch = getch()) != KEY_ESC )
	{
		switch (ch)
		{
			case KEY_BACKSPACE:
				if(n_ch > 0)
				{
					getyx(stdscr, cursor_y, cursor_x);
					mvdelch(cursor_y, cursor_x - 1);
					user_word--;
					n_ch--;
				}
				break;
			case ' ':
				// end the string
				*user_word = '\0';
				user_word -= n_ch;
				n_ch = 0;

				// update the word that must be typed
				index_word_to_type++;
				if (index_word_to_type == (N_WORD_PER_ROW * print_raw))
				{
					print_words_to_type();
					word_lenght = 0;
				} else {
					if (word_typed_right(user_word, word_to_type))
					{
						correct_typed_words++;
						attron(COLOR_PAIR(RIGHT_WORD));

					} else 
						attron(COLOR_PAIR(WRONG_WORD));

					// mark as correct or wrong the current typed word
					mvprintw(y_offset, x_offset + word_lenght, word_to_type);
					attroff(COLOR_PAIR(WRONG_WORD));
					attroff(COLOR_PAIR(RIGHT_WORD));

					word_lenght += strlen(word_to_type) + 1;
				}
				strcpy(word_to_type, words[index_word_to_type]);

				// select next word to be typed
				attron(COLOR_PAIR(SELECT_WORD));
				mvprintw(y_offset, x_offset + word_lenght, word_to_type);

				// reset the cursor
				move(y_offset + 2, (COLS/2) - 5);
				clrtoeol();

				break;
			case '\n':
				break;
			case KEY_ESC:
				endwin();
				return 0;
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

	return 1;
}
