#include <ncurses.h>
#include <locale.h>
#include <stdlib.h>

#include "gui.h"
#include "util.h"

int x_offset, y_offset;

int init_curses()
{
	// set locale for extended ASCII
	setlocale(LC_ALL, "");

	// initial curses setup
	initscr();
	/* raw(); */
	keypad(stdscr, TRUE);
	noecho();
	/* timeout(1); */

	// no colors no party
	if(has_colors() == FALSE)
	{	endwin();
		printf("Your terminal does not support color\n");
		exit(1);
	}

	start_color();
	init_pair(COL_RIGHT_WORD, COLOR_GREEN, COLOR_BLACK);
	init_pair(COL_WRONG_WORD, COLOR_RED, COLOR_BLACK);
	init_pair(COL_SELECT_WORD, COLOR_BLACK, COLOR_WHITE);

	x_offset = (COLS/2) - 30;
	y_offset = 5;

	return 1;
}

int draw_gui()
{
	attron(COLOR_PAIR(COL_SELECT_WORD));

	// draw a white line
	move(0,0);
	for (int i = 0; i < COLS; i++)
		addch(' ');

	// print some info
	mvprintw(0, 2, "CTRL+C to exit");
	mvprintw(0, (COLS/2) - 6, "Fast Fingers");
	mvprintw(0, COLS - 14, "TAB to reset");

	attroff(COLOR_PAIR(COL_SELECT_WORD));

	return 1;
}

void print_words_to_type(int print_raw, char *array[])
{
	size_t word_lenght = 0;
	int i;

	attroff(COLOR_PAIR(COL_WRONG_WORD));
	attroff(COLOR_PAIR(COL_RIGHT_WORD));

	move(y_offset, 0);
	clrtoeol();
	for (i = MAX_WORDS_PER_ROW * print_raw; i < MAX_WORDS_PER_ROW + MAX_WORDS_PER_ROW * print_raw; i++) {
		/* mvprintw(y_offset, x_offset + word_lenght, words[i]); */
		mvaddstr(y_offset, x_offset + word_lenght, array[i]);
		word_lenght += string_len(array[i]) + 1;
	}

	word_lenght = 0;
	print_raw++;

	move(y_offset + 1, 0);
	clrtoeol();

	for (i = MAX_WORDS_PER_ROW * print_raw; i < MAX_WORDS_PER_ROW * (1 + print_raw); i++) {
		/* mvprintw(y_offset + 1, x_offset + word_lenght, words[i]); */
		mvaddstr(y_offset + 1, x_offset + word_lenght, array[i]);
		word_lenght += string_len(array[i]) + 1;
	}
}

int view_score(struct score round)
{
	// disable cursor
	curs_set(0);

	// clear the screen
	clear();

	// redraw the GUI
	draw_gui();
	
	// disable color text
	attroff(COLOR_PAIR(COL_WRONG_WORD));
	attroff(COLOR_PAIR(COL_RIGHT_WORD));
	attroff(COLOR_PAIR(COL_SELECT_WORD));

	// print the WPM
	// WPM is calculated with this formula
	// WPM = (correct_keystroke / 5) / Time[min]
	attron(A_BOLD);
	mvprintw(y_offset, (COLS/2) - 3, "%d WPM", round.correct_keystrokes / 5);
	attroff(A_BOLD);

	// print keystrokes
	mvprintw(y_offset + 1, (COLS/2) - 10, "Keystrokes: %d (", round.correct_keystrokes + round.incorrect_keystrokes);

	attron(COLOR_PAIR(COL_RIGHT_WORD));
	printw("%d", round.correct_keystrokes);
	attroff(COLOR_PAIR(COL_RIGHT_WORD));

	printw("|");

	attron(COLOR_PAIR(COL_WRONG_WORD));
	printw("%d", round.incorrect_keystrokes);
	attroff(COLOR_PAIR(COL_WRONG_WORD));
	printw(")");

	// print number of correct and incorrect words
	mvprintw(y_offset + 2, (COLS/2) - 10, "Correct words: %d", round.correct_typed_words);
	mvprintw(y_offset + 3, (COLS/2) - 10, "Mistyped words: %d", round.incorrect_typed_words);

	// print exit key
	move(y_offset + 5, (COLS/2) - 20 - 5);
	printw("Press ");
	attron(A_BOLD);
	printw("CTRL+C");
	attroff(A_BOLD);
	printw(" to exit");

	// print replay key
	move(y_offset + 5, (COLS/2) + 5);
	printw("Press ");
	attron(A_BOLD);
	printw("TAB");
	attroff(A_BOLD);
	printw(" to play again");

	// CTRL+C to exit
	// TAB to play again
	int ch;
	while((ch = getch()))
		if(ch == '	')
			break;

	curs_set(1);

	return 1;
}
