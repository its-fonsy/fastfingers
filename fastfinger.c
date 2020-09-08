#include <curses.h>
#include <string.h>
#include <stdlib.h>

#define N_WORDS 8
#define KEY_ESC 27

int
main()
{
	int ch;
	char *words[N_WORDS] = {"ciao", "prova", "test", "luce", "astronomo", "fiore", "pianeta", "inventore"};

	/* Initial setup */
	initscr();
	raw();
	keypad(stdscr, TRUE);
	noecho();

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
	move(6, (COLS/2) - 5);
	int cursor_x, cursor_y, n_ch = 0;
	char *user_word;
	user_word = (char*)malloc(32*sizeof(char));

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
				*user_word = '\0';
				user_word -= n_ch;
				n_ch = 0;

				move(0, 0);
				clrtoeol();
				printw("The word is %s", user_word);

				move(6, (COLS/2) - 5);
				clrtoeol();
				break;
			default:
				addch(ch);
				*(user_word++) = ch;
				n_ch++;
				break;
		}
	}

	endwin(); 

	return 0;
}

