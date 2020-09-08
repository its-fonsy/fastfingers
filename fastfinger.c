#include <curses.h>
#include <string.h>
#include <stdlib.h>

#define N_WORDS 8
#define MAX_WORDS 255
#define WORD_LENGHT 8

#define KEY_ESC 27

/* Color stuff */
#define RIGHT_WORD 1
#define WRONG_WORD 2
#define SELECT_WORD 3

int typing_word_correctly(char *user, char *word_to_type);

int main()
{
	int ch;
	char *words[N_WORDS] = {"ciao", "prova", "test", "luce", "astronomo", "fiore", "pianeta", "inventore"};

	/* Initial setup */
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

	/* Print the word to type in the center of the screen */
	size_t word_lenght = 0;
	for (int i = 0; i < N_WORDS; i++) {
		word_lenght += strlen(words[i]);
	}

	int x_offset = (COLS/2) - (word_lenght + 7)/2;

	word_lenght = 0;
	for (int i = 0; i < N_WORDS; i++) {
		mvprintw(5, x_offset + word_lenght, words[i]);
		word_lenght += strlen(words[i]) + 1;
	}

	/* Prepare to user input */
	int cursor_x, cursor_y, n_ch = 0, index_word_to_type = 0;
	int correct_typed_words = 0;

	char *user_word, *word_to_type;
	user_word = (char*)malloc(WORD_LENGHT*sizeof(char));
	word_to_type = (char*)malloc(WORD_LENGHT*sizeof(char));

	strcpy(word_to_type, words[index_word_to_type]);

	word_lenght = 0;

	attron(COLOR_PAIR(SELECT_WORD));
	mvprintw(5, x_offset, word_to_type);
	attroff(COLOR_PAIR(SELECT_WORD));

	move(6, (COLS/2) - 5);
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
				if (index_word_to_type < N_WORDS - 1)
					index_word_to_type++;
				else
					index_word_to_type = 0, word_lenght = 0;

				// check if the word typed is correct
				if(typing_word_correctly(user_word, word_to_type))
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
				strcpy(word_to_type, words[index_word_to_type]);

				// select next word to be typed
				attron(COLOR_PAIR(SELECT_WORD));
				mvprintw(5, x_offset + word_lenght, word_to_type);

				// reset the cursor
				move(6, (COLS/2) - 5);
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

int typing_word_correctly(char *user, char *word_to_type){
	for( ; *user != '\0' ; user++, word_to_type++ ){
		if (*user != *word_to_type)
			return 0;
	}
	return 1;
}
