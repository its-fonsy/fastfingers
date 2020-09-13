#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <locale.h>
#include <unistd.h>
#include <time.h>

#include "util.h"

#define MAX_WORDS_PER_ROW 7
#define KEY_ESC 27

// color stuff
#define COL_RIGHT_WORD 1
#define COL_WRONG_WORD 2
#define COL_SELECT_WORD 3

#define DEFAULT_ROUND_TIME 59 

// variables
int x_offset, y_offset;
char *words[MAX_WORDS];

// functions
int time_track();
void second_elapsed();

int init_curses();
int draw_gui();
int feed_words_into_array(char* filename, char* array[]);
void print_words_to_type();
int typing_round();
int view_score();

int main(int argc, char *argv[])
{
	int time_child, ch;
	int play_again = 1;

	if( argc == 2 )
	{
		ch = *++argv[1];
		switch(ch)
		{
			case 'h':
				// print usage message
				printf("usage: %s [-h]\n\n", argv[0]);
				printf("Practice your touch typing in your terminal\n\n");
				printf("OPTION\n");
				printf("  -h\t\tPrint this help message\n");
				break;
		}
		return 0;
	}

	// gui init
	init_curses();

	// malloc the words array
	for (int i = 0; i < MAX_WORDS; i++)
		words[i] = (char*)malloc(MAX_WORD_LENGHT*sizeof(char));
	
	// child take track of time
	time_child = fork();
	if (time_child == 0)
		time_track();

	// father handle child signals
	signal(SIGUSR1, second_elapsed);

	while(play_again)
	{
		draw_gui();

		// populate the array with words from a file
		feed_words_into_array("words.txt", words);
		print_words_to_type();

		// type some words
		if( typing_round() )
		{
			// disable cursor
			curs_set(0);

			// view the score of a round
			view_score();

			// CTRL+C to exit
			// TAB to play again
			while((ch = getch()))
				if(ch == '	')
					break;
		}
		clear();
		curs_set(1);
	}
	
	endwin();
	kill(time_child, SIGKILL);

	return 0;
}

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

uint8_t print_raw = 0;
void print_words_to_type()
{
	size_t word_lenght = 0;
	int i;

	attroff(COLOR_PAIR(COL_WRONG_WORD));
	attroff(COLOR_PAIR(COL_RIGHT_WORD));

	move(y_offset, 0);
	clrtoeol();
	for (i = MAX_WORDS_PER_ROW * print_raw; i < MAX_WORDS_PER_ROW + MAX_WORDS_PER_ROW * print_raw; i++) {
		/* mvprintw(y_offset, x_offset + word_lenght, words[i]); */
		mvaddstr(y_offset, x_offset + word_lenght, words[i]);
		word_lenght += string_len(words[i]) + 1;
	}

	word_lenght = 0;
	print_raw++;

	move(y_offset + 1, 0);
	clrtoeol();

	for (i = MAX_WORDS_PER_ROW * print_raw; i < MAX_WORDS_PER_ROW * (1 + print_raw); i++) {
		/* mvprintw(y_offset + 1, x_offset + word_lenght, words[i]); */
		mvaddstr(y_offset + 1, x_offset + word_lenght, words[i]);
		word_lenght += string_len(words[i]) + 1;
	}
}

int n_ch = 0, playing_round = 0, playing_time = DEFAULT_ROUND_TIME;
int correct_typed_words 	= 0;
int correct_keystroke		= 0;
int incorrect_typed_words 	= 0;
int incorrect_keystroke		= 0;
char *user_word;

/* playing_round has 3 state:			*/
/* 	  0 new game wait to start	 		*/
/* 	  1 game in progress				*/
/* 	 -1 round ended cause time expired	*/

int typing_round()
{
	int cursor_x, cursor_y, ch, index_word_to_type = 0;
	size_t word_lenght = 0;

	char *word_to_type;
	user_word = (char*)malloc(MAX_WORD_LENGHT*sizeof(char));
	word_to_type = (char*)malloc(MAX_WORD_LENGHT*sizeof(char));

	// copy the first word to type
	strcpy(word_to_type, words[index_word_to_type]);

	// select the first word to type
	attron(COLOR_PAIR(COL_SELECT_WORD));
	mvprintw(y_offset, x_offset, word_to_type);
	attroff(COLOR_PAIR(COL_SELECT_WORD));

	// reset the round variables
	playing_round 	= 0;
	n_ch 			= 0;
	playing_time	= DEFAULT_ROUND_TIME;

	// reset the score
	correct_typed_words 	= 0;
	correct_keystroke		= 0;
	incorrect_typed_words 	= 0;
	incorrect_keystroke		= 0;

	// print the start time
	attron(A_BOLD);
	mvprintw(y_offset - 2, (COLS/2) - 3, "01:00");
	attroff(A_BOLD);

	// clear the cursor line and move on position
	move(y_offset + 2, 0);
	clrtoeol();
	move(y_offset + 2, (COLS/2) - 5);

	// print a line under user input
	mvaddstr(y_offset + 4, (COLS/2) - 5 - 1, "────────────────");

	// move the cursor to user input
	move(y_offset + 3, (COLS/2) - 5);
	*user_word = '\0';

	while( playing_round != -1 && (ch = getch()))
	{
		switch (ch)
		{
			case KEY_BACKSPACE:
				if(n_ch > 0)
				{
					// delete character from the word
					getyx(stdscr, cursor_y, cursor_x);
					mvdelch(cursor_y, cursor_x - 1);

					user_word--;
					n_ch--;
					incorrect_keystroke--;

					// dealing with accent letters
					if(*(--user_word) == -61)
						n_ch--;
					else
						user_word++;

					*user_word = '\0';
				}
				break;

			case ' ': // space pressed

				// missclick
				if(!n_ch)
					break;

				// end the string
				*user_word = '\0';
				user_word -= n_ch;
				n_ch = 0;

				// update the word to be typed
				index_word_to_type++;
				if (index_word_to_type == (MAX_WORDS_PER_ROW * print_raw))
				{
					print_words_to_type();
					word_lenght = 0;

					if (word_typed_right(user_word, word_to_type))
						correct_typed_words++;
					else
						incorrect_typed_words++;

				} else {
					if (word_typed_right(user_word, word_to_type))
					{
						correct_typed_words++;
						attron(COLOR_PAIR(COL_RIGHT_WORD));

					} else
					{
						incorrect_typed_words++;
						attron(COLOR_PAIR(COL_WRONG_WORD));
					}

					// mark as correct or wrong the current typed word
					mvprintw(y_offset, x_offset + word_lenght, word_to_type);
					attroff(COLOR_PAIR(COL_WRONG_WORD));
					attroff(COLOR_PAIR(COL_RIGHT_WORD));

					word_lenght += string_len(word_to_type) + 1;
				}
				strcpy(word_to_type, words[index_word_to_type]);

				// select next word to be typed
				attron(COLOR_PAIR(COL_SELECT_WORD));
				mvprintw(y_offset, x_offset + word_lenght, word_to_type);
				attroff(COLOR_PAIR(COL_SELECT_WORD));

				// reset the cursor and user word
				move(y_offset + 3, (COLS/2) - 5);
				*user_word = '\0';
				clrtoeol();

				break;

			case '\n' || KEY_ESC:
				break;

			case '	': // TAB pressed
				print_raw = 0;
				return 0; break;

			default:
				break;
		}

		// keypress is alphabetical character
		if ( ((ch >=97)   && (ch <= 122))||	// a to z
			  (ch == 160) || (ch == 168) ||	// à and è
			  (ch == 172) || (ch == 178) ||	// ì and ò
		 	  (ch == 185)             	  )	// ù
		{

			// start the timer
			if(!playing_round)
				playing_round = 1;

			// deal with accent word
			switch(ch)
			{
				case 160: // à
					*(user_word++) = -61;
					*(user_word++) = -96;
					n_ch += 2;
					break;

				case 168: // è
					*(user_word++) = -61;
					*(user_word++) = -88;
					n_ch += 2;
					break;

				case 172: // ì
					*(user_word++) = -61;
					*(user_word++) = -84;
					n_ch += 2;
					break;

				case 178: // ò
					*(user_word++) = -61;
					*(user_word++) = -78;
					n_ch += 2;
					break;

				case 185: // ù
					*(user_word++) = -61;
					*(user_word++) = -71;
					n_ch += 2;
					break;

				default: // a to z
					*(user_word++) = ch;
					n_ch++;
					break;
				}

			*user_word = '\0';

			if( typing_word_correctly(user_word - n_ch, word_to_type) )
			{
				correct_keystroke++;
				attron(COLOR_PAIR(COL_RIGHT_WORD));
			}
			else
			{
				attron(COLOR_PAIR(COL_WRONG_WORD));
				incorrect_keystroke++;
			}

			switch(ch)
			{
				case 160:
					addstr("à");
					break;
				case 168:
					addstr("è");
					break;
				case 172:
					addstr("ì");
					break;
				case 178:
					addstr("ò");
					break;
				case 185:
					addstr("ù");
					break;
				default:
					addch(ch);
					break;
			}
		}
	}

	// round ended because the time expired
	print_raw = 0;

	// memory clear
	free(user_word);
	free(word_to_type);

	return 1;
}

int view_score(int c_ks, int i_ks, int c_w, int i_w)
{
	clear();
	draw_gui();
	
	// disable color text
	attroff(COLOR_PAIR(COL_WRONG_WORD));
	attroff(COLOR_PAIR(COL_RIGHT_WORD));
	attroff(COLOR_PAIR(COL_SELECT_WORD));

	// print the WPM
	// WPM is calculated with this formula
	// WPM = (correct_keystroke / 5) / Time[min]
	attron(A_BOLD);
	mvprintw(y_offset, (COLS/2) - 3, "%d WPM", correct_keystroke / 5);
	attroff(A_BOLD);

	// print keystrokes
	mvprintw(y_offset + 1, (COLS/2) - 10, "Keystrokes: %d (", correct_keystroke + incorrect_keystroke);

	attron(COLOR_PAIR(COL_RIGHT_WORD));
	printw("%d", correct_keystroke);
	attroff(COLOR_PAIR(COL_RIGHT_WORD));

	printw("|");

	attron(COLOR_PAIR(COL_WRONG_WORD));
	printw("%d", incorrect_keystroke);
	attroff(COLOR_PAIR(COL_WRONG_WORD));
	printw(")");

	// print number of correct and incorrect words
	mvprintw(y_offset + 2, (COLS/2) - 10, "Correct words: %d", correct_typed_words);
	mvprintw(y_offset + 3, (COLS/2) - 10, "Mistyped words: %d", incorrect_typed_words);

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

	return 1;
}

void second_elapsed()
{
	// disabling all coloring fancy
	attroff(COLOR_PAIR(COL_WRONG_WORD));
	attroff(COLOR_PAIR(COL_RIGHT_WORD));
	attroff(COLOR_PAIR(COL_SELECT_WORD));

	// decrease the time only if the user is playing
	attron(A_BOLD);
	if(playing_round == 1)
		mvprintw(y_offset - 2, (COLS/2) - 3, (playing_time-- >= 10) ? "00:%d" : "00:0%d", playing_time);
	attroff(A_BOLD);

	// when time reach 0 then end the round
	playing_round = (playing_time <= 0 && playing_round != 0) ? -1 : playing_round;

	// reset the cursor to the input box
	move(y_offset + 3, (COLS/2) - 5 + string_len(user_word - n_ch));
	refresh();
}

// Child send SIGUSR1 every second 
int time_track()
{
	clock_t begin = clock();
	double time_spent;

	while(1)
	{
		time_spent = (double)(clock() - begin) / CLOCKS_PER_SEC;

		if(time_spent >= 1.0)
		{
			begin = clock();
			kill(getppid(), SIGUSR1);
		}
	}
}
